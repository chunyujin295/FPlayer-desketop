/*************************************************
  * 描述：摄像头播放抽象
  *
  * File：camera.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/2/27
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_CAMERA_H
#define FPLAYER_DESKETOP_CAMERA_H
#include <qlist.h>
#include <string>
#include <vector>
#include <fplayer/api/export.h>

namespace fplayer
{
    struct CameraDescription
    {
        QString name;
        QString resolution;
        QString fps;
    };

    class  FPLAYER_API_EXPORT Camera
    {
        public:
            virtual ~Camera();

            virtual bool selectCamera(int index) = 0;

            virtual void refreshCameras() = 0;

            virtual QList<QString> getCameras() = 0;

            virtual CameraDescription getDescription() = 0;

            virtual int getIndex() = 0;

            virtual void pause() = 0;

            virtual void resume() = 0;

        protected:
            QList<QString> m_cameras;
            QList<QString> m_res; // 摄像头分辨率
            QList<QString> m_fps; // 摄像头帧率
            int m_index = 0;
            CameraDescription m_description; // 当前摄像头信息
    };
}

#endif //FPLAYER_DESKETOP_CAMERA_H
