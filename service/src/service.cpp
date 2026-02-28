#include <fplayer/service/service.h>
#include <fplayer/common/maplist/maplist.hpp>

#include <logger/logger.h>

fplayer::Service::Service() = default;

fplayer::Service::~Service() = default;

void fplayer::Service::initCamera(MediaBackend backend)
{
	this->m_camera = fplayer::createCamera(backend);
	if (this->m_camera == nullptr)
	{
		LOG_WARN("fplayer::Service::initCamera(MediaBackend backend) ==> 摄像头获取失败");
	}
}

QList<QString> fplayer::Service::getCameraList() const
{
	QList<QString> cameraList;
	if (this->m_camera == nullptr)
	{
		return cameraList;
	}

	auto cameraDescriptions = this->m_camera->getDescriptions();
	cameraList = mapList(cameraDescriptions, [](const CameraDescription& description) {
		return description.description;
	});

	return cameraList;
}