//
// Created by Administrator on 2024/7/30.
//
#include "XAVCodecParameters.hpp"

extern "C"{
#include <libavcodec/codec_par.h>
#include <libavcodec/avcodec.h>
}

#include <iostream>

XAVCodecParameters_sptr new_XAVCodecParameters() noexcept(false)
{
    XAVCodecParameters_sptr obj;

    CHECK_EXC(obj.reset(new XAVCodecParameters));

    try {
        obj->Construct();
        return obj;
    } catch (...) {
        obj.reset();
        std::rethrow_exception(std::current_exception());
    }
}

void XAVCodecParameters::Construct() noexcept(false) {
    CHECK_NULLPTR(m_parm = avcodec_parameters_alloc());
}

void XAVCodecParameters::DeConstruct() noexcept(true) {
    avcodec_parameters_free(&m_parm);
}

void XAVCodecParameters::from_AVFormatContext(const AVCodecParameters *src) const noexcept(false) {
    if (!src){
        std::cerr << __func__ << " src is nullptr\n";
    }
    FF_CHECK_ERR(avcodec_parameters_copy(m_parm,src));
}

void XAVCodecParameters::from_context(const AVCodecContext *src) const noexcept(false) {
    if (!src){
        std::cerr << __func__ << " src is nullptr\n";
    }
    FF_CHECK_ERR(avcodec_parameters_from_context(m_parm,src));
}

void XAVCodecParameters::to_context(AVCodecContext *dst) const noexcept(false) {
    if (!dst){
        std::cerr << __func__ << " dst is nullptr\n";
    }
    FF_CHECK_ERR(avcodec_parameters_to_context(dst,m_parm));
}

XAVCodecParameters::~XAVCodecParameters() {
    DeConstruct();
}

int XAVCodecParameters::MediaType() const noexcept(true){
    return m_parm->codec_type;
}

int XAVCodecParameters::codec_id() const noexcept(true){
    return m_parm->codec_id;
}

std::string XAVCodecParameters::codec_name() const noexcept(true) {
    return avcodec_get_name(m_parm->codec_id);
}