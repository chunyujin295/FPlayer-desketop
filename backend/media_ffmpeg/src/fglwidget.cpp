#include <fplayer/backend/media_ffmpeg/fglwidget.h>

fplayer::FGLWidget::FGLWidget(QWidget* parent)
{
}

void fplayer::FGLWidget::initializeGL()
{
	QOpenGLWidget::initializeGL();
}

void fplayer::FGLWidget::resizeGL(int w, int h)
{
	QOpenGLWidget::resizeGL(w, h);
}

void fplayer::FGLWidget::paintGL()
{
	QOpenGLWidget::paintGL();
}