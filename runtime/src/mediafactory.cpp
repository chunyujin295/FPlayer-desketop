#include <fplayer/runtime/mediafactory.h>
#include <fplayer/backend/media_qt6/cameraqt6.h>

std::unique_ptr<fplayer::Camera> fplayer::createCamera(MediaBackend backend)
{
    // switch (backend) {
    //     case MediaBackend::Qt6:
            return std::make_unique<fplayer::CameraQt6>();
        // case MediaBackend::FFmpeg:
        //     return std::make_unique<PlayerFFmpeg>();
    // }
}
