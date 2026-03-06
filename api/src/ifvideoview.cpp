#include <fplayer/api/media/ifvideoview.h>

fplayer::IFVideoView::IFVideoView(MediaBackendType backendType): m_backendType(backendType)
{

}

void fplayer::IFVideoView::setBackendType(MediaBackendType backendType)
{
	m_backendType = backendType;
}