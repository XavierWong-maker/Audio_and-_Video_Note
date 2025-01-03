//
// Created by Administrator on 2024/7/30.
//

#ifndef XPLAY2_XAVCODECPARAMETERS_HPP
#define XPLAY2_XAVCODECPARAMETERS_HPP

extern "C"{
#include <libavcodec/codec_par.h>
}

#include "xhelper.hpp"

class XCodecParameters final : AVCodecParameters {
    static void Reset(AVCodecParameters *) noexcept(true);
    void Move(XCodecParameters *) noexcept(true);
public:
    XCodecParameters();
    explicit XCodecParameters(const AVCodecContext *,const AVRational & = {1,1}) noexcept(false);
    explicit XCodecParameters(const AVCodecParameters *,const AVRational & = {1,1}) noexcept(false);
    XCodecParameters(const XCodecParameters &) noexcept(false);
    XCodecParameters(XCodecParameters &&) noexcept(true);
    XCodecParameters& operator=(const XCodecParameters &) noexcept(false);
    XCodecParameters& operator=(XCodecParameters &&) noexcept(true);

    bool from_AVFormatContext(const AVCodecParameters *)  noexcept(true);
    bool to_AVCodecParameters(AVCodecParameters *) const noexcept(true);
    bool from_context(const AVCodecContext *)  noexcept(true);
    bool to_context(AVCodecContext *) const noexcept(true);

    [[nodiscard]] auto MediaType() const noexcept(true){
        return codec_type;
    }

    [[nodiscard]] auto Codec_id() const noexcept(true){
        return codec_id;
    }

    [[nodiscard]] std::string Codec_name() const noexcept(true);

    [[nodiscard]] auto Ch_layout() const noexcept(true){
        return &ch_layout;
    }

    [[nodiscard]] auto Audio_sample_format() const noexcept(true){
        return format;
    }

    [[nodiscard]] auto Video_pixel_format() const noexcept(true){
        return format;
    }

    [[nodiscard]] int Sample_Format_Size() const noexcept(true);

    [[nodiscard]] auto Sample_rate() const noexcept(true){
        return sample_rate;
    }

    [[nodiscard]] auto Width() const noexcept(true){
        return width;
    }

    [[nodiscard]] auto Height() const noexcept(true){
        return height;
    }

    ~XCodecParameters();

    void set_time_base(const AVRational &tb) noexcept(true){
        m_time_base = tb;
    }

    [[nodiscard]] auto time_base() const noexcept(true){return m_time_base;}

private:
    AVRational m_time_base{1,1};
};

XCodecParameters_sp new_XCodecParameters();
XCodecParameters_sp new_XCodecParameters(const AVCodecParameters *src,const AVRational &tb = {1,1});
XCodecParameters_sp new_XCodecParameters(const AVCodecContext *src,const AVRational &tb = {1,1});

#endif

