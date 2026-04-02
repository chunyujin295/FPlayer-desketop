#include <fplayer/common/fglwidget/fglwidget.h>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLFunctions>

fplayer::FGLWidget::FGLWidget(QWidget* parent) : QOpenGLWidget(parent), QOpenGLFunctions()
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