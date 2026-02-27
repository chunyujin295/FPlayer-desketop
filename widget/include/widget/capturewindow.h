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
        Ui::CaptureWindow* ui;
};


#endif //FPLAYER_DESKETOP_CAPTUREWINDOW_H