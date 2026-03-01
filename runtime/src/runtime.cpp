#include <fplayer/runtime/runtime.h>
#include <fplayer/backend/media_qt6/cameraqt6.h>

std::shared_ptr<fplayer::Camera> fplayer::RunTime::createCamera(MediaBackendType backend)
{
	m_camera.reset();
	// switch (backend) {
	//     case MediaBackend::Qt6:
	m_camera = std::make_shared<fplayer::CameraQt6>();
	// case MediaBackend::FFmpeg:
	//     return std::make_unique<PlayerFFmpeg>();
	// }
	return m_camera;
}

void fplayer::RunTime::bindCameraPreview(const PreviewTarget& target)
{
	if (!m_camera)
	{
		return;
	}
	this->m_camera->setPreviewTarget(target);
}