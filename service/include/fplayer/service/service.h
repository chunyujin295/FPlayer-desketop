/*************************************************
  * 描述：
  *
  * File：service.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/2/28
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_SERVICE_H
#define FPLAYER_DESKETOP_SERVICE_H
#include <fplayer/service/export.h>
#include <fplayer/runtime/runtime.h>

class QWidget;

namespace fplayer
{
	class FPLAYER_SERVICE_EXPORT Service
	{
	public:
		Service();

		~Service();

		void initCamera(MediaBackendType backend);

		/**
		 * 初始化摄像头视频播放窗口
		 * @param widget
		 */
		void bindCameraPreview(fplayer::FVideoView* widget);// TODO service层不能调用widget层

		QList<QString> getCameraList() const;

		QList<QString> getCameraFormats(int index) const;

	private:
		// void bindCameraPreviewQt6(QWidget* widget);
		// void bindCameraPreviewFFmpeg(QWidget* widget);

	private:
		fplayer::RunTime* m_runtime;
		std::shared_ptr<fplayer::Camera> m_camera;
		int m_cameraIndex;// 摄像头索引
	};
}

#endif //FPLAYER_DESKETOP_SERVICE_H