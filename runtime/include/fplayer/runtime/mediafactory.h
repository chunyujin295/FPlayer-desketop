/*************************************************
  * 描述：多媒体工厂
  *
  * File：camerafactory.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/2/28
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_CAMERAFACTORY_H
#define FPLAYER_DESKETOP_CAMERAFACTORY_H

#include <memory>
#include <fplayer/api/media/camera.h>
#include <fplayer/runtime/export.h>

namespace fplayer
{

	std::unique_ptr<Camera> FPLAYER_RUNTIME_EXPORT createCamera(MediaBackendType backend);

}

#endif //FPLAYER_DESKETOP_CAMERAFACTORY_H