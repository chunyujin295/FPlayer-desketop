/*************************************************
  * 描述：摄像头播放抽象
  *
  * File：camera.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/2/27
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_CAMERA_H
#define FPLAYER_DESKETOP_CAMERA_H

#include <QList>
#include <QString>
#include <fplayer/api/export.h>
#include <fplayer/api/media/mediabackendtype.h>
#include <fplayer/api/media/previewtarget.h>

namespace fplayer
{
	struct CameraDescription
	{
		QString description;// 摄像头描述/名称
		QString id;
		int formatIndex = 0;// 当前摄像头的格式索引
		QList<QString> formats;
	};

	class FPLAYER_API_EXPORT ICamera
	{
	public:
		virtual ~ICamera();

		virtual bool selectCamera(int index) = 0;

		virtual void refreshCameras() = 0;

		virtual QList<CameraDescription> getDescriptions() = 0;

		virtual int getIndex() = 0;

		virtual void pause() = 0;

		virtual void resume() = 0;

		virtual void setPreviewTarget(const PreviewTarget&) = 0;

		MediaBackendType getBackendType() const;

	protected:
		int m_cameraIndex = 0;
		QList<CameraDescription> m_descriptions;// 当前摄像头信息
		MediaBackendType m_backend = MediaBackendType::Qt6;
	};
}

#endif //FPLAYER_DESKETOP_CAMERA_H