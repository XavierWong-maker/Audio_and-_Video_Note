//
// Created by wong on 2024/8/8.
//

#ifndef XPLAY2_XAUDIOTHREAD_HPP
#define XPLAY2_XAUDIOTHREAD_HPP

#include "XDecodeThread.hpp"

class XResample;
class XAudioPlay;

class XAudioThread : public XDecodeThread {

Q_OBJECT
    void DeConstruct() noexcept(true);
    void entry() noexcept(false) override;
public:
    explicit XAudioThread(std::exception_ptr * = nullptr);
    ~XAudioThread() override;
    void Open(const XAVCodecParameters_sptr &) noexcept(false) override;
    void Close() noexcept(true) override;
protected:
    std::atomic<XAudioPlay *> m_audio_play{};
    QSharedPointer<XResample> m_resample;
    QMutex m_a_mux;
};

#endif