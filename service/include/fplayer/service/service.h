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
#include <fplayer/runtime/mediafactory.h>

namespace fplayer
{
	class FPLAYER_SERVICE_EXPORT Service
	{
	public:
		Service();

		~Service();

		void initCamera(MediaBackend backend = MediaBackend::Qt6);

		QList<QString> getCameraList() const;

		QList<QString> getCameraFormats(int index) const;

	private:
		std::unique_ptr<fplayer::Camera> m_camera;
	};
}

#endif //FPLAYER_DESKETOP_SERVICE_H