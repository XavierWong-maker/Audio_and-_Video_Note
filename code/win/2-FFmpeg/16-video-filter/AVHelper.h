
#ifndef AVHELPER_H
#define AVHELPER_H

#include <iostream>
#include <string>

extern "C"{
#include <libavutil/error.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libavfilter/avfilter.h>
}

#include "ShareAVFrame.hpp"
#include "ShareAVPacket.hpp"

namespace AVHelper {

    std::string av_get_err(const int&) noexcept(true);
    void log_packet(const AVFormatContext &, const AVPacket &)  noexcept(true);

    void avfilter_graph_dump(AVFilterGraph *,const std::string & ) noexcept(false);

    std::error_code make_error_code_helper(const int &errcode) noexcept(true);

    template<typename F,typename ...Args>
    static inline void decode(const std::string &name,
                              AVCodecContext *codec_ctx,
                              const AVPacket *pkt,
                              ShareAVFrame_sp_type &frame,
                F&& f,Args&& ...args) noexcept(false)
    {
        /* send the packet with the compressed data to the decoder */
        auto ret { avcodec_send_packet(codec_ctx, pkt) };

        if(AVERROR(EAGAIN) == ret || AVERROR_EOF == ret){
            std::cerr << name + " Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n";
        }else if (ret < 0){
            const auto errmsg (name + " Error submitting the packet to the decoder: " + av_get_err(ret) +
                      " , pkt_size : " + std::to_string(pkt->size) + "\n");
            throw std::system_error(make_error_code_helper(ret),errmsg);
        }else{}

        /* read all the output frames (infile general there may be any number of them */
        for(;;){
            // 对于frame, avcodec_receive_frame内部每次都先调用
            ret = avcodec_receive_frame(codec_ctx, frame->m_frame);

            if (AVERROR(EAGAIN) == ret  || AVERROR_EOF == ret) {

                const auto errmsg(name + " avcodec_receive_frame failed: " +av_get_err(ret) + "\n");
                throw std::system_error(make_error_code_helper(ret),errmsg);

            }else if (ret < 0){
                const auto errmsg(name + " Error during decoding : " + av_get_err(ret) + "\n");
                throw std::system_error(make_error_code_helper(ret),errmsg);
            }else{
                f(args...);
            }
        }
    }

    template<typename F,typename ...Args>
    static inline void encode(const std::string &name,
                              AVCodecContext *codec_ctx,
                              const AVFrame *frame ,
                              ShareAVPacket_sp_type &packet,
                F&& f,Args&& ...args) noexcept(false)
    {
        auto ret{avcodec_send_frame(codec_ctx,frame)};

        if (AVERROR(EAGAIN) == ret || AVERROR_EOF == ret){
            std::cerr << name + " Receive_packet and send_frame both returned EAGAIN, which is an API violation.\n";
        }else if (ret < 0) {
            const auto errmsg(name + " avcodec_send_frame failed : " + av_get_err(ret) + "\n");
            throw std::system_error(make_error_code_helper(ret),errmsg);
        }else{}

        for (;;) {

            ret = avcodec_receive_packet(codec_ctx,packet->m_packet);

            if (AVERROR_EOF == ret || AVERROR(EAGAIN) == ret){
                const auto msg(name + " avcodec_receive_packet failed: " +
                               std::to_string(ret) +  "\t" + av_get_err(ret) + "\n");
                throw std::system_error(make_error_code_helper(ret),msg);
            }else if(ret < 0){

                const auto errmsg(name + " Error during encoding: " +
                                            std::to_string(ret) + "\t" + av_get_err(ret) + "\n");
                throw std::system_error(make_error_code_helper(ret),errmsg);
            }else{
                f(args...);
            }
        }
    }
}

#endif