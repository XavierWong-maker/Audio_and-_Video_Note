#ifndef INC_120_TEST_XDECODE_XDECODE_HPP
#define INC_120_TEST_XDECODE_XDECODE_HPP

#include "xcodec.hpp"

using XAVFrames = std::vector<XAVFrame_sp>;

class XDecode : public XCodec {

public:
    /**
     * 发送包到解码器
     * @param packet
     * @return true or false
     */
    [[nodiscard]] bool Send(const XAVPacket &packet) const;

    /**
     * 从解码器接收解码后到帧,需循环读取
     * @param frame
     * @return true or false
     */
    [[nodiscard]] bool Receive(XAVFrame &frame) const;

    /**
     * 冲刷解码器,把缓冲的帧全部读取出来
     * @return XAVFrames or empty
     */
    [[nodiscard]] XAVFrames Flush() const;

    /**
     * 初始化硬解码
     * @param type 参考AVHWDeviceType
     * @return
     */
    bool InitHw(const int &type = 4);

    explicit XDecode() = default;

    X_DISABLE_COPY_MOVE(XDecode)
};

#endif
