/*************************************************
  * 描述：
  *
  * File：fglwidget.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/3/6
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_FGLWIDGET_H
#define FPLAYER_DESKETOP_FGLWIDGET_H

#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLFunctions>

#include <fplayer/backend/media_ffmpeg/export.h>
#include <fplayer/api/media/ifglwidget.h>

namespace fplayer
{
	class FGLWidget : public QOpenGLWidget, protected QOpenGLFunctions, public IFGLWidget
	{
		Q_OBJECT

	public:
		explicit FGLWidget(QWidget* parent = nullptr);
		~FGLWidget() = default;

	protected:
		void initializeGL() override;
		void resizeGL(int w, int h) override;
		void paintGL() override;
	};
}

#endif //FPLAYER_DESKETOP_FGLWIDGET_H