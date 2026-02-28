#include <fplayer/backend/media_qt6/cameraqt6.h>

namespace fplayer
{
    CameraQt6::CameraQt6() {}

    bool CameraQt6::selectCamera(int index)
    {
        return true;
    }

    void CameraQt6::refreshCameras() {}

    QList<QString> CameraQt6::getCameras()
    {
        return m_cameras;
    }

    CameraDescription CameraQt6::getDescription()
    {
        return m_description;
    }

    int CameraQt6::getIndex()
    {
        return m_index;
    }

    void CameraQt6::pause() {}
    void CameraQt6::resume() {}
} // fplayer
