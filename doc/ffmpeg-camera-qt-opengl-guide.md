# CameraFFmpeg + Qt OpenGL 流程解析（教学版）

本文基于当前工程里的 `CameraFFmpeg`、`FVideoView`、`FGLWidget` 实现，讲清楚三件事：

1. FFmpeg 如何打开摄像头并读到压缩数据包  
2. FFmpeg 如何把数据包解码成 YUV 帧  
3. Qt 如何把 YUV 帧送进 `QOpenGLWidget` 并显示

---

## 1. 整体架构（先建立全局图）

当前链路可以概括为：

- 设备层：`CameraDescriptionFetcher::getDescriptions()` 枚举摄像头
- 采集解码层：`CameraFFmpeg::selectCamera()` + `captureLoop()`
- 预览容器：`FVideoView` 根据后端创建 `FGLWidget`
- 渲染层：`FGLWidget::updateYUVFrame()` -> `paintGL()`

数据流如下：

`dshow 摄像头` -> `av_read_frame(AVPacket)` -> `avcodec_send_packet/receive_frame(AVFrame)` -> `emit yuvFrameReady(...)` -> `FGLWidget::updateYUVFrame(...)` -> `OpenGL 纹理` -> `paintGL()`

---

## 2. FFmpeg 如何获取摄像头

### 2.1 初始化设备模块

在 `CameraFFmpeg` 构造函数里调用：

- `avdevice_register_all()`：注册输入设备（含 Windows 的 dshow）

并设置了日志回调：

- `av_log_set_callback(loggerCallback)`：把 FFmpeg 日志接到项目日志系统

### 2.2 选择设备并打开输入

`selectCamera(int index)` 做了以下关键动作：

1. 校验索引、停止旧采集线程
2. 构造 dshow 设备名：`video=<description>`
3. 调用 `avformat_open_input(..., av_find_input_format("dshow"), ...)`

这一步就完成了“打开 Windows 摄像头采集源”。

> 注意：dshow 的设备字符串通常要求和系统设备名严格匹配；当前代码使用 `description` 作为设备名，有些机器上可能更稳的是 `friendly name` 或唯一标识符。

### 2.3 获取流信息并定位视频流

打开输入后：

- `avformat_find_stream_info()`：探测流
- 遍历 `formatContext->streams`，找 `codec_type == AVMEDIA_TYPE_VIDEO`

找到后保存到 `m_impl->stream`，后续读包时只处理这个 stream。

---

## 3. FFmpeg 如何解码摄像头数据

### 3.1 初始化解码器

仍在 `selectCamera()` 中：

1. `avcodec_find_decoder(codec_id)` 找解码器
2. `avcodec_alloc_context3(codec)` 分配上下文
3. `avcodec_parameters_to_context(...)` 拷贝流参数
4. `avcodec_open2(...)` 打开解码器

这四步完成后，就可以“送包取帧”。

### 3.2 采集线程里循环解码

`captureLoop()` 的核心模式是 FFmpeg 标准写法：

1. `av_read_frame()` 读取一个 `AVPacket`
2. 仅处理视频流包：`packet->stream_index == m_impl->stream->index`
3. `avcodec_send_packet(codecCtx, packet)` 送入解码器
4. 循环 `avcodec_receive_frame(codecCtx, frame)` 取出 0~N 个 `AVFrame`

### 3.3 只接受 YUV420P，然后发信号给渲染层

当前实现仅接受：

- `AV_PIX_FMT_YUV420P`
- `AV_PIX_FMT_YUVJ420P`

拿到后直接发 Qt 信号：

- `emit yuvFrameReady(y, u, v, width, height, yStride, uStride, vStride)`

这里传的 `stride`（`linesize`）非常关键，渲染时必须按 stride 上传纹理，不能假设每行紧密等于 width。

---

## 4. 如何输出到 QOpenGLWidget（FGLWidget）

### 4.1 预览目标绑定

在 `setPreviewTarget()`：

- 从 `PreviewTarget.backend_hint` 拿到 `FGLWidget*`
- 连接信号槽（`QueuedConnection`）：
  - 生产者：`CameraFFmpeg::yuvFrameReady`
  - 消费者：`FGLWidget::updateYUVFrame`

`QueuedConnection` 让采集线程安全地把数据投递到 UI 线程/GL 线程上下文。

### 4.2 FVideoView 如何提供 FGLWidget

`FVideoView::previewTarget()` 中，当后端是 FFmpeg：

- 按需创建 `m_glWidget = new FGLWidget(this)`
- `t.backend_hint = static_cast<void*>(m_glWidget)`

这就是采集层拿到渲染目标的桥。

### 4.3 FGLWidget 内部渲染机制

`FGLWidget` 主要分 3 步：

1. `updateYUVFrame(...)`：复制 Y/U/V 数据到 `QByteArray`，保存宽高和 stride，调用 `update()`
2. `paintGL()`：在绘制时 `updateYUVTextures()` 上传三张纹理（Y/U/V）
3. 着色器把 YUV 转 RGB，然后 `glDrawArrays` 画满视口

片段着色器中进行了 YUV->RGB 变换矩阵计算，所以 CPU 侧避免了 swscale 转 RGB，效率更高。

---

## 5. 这份代码“不完美”在哪里（重点）

下面是我建议你优先关注的问题：

### 5.1 线程模型不够 Qt 化

当前用法是：

- `new QThread()`
- `connect(started, lambda -> captureLoop())`

但 `CameraFFmpeg` 对象本身并没有 `moveToThread`，`captureLoop()` 只是被 lambda 在新线程执行。能跑，但可维护性一般。

建议：使用“Worker QObject + moveToThread”模式，或用 `std::jthread` 管理采集循环。

### 5.2 停止逻辑存在强制 terminate 风险

`stopCapture()` 在超时后用 `QThread::terminate()`，这会导致资源状态不可控（锁、FFmpeg 对象、Qt 对象都可能处于中间态）。

建议：  
- 用 `AVFormatContext::interrupt_callback` 中断 `av_read_frame`  
- 仅用协作式退出（原子标志 + wait），避免 `terminate()`

### 5.3 资源释放可能有竞态

`stopCapture()` 会先 `avformat_close_input(&formatContext)`；若采集线程正在读 `formatContext`，有潜在并发风险。  
虽然意图是“让阻塞读尽快返回”，但要配合严格同步。

建议：把“关闭输入”放在线程退出路径里，主线程只发停止信号并等待。

### 5.4 像素格式兜底不足

很多摄像头会给 `NV12`、`YUYV422`、`MJPEG` 等。当前仅支持 YUV420P，其他格式直接丢弃。

建议：

- 增加 `libswscale` 转换到 `YUV420P`（或直接支持 NV12 shader）
- 对不支持格式打印一次警告并自动启用转换

### 5.5 OpenGL 兼容性问题（重要）

`QOpenGLTexture::LuminanceFormat`、`GL_LUMINANCE` 在 Core Profile / 新版 OpenGL 下兼容性不佳。

建议：

- 改用 `GL_R8`/`GL_RED` 单通道纹理
- shader 中读取 `.r`（你现在已经在读 `.r`，迁移成本低）

### 5.6 性能上的可优化点

- 每帧把 Y/U/V 复制到 `QByteArray`，会有额外内存拷贝
- 每帧日志较多（尤其调试模式）

可逐步优化为环形缓冲区或 PBO 上传，降低拷贝成本。

---

## 6. 你可以这样理解“最小可用闭环”

如果你要把这套流程记住，背这个 8 步即可：

1. 枚举设备（dshow）
2. `avformat_open_input` 打开摄像头
3. `avformat_find_stream_info`
4. 找视频流、开解码器
5. 子线程循环 `av_read_frame`
6. `send_packet/receive_frame` 解码出 YUV
7. Qt 信号把 YUV 送到 `FGLWidget`
8. OpenGL 三纹理 + shader 转 RGB 显示

---

## 7. 建议你的下一步学习顺序

1. 先把 `captureLoop()` 按“读包-送包-收帧”逐行断点跑通  
2. 再看 `FGLWidget::updateYUVFrame()` 与 `paintGL()` 的调用关系  
3. 最后做一个小改造：支持 `NV12 -> YUV420P`（用 `sws_scale`），你会立刻理解像素格式差异

---

## 8. 一句话总结

这份代码已经具备完整的“摄像头采集 + FFmpeg 解码 + Qt OpenGL 渲染”主链路；当前主要短板在**线程退出安全性、像素格式兼容性和 OpenGL 格式现代化**，把这三点补齐后，稳定性和可移植性会明显提升。

