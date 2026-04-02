/*************************************************
  * 描述：
  *
  * File：fglwidget.h
  * Date：2026/3/6
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_FGLWIDGET_H
#define FPLAYER_DESKETOP_FGLWIDGET_H

#include <fplayer/common/export.h>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLFunctions>

namespace fplayer
{
	class FPLAYER_COMMON_EXPORT FGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
	{
		Q_OBJECT
	public:
		explicit FGLWidget(QWidget* parent = nullptr);

	protected:
		void initializeGL() override;
		void resizeGL(int w, int h) override;
		void paintGL() override;
	};
}
#endif //FPLAYER_DESKETOP_FGLWIDGET_H