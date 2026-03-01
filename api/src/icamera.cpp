#include <fplayer/api/media/icamera.h>
fplayer::ICamera::~ICamera() = default;

fplayer::MediaBackendType fplayer::ICamera::getBackendType() const
{
	return m_backend;
}