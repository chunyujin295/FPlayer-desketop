#include <fplayer/widget/fvideoview.h>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QtMultimedia/QVideoSink>

fplayer::FVideoView::FVideoView(QWidget* parent) : QWidget(parent)
{
	// 让这个控件有原生窗口句柄（FFmpeg/SDL/D3D 渲染需要）
	setAttribute(Qt::WA_NativeWindow, true);

	// Qt6 视频输出
	auto lay = new QVBoxLayout(this);
	lay->setContentsMargins(0,0,0,0);
	lay->setSpacing(0);
	m_qtVideoWidget = new QVideoWidget(this);
	lay->addWidget(m_qtVideoWidget);
}

fplayer::FVideoView::~FVideoView() = default;

// QVideoSink* fplayer::FVideoView::videoSink() const
// {
// 	return m_sink;
// }

fplayer::PreviewTarget fplayer::FVideoView::previewTarget() const
{
	PreviewTarget t{};

#ifdef _WIN32
	t.window.hwnd = reinterpret_cast<void*>(winId());
#else
	t.window.handle = reinterpret_cast<void*>(winId());
#endif
	t.window.width = width();
	t.window.height = height();
	t.window.device_pixel_ratio = devicePixelRatioF();

	// Qt6 backend 用
	t.backend_hint = static_cast<void*>(m_qtVideoWidget);
	return t;
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