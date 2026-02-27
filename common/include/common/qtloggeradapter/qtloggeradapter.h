/*************************************************
  * 描述：接管qDebug输出内容为Logger日志打印
  *
  * File：qtloggeradapter.h
  * Author：chenyujin@mozihealthcare.cn
  * Date：2026/2/27
  * Update：
  * ************************************************/
#ifndef FPLAYER_DESKETOP_QTLOGGERADAPTER_H
#define FPLAYER_DESKETOP_QTLOGGERADAPTER_H

#include <common/export.h>
#include <logger/logger.h>
// 防止递归：如果 spdlog 的 sink/formatter 间接触发 Qt 日志，会无限递归
static thread_local bool g_inQtHandler = false;

class FPLAYER_COMMON_EXPORT QtLoggerAdapter
{
public:
    QtLoggerAdapter();
};

#endif //FPLAYER_DESKETOP_QTLOGGERADAPTER_H
