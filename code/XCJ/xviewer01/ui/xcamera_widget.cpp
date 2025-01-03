#include "xcamera_widget.hpp"
#include <qevent.h>
#include <QListWidget>
#include <QStyleOption>
#include <QPainter>
#include <xdecodetask.hpp>
#include <xdemuxtask.hpp>
#include <xvideo_view.hpp>
#include "xcamera_config.hpp"

XCameraWidget::XCameraWidget(QWidget *parent) :
#ifdef MACOS
XVideoWidget(parent)
#else
QWidget(parent)
#endif
{
    setAcceptDrops(true);
}

void XCameraWidget::dragEnterEvent(QDragEnterEvent *event) {
    //接受拖拽进入
    event->acceptProposedAction();
    QWidget::dragEnterEvent(event);
}

//松开拖拽
void XCameraWidget::dropEvent(QDropEvent *event) {

    const auto wid{dynamic_cast<QListWidget*>(event->source())};

    const auto &[m_name_, m_url,
        m_sub_url, m_save_path]{XCamCfg()->GetCam(wid->currentRow())};
    Open(m_url);
    QWidget::dropEvent(event);
}

void XCameraWidget::paintEvent(QPaintEvent *event) {

    const auto temp_func{[this]{
        QStyleOption opt;
        opt.initFrom(this);
        QPainter painter(this);
        style()->drawPrimitive(QStyle::PE_Widget,&opt,&painter,this);
    }};

#ifdef MACOS
    if (!m_is_setStyle) {
        m_is_setStyle = true;
        temp_func();
    }
    QOpenGLWidget::paintEvent(event);
#else
    temp_func();
#endif
}

bool XCameraWidget::Open(const QString &url){

    if (!m_demux_){
        TRY_CATCH(CHECK_EXC(m_demux_.reset(new XDemuxTask())),return {});
    }else {
        m_demux_->Stop();
    }

    if (!m_decode_){
        TRY_CATCH(CHECK_EXC(m_decode_.reset(new XDecodeTask())),return {});
    }else {
        m_decode_->Stop();
    }

#ifndef MACOS
    if (!m_view_){
        TRY_CATCH(CHECK_EXC(m_view_.reset(XVideoView::create())),return {});
    }
#endif

    //打开解封转
    CHECK_FALSE_(m_demux_->Open(url.toStdString()),return {});

    XCodecParameters_sp parm;
    CHECK_FALSE_((parm = m_demux_->CopyVideoParm()).operator bool(),return {});
    //打开视频解码器
    CHECK_FALSE_(m_decode_->Open(parm),return {});

    //设定解码线程接收解封转数据
    m_demux_->set_next(m_decode_.get());

    //初始化渲染
#ifdef MACOS
    CHECK_FALSE_(Init(*parm),return {});
#else
    m_view_->Set_Win_ID(reinterpret_cast<void *>(winId()));
    CHECK_FALSE_(m_view_->Init(*parm),return {});
#endif

    m_demux_->Start();
    m_decode_->Start();
    return true;
}

#ifdef MACOS
void XCameraWidget::Draw() {

    if (!m_decode_ || !m_demux_ ) {
        return;
    }

    if (const auto f{m_decode_->CopyFrame()}) {
        Repaint(*f);
    }
}
#else
void XCameraWidget::Draw() const {

    if (!m_decode_ || !m_demux_ || !m_view_) {
        return;
    }

    if (const auto f{m_decode_->CopyFrame()}) {
        m_view_->DrawFrame(*f);
    }
}
#endif
