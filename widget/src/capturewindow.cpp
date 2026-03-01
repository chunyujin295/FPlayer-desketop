#include "ui_CaptureWindow.h"

#include <fplayer/widget/capturewindow.h>
#include <fplayer/service/service.h>

#include <QVideoWidget>
#include <QVBoxLayout>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <logger/logger.h>
#include <QDebug>

CaptureWindow::CaptureWindow(QWidget* parent)
	: QWidget(parent)
	  , ui(new Ui::CaptureWindow)
{
	ui->setupUi(this);
	m_service = new fplayer::Service();

	// 1) 把视频窗口塞进 Designer 里的 wgtTop（而不是 this）
	auto container = ui->wgtTop;

	// 如果 wgtTop 没有 layout，就创建一个（让 video 填满）
	if (!container->layout())
	{
		auto layout = new QVBoxLayout(container);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		container->setLayout(layout);
	}

	m_view = new QVideoWidget(container);
	container->layout()->addWidget(m_view);

	// 2) 建立 capture session
	m_session = new QMediaCaptureSession(this);
	m_session->setVideoOutput(m_view);

	// 3) 诊断输出：默认设备 & 设备列表
	refreshCameras();

	// 4) 选择第一个摄像头
	if (!m_devices.isEmpty())
	{
		selectCamera(0);
	}

	m_service->initCamera(fplayer::MediaBackendType::Qt6);
	auto cameraList = m_service->getCameraList();
	QStringList list;
	list << "" << cameraList;
	this->ui->cmbDevices->addItems(list);
	this->ui->cmbDevices->setCurrentIndex(0);
}

void CaptureWindow::refreshCameras()
{
	const auto def = QMediaDevices::defaultVideoInput();
	qDebug() << "defaultVideoInput isNull =" << def.isNull()
			<< "desc =" << def.description()
			<< "id =" << def.id();

	m_devices = QMediaDevices::videoInputs();

	qDebug() << "Cameras found:" << m_devices.size();
	for (int i = 0; i < m_devices.size(); ++i)
	{
		const auto& d = m_devices[i];
		qDebug() << "[" << i << "]" << d.description() << d.id();
	}
}

bool CaptureWindow::selectCamera(int index)
{
	if (index < 0 || index >= m_devices.size())
	{
		qWarning() << "selectCamera invalid index:" << index;
		return false;
	}

	if (m_camera)
	{
		m_camera->stop();
		m_camera->deleteLater();
		m_camera = nullptr;
	}

	m_camera = new QCamera(m_devices[index], this);

	connect(m_camera, &QCamera::errorOccurred, this,
	        [](QCamera::Error e, const QString& s) {
		        qWarning() << "Camera error:" << e << s;
	        });

	m_session->setCamera(m_camera);
	m_camera->start();

	qDebug() << "Selected camera:" << index << m_devices[index].description();
	return true;
}

CaptureWindow::~CaptureWindow()
{
	delete ui;
}