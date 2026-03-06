/*************************************************
  * 描述：一个openwidget的抽象类，用于ffmpeg输出，需要后端继承实现
  *
  * File：ifglwidget.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/3/6
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_IFGLWIDGET_H
#define FPLAYER_DESKETOP_IFGLWIDGET_H

// #include <QtOpenGLWidgets/QOpenGLWidget>
// #include <QOpenGLFunctions>
#include <fplayer/api/export.h>

namespace fplayer
{
	class FPLAYER_API_EXPORT IFGLWidget//: public QOpenGLWidget, protected QOpenGLFunctions
	{
		//Q_OBJECT
	public:
		virtual ~IFGLWidget() = default;
	};
}

#endif //FPLAYER_DESKETOP_IFGLWIDGET_H