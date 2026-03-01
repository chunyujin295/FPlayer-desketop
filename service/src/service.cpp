#include <fplayer/service/service.h>
#include <fplayer/common/maplist/maplist.hpp>

#include <logger/logger.h>

fplayer::Service::Service() = default;

fplayer::Service::~Service() = default;

void fplayer::Service::initCamera(MediaBackendType backend)
{
	this->m_camera = fplayer::createCamera(backend);
	if (this->m_camera == nullptr)
	{
		LOG_WARN("fplayer::Service::initCamera(MediaBackend backend) ==> 摄像头获取失败");
	}
}

void fplayer::Service::bindCameraPreview(QWidget* widget)
{
	switch (this->m_camera->getBackendType())
	{
	case MediaBackendType::Qt6:
		this->bindCameraPreviewQt6(widget);
		break;
	case MediaBackendType::FFmpeg:
		this->bindCameraPreviewFFmpeg(widget);
		break;
	}
}

QList<QString> fplayer::Service::getCameraList() const
{
	QList<QString> cameraList;
	if (this->m_camera == nullptr)
	{
		return cameraList;
	}

	const auto cameraDescriptions = this->m_camera->getDescriptions();
	cameraList = mapList(cameraDescriptions, [](const CameraDescription& description) {
		return description.description;
	});

	return cameraList;
}

QList<QString> fplayer::Service::getCameraFormats(int index) const
{
	if (this->m_camera == nullptr)
	{
		return {};
	}

	const auto cameraDescriptions = this->m_camera->getDescriptions();
	if (index < 0 || index >= cameraDescriptions.size())
	{
		return {};
	}
	return cameraDescriptions.at(index).formats;
}

void fplayer::Service::bindCameraPreviewQt6(QWidget* widget)
{
}

void fplayer::Service::bindCameraPreviewFFmpeg(QWidget* widget)
{
}