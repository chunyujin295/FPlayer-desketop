/*************************************************
  * 描述：
  *
  * File：capturewindow.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/2/19
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_CAPTUREWINDOW_H
#define FPLAYER_DESKETOP_CAPTUREWINDOW_H

#include <QWidget>
#include <widget/export.h>


class QCameraDevice;
class QCamera;
class QMediaCaptureSession;
class QVideoWidget;
QT_BEGIN_NAMESPACE

namespace Ui
{
    class CaptureWindow;
}

QT_END_NAMESPACE

class FPLAYER_WIDGET_EXPORT CaptureWindow : public QWidget
{
    Q_OBJECT

    public:
        explicit CaptureWindow(QWidget* parent = nullptr);

        ~CaptureWindow() override;
    private:
        void refreshCameras();
        bool selectCamera(int index);

    private:
        Ui::CaptureWindow* ui;
        QVideoWidget* m_view = nullptr;
        QMediaCaptureSession* m_session = nullptr;
        QCamera* m_camera = nullptr;
        QList<QCameraDevice> m_devices;
};


#endif //FPLAYER_DESKETOP_CAPTUREWINDOW_H