#include "ui_CaptureWindow.h"

#include <fplayer/widget/capturewindow.h>
#include <fplayer/service/service.h>
#include <fplayer/widget/fvideoview.h>

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

	auto container = ui->wgtView;

	// 1) 初始化摄像头
	m_service->initCamera(fplayer::MediaBackendType::Qt6);

	// 2) 绑定预览窗口
	m_service->bindCameraPreview(this->ui->wgtView);

	// 3) 获取摄像头列表
	auto cameraList = m_service->getCameraList();
	QStringList list;
	list << cameraList;
	this->ui->cmbDevices->addItems(list);

	// 4) 连接信号槽
	connect(this->ui->cmbDevices, &QComboBox::currentIndexChanged, [this](int index) {
		this->m_service->selectCamera(index);
		QStringList formats(this->m_service->getCameraFormats(index));
		this->ui->cmbFormats->clear();
		this->ui->cmbFormats->addItems(formats);
		this->ui->cmbFormats->setCurrentIndex(0);
	});

	connect(this->ui->cmbFormats, &QComboBox::currentIndexChanged, [this](int index) {
		this->m_service->selectCameraFormat(index);
	});

	// TODO 缺少摄像头格式变更的槽，以及对应调整摄像头格式的backend方法

	// 5) 选择第一个摄像头（此时预览已经设置好了）
	if (!list.isEmpty())
	{
		this->ui->cmbDevices->setCurrentIndex(0);
		this->m_service->selectCamera(0);

		QStringList formats(this->m_service->getCameraFormats(0));
		this->ui->cmbFormats->addItems(formats);
		this->ui->cmbFormats->setCurrentIndex(0);
		this->m_service->selectCameraFormat(0);
	}
}

// void CaptureWindow::refreshCameras()
// {
// 	const auto def = QMediaDevices::defaultVideoInput();
// 	qDebug() << "defaultVideoInput isNull =" << def.isNull()
// 			<< "desc =" << def.description()
// 			<< "id =" << def.id();
//
// 	m_devices = QMediaDevices::videoInputs();
//
// 	qDebug() << "Cameras found:" << m_devices.size();
// 	for (int i = 0; i < m_devices.size(); ++i)
// 	{
// 		const auto& d = m_devices[i];
// 		qDebug() << "[" << i << "]" << d.description() << d.id();
// 	}
// }
//
// bool CaptureWindow::selectCamera(int index)
// {
// 	if (index < 0 || index >= m_devices.size())
// 	{
// 		qWarning() << "selectCamera invalid index:" << index;
// 		return false;
// 	}
//
// 	if (m_camera)
// 	{
// 		m_camera->stop();
// 		m_camera->deleteLater();
// 		m_camera = nullptr;
// 	}
//
// 	m_camera = new QCamera(m_devices[index], this);
//
// 	connect(m_camera, &QCamera::errorOccurred, this,
// 	        [](QCamera::Error e, const QString& s) {
// 		        qWarning() << "Camera error:" << e << s;
// 	        });
//
// 	m_session->setCamera(m_camera);
// 	m_camera->start();
//
// 	qDebug() << "Selected camera:" << index << m_devices[index].description();
// 	return true;
// }

CaptureWindow::~CaptureWindow()
{
	delete ui;
}