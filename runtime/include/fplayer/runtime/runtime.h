/*************************************************
  * 描述：
  *
  * File：runtime.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/2/28
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_RUNTIME_H
#define FPLAYER_DESKETOP_RUNTIME_H


#include <memory>
#include <fplayer/api/media/camera.h>
#include <fplayer/api/media/previewtarget.h>
#include <fplayer/runtime/export.h>

namespace fplayer
{
	class RunTime
	{
	public:
		std::shared_ptr<Camera> FPLAYER_RUNTIME_EXPORT createCamera(MediaBackendType backend);
		void bindCameraPreview(const PreviewTarget& target);

	private:
		std::shared_ptr<Camera> m_camera;
	};


}

#endif //FPLAYER_DESKETOP_RUNTIME_H