/*************************************************
  * 描述：
  *
  * File：capturewindow.cpp
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/2/19
  * Update：
  * ************************************************/
// You may need to build the project (run Qt uic code generator) to get "ui_CaptureWindow.h" resolved

#include "../include/widget/capturewindow.h"
#include "ui_CaptureWindow.h"


CaptureWindow::CaptureWindow(QWidget* parent) : QWidget(parent), ui(new Ui::CaptureWindow)
{
    ui->setupUi(this);
}

CaptureWindow::~CaptureWindow()
{
    delete ui;
}