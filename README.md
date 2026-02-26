

<div><center><img src="https://map--depot.oss-cn-hangzhou.aliyuncs.com/image/LOGO_WORD_CUTED.png" alt="LOGO_WORD_CUTED" style="zoom:50%;" /></center></div>

<div><center><img src="https://map--depot.oss-cn-hangzhou.aliyuncs.com/image/LOGO_ICON_CUTED.png" alt="LOGO_ICON_CUTED" style="zoom:33%;" /></center></div>

# 产品

## 基本介绍

**跨平台局域网多输入源的流媒体播放系统**

1. 桌面端，支持播放本地视频、捕获摄像头画面(包括虚拟摄像头)、屏幕共享，局域网环境下进行推流与拉流。
2. 安卓端，仅播放功能，但是局域网内可以实现拉流。

## 详细介绍

# 技术

## 技术栈





## 技术选型

一开始想要使用的是GStreamer框架 + webRtc，其中ffmpeg作为插件集成进入GStreamer。但是GStreamer学习成本可能稍高，且Windows下部署存在一定难度。因此当前想法为：跨平台+部署方便为第一优先级。

### 0.编解码直接使用Qt音视频库

编解码：Qt 音视频库（Qt6）

传输层：

UI：Qt widget

### 1.更容易部署的角度

编解码：ffmpeg

传输层：RTSP/RTP

UI：Qt widget

### 2.网络更好的角度

编解码：ffmpeg

传输层：原生WebRTC（C++栈）

UI：Qt widget

> 先实现1，后续向2进行发展

```
                            ███████╗██████╗ ██╗      █████╗ ██╗   ██╗███████╗██████╗ 
                            ██╔════╝██╔══██╗██║     ██╔══██╗╚██╗ ██╔╝██╔════╝██╔══██╗
                            █████╗  ██████╔╝██║     ███████║ ╚████╔╝ █████╗  ██████╔╝
                            ██╔══╝  ██╔═══╝ ██║     ██╔══██║  ╚██╔╝  ██╔══╝  ██╔══██╗
                            ██║     ██║     ███████╗██║  ██║   ██║   ███████╗██║  ██║
                            ╚═╝     ╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝

```



![build.png](build.png)

