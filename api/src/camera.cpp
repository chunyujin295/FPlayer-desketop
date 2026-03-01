#include <fplayer/api/media/camera.h>
fplayer::Camera::~Camera() = default;

fplayer::MediaBackendType fplayer::Camera::getBackendType() const
{
	return m_backend;
}