/*************************************************
  * 描述：
  *
  * File：fvideoview.h
  * Date：2026/3/2
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_FVIDEOVIEW_H
#define FPLAYER_DESKETOP_FVIDEOVIEW_H
#include <QWidget>
#include <fplayer/widget/export.h>

class QVideoSink;

namespace fplayer
{
	class FPLAYER_WIDGET_EXPORT FVideoView : public QWidget
	{
		Q_OBJECT
	public:
		explicit FVideoView(QWidget* parent = nullptr);
		~FVideoView() override;

		QVideoSink* videoSink() const;// 给 Qt6 backend 用

	protected:
		void showEvent(QShowEvent* e) override;// 确保 winId 可用
		void resizeEvent(QResizeEvent* e) override;

	signals:
		void nativeWindowReady();// 句柄可用（可选）
		void viewResized(int w, int h, double dpr);

	private:
		QVideoSink* m_sink = nullptr;
	};
}

#endif //FPLAYER_DESKETOP_FVIDEOVIEW_H