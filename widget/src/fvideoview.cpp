#include <fplayer/widget/fvideoview.h>
#include <QtMultimedia/QVideoSink>

fplayer::FVideoView::FVideoView(QWidget* parent) : QWidget(parent)
{
	// 让这个控件有原生窗口句柄（FFmpeg/SDL/D3D 渲染需要）
	setAttribute(Qt::WA_NativeWindow, true);

	// Qt6 视频输出走 sink
	m_sink = new QVideoSink(this);
}

fplayer::FVideoView::~FVideoView() = default;

QVideoSink* fplayer::FVideoView::videoSink() const
{
	return m_sink;
}

void fplayer::FVideoView::showEvent(QShowEvent* e)
{
	QWidget::showEvent(e);
	// 触发/确保句柄创建
	(void)winId();
	emit nativeWindowReady();
}

void fplayer::FVideoView::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);
	emit viewResized(width(), height(), devicePixelRatioF());
}