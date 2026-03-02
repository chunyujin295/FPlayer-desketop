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
	// 摄像头变更
	connect(this->ui->cmbDevices, &QComboBox::currentIndexChanged, [this](int index) {
		this->m_service->selectCamera(index);
		QStringList formats(this->m_service->getCameraFormats(index));
		this->ui->cmbFormats->clear();
		this->ui->cmbFormats->addItems(formats);
		this->ui->cmbFormats->setCurrentIndex(0);
	});

	// 摄像头格式变更
	connect(this->ui->cmbFormats, &QComboBox::currentIndexChanged, [this](int index) {
		this->m_service->selectCameraFormat(index);
	});


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

CaptureWindow::~CaptureWindow()
{
	delete ui;
}