//
// Created by Administrator on 2024/8/9.
//

#ifndef XPLAY2_IVIDEOCALL_HPP
#define XPLAY2_IVIDEOCALL_HPP

#include <memory>

class XAVFrame;
using XAVFrame_sptr = typename std::shared_ptr<XAVFrame>;

class IVideoCall {

public :
    /***
     * 初始化
     * @param w,h
     */
    virtual void Init(const int &w,const int&h) noexcept(false) = 0;
    /***
     * param XAVFrame_sptr
     */
    virtual void Repaint(const XAVFrame_sptr &) noexcept(false) = 0;
};

#endif
