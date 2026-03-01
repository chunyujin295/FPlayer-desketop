/*************************************************
  * 描述：
  *
  * File：ifvideoview.h
  * Date：2026/3/2
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_IFVIDEOVIEW_H
#define FPLAYER_DESKETOP_IFVIDEOVIEW_H
#include <fplayer/api/media/previewtarget.h>
#include <fplayer/api/export.h>

namespace fplayer
{
	class FPLAYER_API_EXPORT IFVideoView
	{
	public:
		virtual ~IFVideoView() = default;

		virtual PreviewTarget previewTarget() const = 0;
	};
}

#endif //FPLAYER_DESKETOP_IFVIDEOVIEW_H