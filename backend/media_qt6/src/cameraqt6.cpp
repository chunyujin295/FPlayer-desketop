#include <fplayer/backend/media_qt6/cameraqt6.h>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QDebug>
#include <QCamera>

namespace fplayer
{
	struct CameraQt6::Impl
	{
		QMediaCaptureSession* session = nullptr;
		QList<QCameraDevice> devices;
		QCamera* camera = nullptr;
	};

	CameraQt6::CameraQt6() : m_d(std::make_unique<Impl>())
	{
	}

	CameraQt6::~CameraQt6() = default;

	bool CameraQt6::selectCamera(int index)
	{
		if (index < 0 || index >= m_d->devices.size())
		{
			qWarning() << "selectCamera invalid index:" << index;
			return false;
		}

		if (m_d->camera)
		{
			m_d->camera->stop();
			m_d->camera->deleteLater();
			m_d->camera = nullptr;
		}

		m_d->camera = new QCamera(m_d->devices[index], this);

		connect(m_d->camera, &QCamera::errorOccurred, this,
		        [](QCamera::Error e, const QString& s) {
			        qWarning() << "Camera error:" << e << s;
		        });

		m_d->session->setCamera(m_d->camera);
		m_d->camera->start();

		qDebug() << "Selected camera:" << index << m_d->devices[index].description();
		return true;
	}

	void CameraQt6::refreshCameras()
	{
		m_descriptions.clear();
		const auto def = QMediaDevices::defaultVideoInput();
		qDebug() << "defaultVideoInput isNull =" << def.isNull()
				<< "desc =" << def.description()
				<< "id =" << def.id();

		m_d->devices = QMediaDevices::videoInputs();

		qDebug() << "Cameras found:" << m_d->devices.size();
		for (int i = 0; i < m_d->devices.size(); ++i)
		{
			const auto& dev = m_d->devices[i];
			qDebug() << "[" << i << "]" << dev.description() << dev.id();

			const auto formats = dev.videoFormats();
			qDebug() << "  formats count =" << formats.size();
			auto cameraDescriptions = fplayer::CameraDescription();
			cameraDescriptions.description = dev.description();
			cameraDescriptions.id = dev.id();

			for (const QCameraFormat& fmt: formats)
			{
				QSize res = fmt.resolution();
				float minFps = fmt.minFrameRate();
				float maxFps = fmt.maxFrameRate();

				QString format = QString("%1x%2 fps: %3-%4")
				                 .arg(res.width())
				                 .arg(res.height())
				                 .arg(minFps)
				                 .arg(maxFps);

				if (!cameraDescriptions.formats.contains(format))
				{
					qDebug() << "    "
							<< format;
					cameraDescriptions.formats.push_back(format);
				}

			}
			m_descriptions.push_back({dev.description(), dev.id(), 0, cameraDescriptions.formats});
		}
	}

	QList<CameraDescription> CameraQt6::getDescriptions()
	{
		if (m_descriptions.empty())
		{
			this->refreshCameras();
		}
		return m_descriptions;
	}

	int CameraQt6::getIndex()
	{
		return m_cameraIndex;
	}

	void CameraQt6::pause()
	{
	}

	void CameraQt6::resume()
	{
	}
}// fplayer