extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}
#include <iostream>
#include <thread>
#include "xdecode.hpp"
#include "xvideo_view.hpp"
#include "xavpacket.hpp"
#include "xavframe.hpp"

using namespace std::chrono;
using namespace std::this_thread;

int main(const int argc,const char *argv[]) {

    AVFormatContext *ic{},*ec{};

    const Destroyer d([&]{
        avformat_close_input(&ic);
        if (ec){
            avio_closep(&ec->pb);
            avformat_free_context(ec);
            ec = nullptr;
        }
    });
////////////////////////////////////////////////////////打开媒体///////////////////////////////////////////////////////////////
    constexpr auto url{"v1080.mp4"};
    /**
     * 打开媒体文件
     */
    FF_ERR_OUT(avformat_open_input(&ic,url, nullptr, nullptr),return -1);

    /**
     * 查找文件信息,对于一些信息不完整的媒体文件特别有用
     */
    FF_ERR_OUT(avformat_find_stream_info(ic, nullptr),return -1);

    /**
     * 打印媒体文件的信息
     */
    av_dump_format(ic,0, url,0);

    /**
     * 通过索引精确查找出音频或者视频信息
     */
    AVStream *vs{},*as{};
    for (int i {}; i < ic->nb_streams; ++i) {
        const auto codecpar{ic->streams[i]->codecpar};
        if (AVMEDIA_TYPE_VIDEO == codecpar->codec_type){
            vs = ic->streams[i];
            std::cerr << "width: " << codecpar->width << " height: " << codecpar->height << "\n";
        } else if (AVMEDIA_TYPE_AUDIO == codecpar->codec_type){
            std::cerr << "sample_rate: " << codecpar->sample_rate << "\n";
            as = ic->streams[i];
        } else{}
    }

    if (!vs && !as){
        return -1;
    }
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////封装输出媒体文件/////////////////////////////////////////////////////////////////////////////
    const auto out_url{GET_STR(out.mp4)};
    /**
     * 创建输出文件,并创建上下文
     */
    FF_ERR_OUT(avformat_alloc_output_context2(&ec, nullptr, nullptr,out_url),return -1);
    auto mvs{avformat_new_stream(ec, nullptr)};
    auto mas{avformat_new_stream(ec, nullptr)};
    if (!mvs || !mas){
        return -1;
    }

    /**
     * 打开文件IO
     */
    FF_ERR_OUT(avio_open(&ec->pb,out_url,AVIO_FLAG_WRITE),return -1);

    /**
     * 拷贝音视频流信息
     */
    if (vs){
        mvs->time_base = vs->time_base;
        avcodec_parameters_copy(mvs->codecpar,vs->codecpar);
    }

    if (as){
        mas->time_base = as->time_base;
        avcodec_parameters_copy(mas->codecpar,as->codecpar);
    }

    /**
     * 写入媒体文件头部信息
     */
    FF_ERR_OUT(avformat_write_header(ec, nullptr),return -1);

    /**
     * 打印输出媒体文件的信息
     */
    av_dump_format(ec,0,out_url,1);

    //截取10 ~ 20秒之间的音视频 取多不取少
    //假定 9 11秒有关键帧 我们取第9秒
    auto begin_sec {10.0},end_sec{20.0};
    int64_t begin_pts{},end_pts{};
    //换算pts换成输入ic的pts,以视频流为准

    if (vs && vs->time_base.num > 0) {
        // pts = sec / time_base
        // pts = sec / (num / den) = sec * (den / num)
        const auto t{static_cast<double>(vs->time_base.den) / static_cast<double>(vs->time_base.num)}; // den分母 / num分子
        begin_pts = static_cast<int64_t>(begin_sec  * t);
        end_pts = static_cast<int64_t>(end_sec * t);
    }

    int64_t begin_audio_pts{};
    if (as && as->time_base.num > 0){
        const auto t{static_cast<double >(as->time_base.den) / static_cast<double >(as->time_base.num)};
        begin_audio_pts = static_cast<int64_t>(begin_sec * t);
    }

    /**
     * 向后关键帧
     */
    FF_ERR_OUT(av_seek_frame(ic,vs->index,begin_pts,
                             AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD),return -1);

    auto packet{new_XAVPacket()};
    while (true){
        int re;
        FF_ERR_OUT(re = av_read_frame(ic, packet.get()));
        if (re < 0) {
            break;
        }
        auto in_stream{ic->streams[packet->stream_index]};
        AVStream *out_stream{};
        int64_t offset_pts{};

        if (packet->stream_index == vs->index){
            std::cerr << "video: packet.size: " << packet->size << "\t" <<
                      "packet.dts: " << packet->dts << "\t" <<
                      "packet.pts: " << packet->pts << "\n";

            //超过第20秒退出,只存10~20秒
            if (packet->pts > end_pts){
                packet->Reset();
                break;
            }

            out_stream = ec->streams[packet->stream_index];
            offset_pts = begin_pts;

        }else if (as && packet->stream_index == as->index){
            std::cerr << "audio: packet.size: " << packet->size << "\t" <<
                      "packet.dts: " << packet->dts << "\t" <<
                      "packet.pts: " << packet->pts << "\n";

            out_stream = ec->streams[packet->stream_index];
            offset_pts = begin_audio_pts;
        } else{}

        //写入前需重新计算pts dts duration
        //a * bq(输入的time_base) / cq(目标的time_base)

        packet->pts = av_rescale_q_rnd(packet->pts - offset_pts,in_stream->time_base,out_stream->time_base,
                                       static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

        packet->dts = av_rescale_q_rnd(packet->dts - offset_pts,in_stream->time_base,out_stream->time_base,
                                       static_cast<AVRounding>(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

        packet->duration = av_rescale_q(packet->duration,in_stream->time_base,out_stream->time_base);
        packet->pos = -1;
        /**
         * 交错写入文件
         */
        FF_ERR_OUT(av_interleaved_write_frame(ec,packet.get()),return -1);
        //packet->Reset();
    }
    /**
     * 写入文件尾部信息
     */
    FF_ERR_OUT(av_write_trailer(ec),return -1);
    return 0;
}
