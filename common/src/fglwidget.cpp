#include <fplayer/common/fglwidget/fglwidget.h>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QDebug>
#include <QtAlgorithms>

namespace fplayer
{
	// YUV 渲染着色器
	static const char* vertexShaderSource = R"(
		attribute vec2 position;
		attribute vec2 texCoord;
		varying vec2 vTexCoord;
		void main()
		{
			gl_Position = vec4(position, 0.0, 1.0);
			vTexCoord = texCoord;
		}
	)";

	static const char* fragmentShaderSource = R"(
		varying vec2 vTexCoord;
		uniform sampler2D texY;
		uniform sampler2D texU;
		uniform sampler2D texV;
		
		void main()
		{
			vec3 yuv;
			yuv.x = texture2D(texY, vTexCoord).r;
			yuv.y = texture2D(texU, vTexCoord).r - 0.5;
			yuv.z = texture2D(texV, vTexCoord).r - 0.5;
			
			vec3 rgb = mat3(
				1.0, 1.0, 1.0,
				0.0, -0.39465, 2.03211,
				1.13983, -0.58060, 0.0
			) * yuv;
			
			gl_FragColor = vec4(rgb, 1.0);
		}
	)";

	FGLWidget::FGLWidget(QWidget* parent) : QOpenGLWidget(parent), QOpenGLFunctions()
	{
	}

	FGLWidget::~FGLWidget()
	{
		makeCurrent();
		
		if (m_texY)
		{
			delete m_texY;
			m_texY = nullptr;
		}
		if (m_texU)
		{
			delete m_texU;
			m_texU = nullptr;
		}
		if (m_texV)
		{
			delete m_texV;
			m_texV = nullptr;
		}
		if (m_program)
		{
			delete m_program;
			m_program = nullptr;
		}
		
		doneCurrent();
	}

	void FGLWidget::initializeGL()
	{
		initializeOpenGLFunctions();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		setupShaders();
		m_initialized = true;
	}

	void FGLWidget::resizeGL(int w, int h)
	{
		glViewport(0, 0, w, h);
	}

	void FGLWidget::paintGL()
	{
		glClear(GL_COLOR_BUFFER_BIT);

		if (!m_program)
		{
			qDebug() << "[FGLWidget::paintGL] No shader program";
			return;
		}

		if (!m_yuvData.hasData)
		{
			qDebug() << "[FGLWidget::paintGL] No YUV data";
			return;
		}

		m_program->bind();

		QMutexLocker locker(&m_mutex);

		updateYUVTextures();

		if (!m_texY || !m_texU || !m_texV)
		{
			locker.unlock();
			m_program->release();
			return;
		}

		// 绑定纹理单元
		glActiveTexture(GL_TEXTURE0);
		m_texY->bind();
		glActiveTexture(GL_TEXTURE1);
		m_texU->bind();
		glActiveTexture(GL_TEXTURE2);
		m_texV->bind();

		locker.unlock();

		// 计算等比例缩放的顶点坐标
		GLfloat vertices[16];
		calculateVertices(vertices, width(), height(), m_yuvData.width, m_yuvData.height);

		GLint positionLocation = m_program->attributeLocation("position");
		GLint texCoordLocation = m_program->attributeLocation("texCoord");
		GLint texYLocation = m_program->uniformLocation("texY");
		GLint texULocation = m_program->uniformLocation("texU");
		GLint texVLocation = m_program->uniformLocation("texV");

		glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), vertices);
		glEnableVertexAttribArray(positionLocation);
		glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), vertices + 2);
		glEnableVertexAttribArray(texCoordLocation);

		m_program->setUniformValue(texYLocation, 0);
		m_program->setUniformValue(texULocation, 1);
		m_program->setUniformValue(texVLocation, 2);

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glDisableVertexAttribArray(positionLocation);
		glDisableVertexAttribArray(texCoordLocation);

		m_program->release();
	}

	void FGLWidget::updateYUVTextures()
	{
		if (!m_yuvData.hasData)
		{
			return;
		}

		int yWidth = m_yuvData.width;
		int yHeight = m_yuvData.height;
		int uvWidth = yWidth / 2;
		int uvHeight = yHeight / 2;

		// 创建或更新 Y 纹理
		if (!m_texY || m_texY->width() != yWidth || m_texY->height() != yHeight)
		{
			delete m_texY;
			m_texY = new QOpenGLTexture(QOpenGLTexture::Target2D);
			m_texY->setMinificationFilter(QOpenGLTexture::Linear);
			m_texY->setMagnificationFilter(QOpenGLTexture::Linear);
			m_texY->setWrapMode(QOpenGLTexture::ClampToEdge);
			m_texY->setFormat(QOpenGLTexture::LuminanceFormat);
			m_texY->setSize(yWidth, yHeight);
			m_texY->allocateStorage();
		}

		// 创建或更新 U 纹理
		if (!m_texU || m_texU->width() != uvWidth || m_texU->height() != uvHeight)
		{
			delete m_texU;
			m_texU = new QOpenGLTexture(QOpenGLTexture::Target2D);
			m_texU->setMinificationFilter(QOpenGLTexture::Linear);
			m_texU->setMagnificationFilter(QOpenGLTexture::Linear);
			m_texU->setWrapMode(QOpenGLTexture::ClampToEdge);
			m_texU->setFormat(QOpenGLTexture::LuminanceFormat);
			m_texU->setSize(uvWidth, uvHeight);
			m_texU->allocateStorage();
		}

		// 创建或更新 V 纹理
		if (!m_texV || m_texV->width() != uvWidth || m_texV->height() != uvHeight)
		{
			delete m_texV;
			m_texV = new QOpenGLTexture(QOpenGLTexture::Target2D);
			m_texV->setMinificationFilter(QOpenGLTexture::Linear);
			m_texV->setMagnificationFilter(QOpenGLTexture::Linear);
			m_texV->setWrapMode(QOpenGLTexture::ClampToEdge);
			m_texV->setFormat(QOpenGLTexture::LuminanceFormat);
			m_texV->setSize(uvWidth, uvHeight);
			m_texV->allocateStorage();
		}

		// 上传 Y 数据
		m_texY->bind();
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_yuvData.yStride);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, yWidth, yHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_yuvData.yBuffer.constData());

		// 上传 U 数据
		m_texU->bind();
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_yuvData.uStride);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uvWidth, uvHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_yuvData.uBuffer.constData());

		// 上传 V 数据
		m_texV->bind();
		glPixelStorei(GL_UNPACK_ROW_LENGTH, m_yuvData.vStride);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uvWidth, uvHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_yuvData.vBuffer.constData());

		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}

	void FGLWidget::updateYUVFrame(const QByteArray& yData, const QByteArray& uData, const QByteArray& vData,
	                               int width, int height, int yStride, int uStride, int vStride)
	{
		if (yData.isEmpty() || uData.isEmpty() || vData.isEmpty() || width <= 0 || height <= 0)
		{
			qDebug() << "[FGLWidget::updateYUVFrame] Invalid parameters";
			return;
		}

		QMutexLocker locker(&m_mutex);

		// 复制 YUV 数据
		m_yuvData.width = width;
		m_yuvData.height = height;
		m_yuvData.yStride = yStride;
		m_yuvData.uStride = uStride;
		m_yuvData.vStride = vStride;

		// Y 平面: yStride * height
		m_yuvData.yBuffer = yData;
		
		// U/V 平面: uStride/vStride * (height/2)
		m_yuvData.uBuffer = uData;
		m_yuvData.vBuffer = vData;
		
		m_yuvData.hasData = true;

		// 检查 Y 数据是否有效（非零）
		bool hasValidData = false;
		for (int i = 0; i < qMin(100, m_yuvData.yBuffer.size()); ++i)
		{
			if (m_yuvData.yBuffer[i] != 0)
			{
				hasValidData = true;
				break;
			}
		}

		qDebug() << "[FGLWidget::updateYUVFrame] Frame:" << width << "x" << height
		         << "Y stride:" << yStride << "U stride:" << uStride << "V stride:" << vStride
		         << "Y size:" << m_yuvData.yBuffer.size() << "Valid data:" << hasValidData;

		update();
	}

	void FGLWidget::setupShaders()
	{
		m_program = new QOpenGLShaderProgram(this);
		
		if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource))
		{
			qDebug() << "[FGLWidget] Vertex shader error:" << m_program->log();
		}
		
		if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource))
		{
			qDebug() << "[FGLWidget] Fragment shader error:" << m_program->log();
		}
		
		if (!m_program->link())
		{
			qDebug() << "[FGLWidget] Shader link error:" << m_program->log();
		}
		else
		{
			qDebug() << "[FGLWidget] YUV shaders compiled successfully";
		}
	}

	void FGLWidget::calculateVertices(float* vertices, int windowWidth, int windowHeight, int imageWidth, int imageHeight)
	{
		// 计算窗口和图像的宽高比
		float windowAspect = static_cast<float>(windowWidth) / windowHeight;
		float imageAspect = static_cast<float>(imageWidth) / imageHeight;

		// 计算缩放后的顶点坐标
		float left, right, top, bottom;

		if (windowAspect > imageAspect)
		{
			// 窗口比图像宽，上下留黑边
			float scale = imageAspect / windowAspect;
			left = -1.0f;
			right = 1.0f;
			top = scale;
			bottom = -scale;
		}
		else
		{
			// 窗口比图像高，左右留黑边
			float scale = windowAspect / imageAspect;
			left = -scale;
			right = scale;
			top = 1.0f;
			bottom = -1.0f;
		}

		// 设置顶点数据：位置 (x, y) 和纹理坐标 (u, v)
		// 注意：OpenGL 纹理坐标原点在左下角，需要垂直翻转纹理坐标
		
		// 左下角 (位置) -> 左上角 (纹理)
		vertices[0] = left;
		vertices[1] = bottom;
		vertices[2] = 0.0f;
		vertices[3] = 0.0f;

		// 右下角 (位置) -> 右上角 (纹理)
		vertices[4] = right;
		vertices[5] = bottom;
		vertices[6] = 1.0f;
		vertices[7] = 0.0f;

		// 右上角 (位置) -> 右下角 (纹理)
		vertices[8] = right;
		vertices[9] = top;
		vertices[10] = 1.0f;
		vertices[11] = 1.0f;

		// 左上角 (位置) -> 左下角 (纹理)
		vertices[12] = left;
		vertices[13] = top;
		vertices[14] = 0.0f;
		vertices[15] = 1.0f;
	}
}
