/*************************************************
  * 描述：
  *
  * File：cameraqt6.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/2/28
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_CAPTUREQT6_H
#define FPLAYER_DESKETOP_CAPTUREQT6_H
#include <fplayer/backend/media_qt6/export.h>
#include <fplayer/api/media/camera.h>
#include <QList>
#include <QObject>
#include <QString>

namespace fplayer
{
    class FPLAYER_BACKEND_MEDIA_QT6_EXPORT CameraQt6 : public QObject, public Camera // QObject的继承必须放在第一位
    {
            Q_OBJECT

        public:
            CameraQt6();

            ~CameraQt6() override;

            bool selectCamera(int index) override;

            void refreshCameras() override;

            QList<CameraDescription> getDescriptions() override;

            int getIndex() override;

            void pause() override;

            void resume() override;

    	void setPreviewTarget(const PreviewTarget&) override;

        private:
            struct Impl;
            std::unique_ptr<Impl> m_d;
    };
} // fplayer

#endif //FPLAYER_DESKETOP_CAPTUREQT6_H
