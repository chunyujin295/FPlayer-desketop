#include <QDebug>
#include <QThread>
#include <QPainter>
#include <QImage>
#include <QWidget>

#include <fplayer/backend/media_ffmpeg/cameraffmpeg.h>

#include "fplayer/backend/media_ffmpeg/fglwidget.h"

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
		SwsContext* swsContext = nullptr;
		uint8_t* frameBuffer = nullptr;
		int frameBufferSize = 0;
		QThread* captureThread = nullptr;
		bool isCapturing = false;
		bool isPaused = false;
		PreviewTarget previewTarget;
		FGLWidget* fGLWieget = nullptr;

		// 用于存储摄像头设备信息
		struct CameraDeviceInfo
		{
			QString name;
			QString devicePath;
			QList<QString> formats;
		};

		QList<CameraDeviceInfo> cameraDevices;

		~Impl()
		{
			stopCapture();
			cleanup();
		}

		void cleanup()
		{
			if (swsContext)
			{
				sws_freeContext(swsContext);
				swsContext = nullptr;
			}
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
			if (frameBuffer)
			{
				av_free(frameBuffer);
				frameBuffer = nullptr;
				frameBufferSize = 0;
			}
		}

		void stopCapture()
		{
			isCapturing = false;
			if (captureThread && captureThread->isRunning())
			{
				captureThread->wait();
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
		m_impl->cameraDevices.clear();

		// 使用 dshow 设备枚举摄像头
		const AVInputFormat* inputFormat = av_find_input_format("dshow");
		if (!inputFormat)
		{
			//qWarning() << "Failed to find dshow input format";
			return;
		}

		AVDeviceInfoList* deviceList = nullptr;
		int ret = avdevice_list_input_sources(inputFormat, nullptr, nullptr, &deviceList);
		if (ret < 0)
		{
			//qWarning() << "Failed to list input sources:" << av_err2str(ret);
			return;
		}

		for (int i = 0; i < deviceList->nb_devices; ++i)
		{
			AVDeviceInfo* device = deviceList->devices[i];
			if (device)
			{
				Impl::CameraDeviceInfo deviceInfo;
				deviceInfo.name = QString::fromUtf8(device->device_name);
				deviceInfo.devicePath = QString::fromUtf8(device->device_description);

				// 简化处理，假设每个摄像头有默认格式
				deviceInfo.formats << "640x480 fps: 30";
				deviceInfo.formats << "1280x720 fps: 30";

				m_impl->cameraDevices.append(deviceInfo);

				CameraDescription desc;
				desc.description = deviceInfo.name;
				desc.id = deviceInfo.devicePath;
				desc.formats = deviceInfo.formats;
				m_descriptions.append(desc);

				// qDebug() << "Found camera:" << desc.description << desc.id;
			}
		}

		avdevice_free_list_devices(&deviceList);
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
		if (index < 0 || index >= m_impl->cameraDevices.size())
		{
			//qWarning() << "Invalid camera index:" << index;
			return false;
		}

		// 停止当前捕获
		m_impl->stopCapture();
		m_impl->cleanup();

		// 打开摄像头
		const Impl::CameraDeviceInfo& deviceInfo = m_impl->cameraDevices[index];
		QString devicePath = "video=" + deviceInfo.name;

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

		// 分配帧缓冲区
		int width = m_impl->codecContext->width;
		int height = m_impl->codecContext->height;
		m_impl->frameBufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
		m_impl->frameBuffer = (uint8_t*)av_malloc(m_impl->frameBufferSize);

		// 创建 SwsContext 用于格式转换
		m_impl->swsContext = sws_getContext(width, height, m_impl->codecContext->pix_fmt,
		                                    width, height, AV_PIX_FMT_RGB24,
		                                    SWS_BILINEAR, nullptr, nullptr, nullptr);

		// 启动捕获线程
		m_impl->isCapturing = true;
		m_impl->isPaused = false;
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
		m_impl->isPaused = true;
		//qdebug() << "Camera paused";
	}

	void CameraFFmpeg::resume()
	{
		m_impl->isPaused = false;
		//qdebug() << "Camera resumed";
	}

	bool CameraFFmpeg::isPlaying() const
	{
		return !m_impl->isPaused;
	}

	void CameraFFmpeg::setPreviewTarget(const PreviewTarget& target)
	{
		m_impl->previewTarget = target;
		// 获取渲染窗口
		if (target.backend_hint)
		{
			m_impl->fGLWieget = static_cast<FGLWidget*>(target.backend_hint);
		}
		// else if (target.window.hwnd)
		// {
		// 	// 尝试通过 HWND 获取 QWidget
		// 	// 注意：这种方式在不同 Qt 版本中可能有差异
		// 	m_impl->renderWidget = QWidget::find(reinterpret_cast<WId>(target.window.hwnd));
		// }
		//qdebug() << "Preview target set, renderWidget:" << m_impl->renderWidget;
	}

	void CameraFFmpeg::captureLoop()
	{
		AVPacket* packet = av_packet_alloc();
		AVFrame* frame = av_frame_alloc();
		AVFrame* rgbFrame = av_frame_alloc();

		// 设置 RGB 帧的参数
		int width = m_impl->codecContext->width;
		int height = m_impl->codecContext->height;
		av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, m_impl->frameBuffer,
		                     AV_PIX_FMT_RGB24, width, height, 1);

		while (m_impl->isCapturing)
		{
			if (m_impl->isPaused)
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
					//qWarning() << "Error reading frame:" << av_err2str(ret);
					QThread::msleep(100);
				}
				continue;
			}

			if (packet->stream_index == m_impl->stream->index)
			{
				ret = avcodec_send_packet(m_impl->codecContext, packet);
				if (ret < 0)
				{
					//qWarning() << "Error sending packet:" << av_err2str(ret);
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
						//qWarning() << "Error receiving frame:" << av_err2str(ret);
						break;
					}

					// 转换为 RGB 格式
					sws_scale(m_impl->swsContext, frame->data, frame->linesize, 0,
					          frame->height, rgbFrame->data, rgbFrame->linesize);

					// 将 RGB 数据渲染到 Qt 窗口
					if (m_impl->fGLWieget && m_impl->fGLWieget->isVisible())
					{
						QImage image(m_impl->frameBuffer, frame->width, frame->height, QImage::Format_RGB888);
						QPainter painter(m_impl->fGLWieget);
						painter.drawImage(0, 0, image.scaled(m_impl->fGLWieget->size()));
					}
					else
					{
						//qdebug() << "Captured frame:" << frame->width << "x" << frame->height;
					}

					av_frame_unref(frame);
				}
			}

			av_packet_unref(packet);
		}

		av_frame_free(&rgbFrame);
		av_frame_free(&frame);
		av_packet_free(&packet);
	}
}