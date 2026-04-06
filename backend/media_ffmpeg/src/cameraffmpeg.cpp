#include <QDebug>
#include <QThread>
#include <QPainter>
#include <QImage>
#include <QWidget>

#include <fplayer/backend/media_ffmpeg/cameraffmpeg.h>

#include "fplayer/backend/media_ffmpeg/camerainfofetcher.h"
#include "fplayer/common/fglwidget/fglwidget.h"

#include <logger/logger.h>

// FFmpeg 设备相关头文件
extern "C" {
#include <libavdevice/avdevice.h>
}

namespace fplayer
{
	// 可选：为线程安全做准备
	static std::mutex log_mutex;

	static void loggerCallback(void* ptr, int level, const char* fmt, va_list vargs)
	{
		// 过滤等级（关键）
		if (level > av_log_get_level())
		{
			return;
		}
		static char message[2048];
		std::lock_guard<std::mutex> lock(log_mutex);

		vsnprintf(message, sizeof(message), fmt, vargs);

		// 可选：过滤掉结尾的换行
		std::string str(message);
		if (!str.empty() && str.back() == '\n')
		{
			str.pop_back();
		}

		// 将 FFmpeg 的日志级别转换为你自己的日志级别
		if (level <= AV_LOG_PANIC || level == AV_LOG_FATAL)
		{
			LOG_CRITI("[ffmpeg]", str);
		}
		else if (level <= AV_LOG_ERROR)
		{
			LOG_ERROR("[ffmpeg]", str);
		}
		else if (level <= AV_LOG_WARNING)
		{
			LOG_WARN("[ffmpeg]", str);
		}
		else if (level <= AV_LOG_INFO)
		{
			LOG_INFO("[ffmpeg]", str);
		}
		else if (level <= AV_LOG_VERBOSE)
		{
			LOG_DEBUG("[ffmpeg]", str);
		}
		else if (level <= AV_LOG_DEBUG)
		{
			LOG_DEBUG("[ffmpeg]", str);
		}
		else
		{
			LOG_TRACE("[ffmpeg]", str);
		}
	}


	struct CameraFFmpeg::Impl
	{
		AVFormatContext* formatContext = nullptr;
		AVCodecContext* codecContext = nullptr;
		AVStream* stream = nullptr;
		QThread* captureThread = nullptr;
		bool isCapturing = false;
		PreviewTarget previewTarget;
		FGLWidget* fGLWieget = nullptr;

		~Impl()
		{
			stopCapture();
			cleanup();
		}

		void cleanup()
		{
			if (codecContext)
			{
				avcodec_free_context(&codecContext);
				codecContext = nullptr;
			}
			if (formatContext)
			{
				avformat_close_input(&formatContext);
				formatContext = nullptr;
			}
		}

		void stopCapture()
		{
			isCapturing = false;

			// 中断阻塞的 av_read_frame
			if (formatContext)
			{
				avformat_close_input(&formatContext);
				formatContext = nullptr;
			}

			if (captureThread && captureThread->isRunning())
			{
				captureThread->wait(3000);// 等待最多3秒
				if (captureThread->isRunning())
				{
					captureThread->terminate();// 强制终止
					captureThread->wait();
				}
				delete captureThread;
				captureThread = nullptr;
			}
		}
	};

	CameraFFmpeg::CameraFFmpeg() : m_impl(new Impl)
	{
		m_backend = MediaBackendType::FFmpeg;
		// 注册 FFmpeg 设备
		avdevice_register_all();

		av_log_set_level(AV_LOG_FATAL);
		av_log_set_callback(loggerCallback);// 指定ffmpeg日志输出到logger
	}

	CameraFFmpeg::~CameraFFmpeg()
	{
		delete m_impl;
	}

	void CameraFFmpeg::refreshCameras()
	{
		m_descriptions.clear();

		m_descriptions = CameraDescriptionFetcher::getDescriptions();
	}

	QList<CameraDescription> CameraFFmpeg::getDescriptions()
	{
		if (m_descriptions.empty())
		{
			refreshCameras();
		}
		return m_descriptions;
	}

	int CameraFFmpeg::getIndex()
	{
		return m_cameraIndex;
	}

	bool CameraFFmpeg::selectCamera(int index)
	{
		// if (index < 0 || index >= m_impl->cameraDevices.size())
		if (index < 0 || index >= m_descriptions.size())
		{
			//qWarning() << "Invalid camera index:" << index;
			return false;
		}

		// 停止当前捕获
		m_impl->stopCapture();
		m_impl->cleanup();

		// 打开摄像头
		// const Impl::CameraDeviceInfo& deviceInfo = m_impl->cameraDevices[index];
		const auto deviceInfo = m_descriptions[index];
		// QString devicePath = "video=" + deviceInfo.name;
		QString devicePath = "video=" + deviceInfo.description;

		int ret = avformat_open_input(&m_impl->formatContext, devicePath.toUtf8().constData(),
		                              av_find_input_format("dshow"), nullptr);
		if (ret < 0)
		{
			//qWarning() << "Failed to open camera:" << av_err2str(ret);
			return false;
		}

		// 查找视频流
		ret = avformat_find_stream_info(m_impl->formatContext, nullptr);
		if (ret < 0)
		{
			//qWarning() << "Failed to find stream info:" << av_err2str(ret);
			m_impl->cleanup();
			return false;
		}

		// 找到视频流索引
		int videoStreamIndex = -1;
		for (unsigned int i = 0; i < m_impl->formatContext->nb_streams; ++i)
		{
			if (m_impl->formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				videoStreamIndex = i;
				break;
			}
		}

		if (videoStreamIndex == -1)
		{
			//qWarning() << "No video stream found";
			m_impl->cleanup();
			return false;
		}

		m_impl->stream = m_impl->formatContext->streams[videoStreamIndex];
		AVCodecParameters* codecParams = m_impl->stream->codecpar;

		// 找到解码器
		const AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
		if (!codec)
		{
			//qWarning() << "Failed to find decoder";
			m_impl->cleanup();
			return false;
		}

		// 打开解码器
		m_impl->codecContext = avcodec_alloc_context3(codec);
		if (!m_impl->codecContext)
		{
			//qWarning() << "Failed to allocate codec context";
			m_impl->cleanup();
			return false;
		}

		ret = avcodec_parameters_to_context(m_impl->codecContext, codecParams);
		if (ret < 0)
		{
			//qWarning() << "Failed to copy codec parameters:" << av_err2str(ret);
			m_impl->cleanup();
			return false;
		}

		ret = avcodec_open2(m_impl->codecContext, codec, nullptr);
		if (ret < 0)
		{
			//qWarning() << "Failed to open codec:" << av_err2str(ret);
			m_impl->cleanup();
			return false;
		}

		// 启动捕获线程
		m_impl->isCapturing = true;
		m_isPlaying = true;
		m_impl->captureThread = new QThread();
		QObject::connect(m_impl->captureThread, &QThread::started, [this]() {
			captureLoop();
		});
		m_impl->captureThread->start();

		m_cameraIndex = index;
		//qdebug() << "Selected camera:" << deviceInfo.name;
		return true;
	}

	bool CameraFFmpeg::selectCameraFormat(int index)
	{
		// 简化处理，这里只是返回 true
		// 实际项目中需要根据索引设置不同的分辨率和帧率
		//qdebug() << "Selected camera format index:" << index;
		return true;
	}

	void CameraFFmpeg::pause()
	{
		// m_impl->isPaused = true;
		this->m_isPlaying = false;
		qDebug() << "Camera paused";
	}

	void CameraFFmpeg::resume()
	{
		// m_impl->isPaused = false;
		this->m_isPlaying = true;
		qDebug() << "Camera resumed";
	}

	bool CameraFFmpeg::isPlaying() const
	{
		// return !m_impl->isPaused;
		return m_isPlaying;
	}

	void CameraFFmpeg::setPreviewTarget(const PreviewTarget& target)
	{
		m_impl->previewTarget = target;
		// 获取渲染窗口
		if (target.backend_hint)
		{
			m_impl->fGLWieget = static_cast<FGLWidget*>(target.backend_hint);
			auto glWidget = static_cast<FGLWidget*>(target.backend_hint);
			// 使用 YUV 直接渲染，更高效
			bool connected = QObject::connect(this, &CameraFFmpeg::yuvFrameReady,
			                                  glWidget, &FGLWidget::updateYUVFrame, Qt::QueuedConnection);
			qDebug() << "[Runtime] YUV signal connection established:" << connected;
		}
		else
		{
			qDebug() << "[Runtime] Failed to connect: ffmpegCamera=" << this << "backend_hint=" << target.backend_hint;
		}
	}

	void CameraFFmpeg::captureLoop()
	{
		AVPacket* packet = av_packet_alloc();
		AVFrame* frame = av_frame_alloc();

		while (m_impl->isCapturing)
		{
			if (!m_isPlaying)
			{
				QThread::msleep(100);
				continue;
			}

			int ret = av_read_frame(m_impl->formatContext, packet);
			if (ret < 0)
			{
				if (ret == AVERROR_EOF)
				{
					// 流结束，重新开始
					avformat_seek_file(m_impl->formatContext, -1, 0, 0, 0, 0);
				}
				else
				{
					QThread::msleep(100);
				}
				continue;
			}

			if (packet->stream_index == m_impl->stream->index)
			{
				ret = avcodec_send_packet(m_impl->codecContext, packet);
				if (ret < 0)
				{
					av_packet_unref(packet);
					continue;
				}

				while (ret >= 0)
				{
					ret = avcodec_receive_frame(m_impl->codecContext, frame);
					if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
					{
						break;
					}
					else if (ret < 0)
					{
						break;
					}

					// 直接发射 YUV 数据，避免 CPU 转换
					// FFmpeg 解码后的帧通常是 YUV420P 格式
					if (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUVJ420P)
					{
						const int yStride = frame->linesize[0];
						const int uStride = frame->linesize[1];
						const int vStride = frame->linesize[2];
						const int width = frame->width;
						const int height = frame->height;
						const int uvHeight = height / 2;

						QByteArray yBuffer(reinterpret_cast<const char*>(frame->data[0]), yStride * height);
						QByteArray uBuffer(reinterpret_cast<const char*>(frame->data[1]), uStride * uvHeight);
						QByteArray vBuffer(reinterpret_cast<const char*>(frame->data[2]), vStride * uvHeight);

						static int frameCount = 0;
						if (++frameCount % 30 == 0)  // 每30帧输出一次
						{
							qDebug() << "[CameraFFmpeg] Emitting YUV frame:" << width << "x" << height
							         << "Y stride:" << yStride << "format:" << frame->format;
						}
						emit yuvFrameReady(
							yBuffer,
							uBuffer,
							vBuffer,
							width,
							height,
							yStride,
							uStride,
							vStride
						);
					}
					else
					{
						qDebug() << "[CameraFFmpeg] Unsupported pixel format:" << frame->format;
					}

					av_frame_unref(frame);
				}
			}

			av_packet_unref(packet);
		}

		av_frame_free(&frame);
		av_packet_free(&packet);
	}
}