#ifndef WM8904_CODEC_LOG_H
#define WM8904_CODEC_LOG_H
#include "hdf_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define WM8904_CODEC_LOG_ERR(fmt, arg...) do { \
    HDF_LOGE("CODEC[%s]: " fmt, __func__, ##arg); \
    } while (0)

#define WM8904_CODEC_LOG_INFO(fmt, arg...) do { \
    HDF_LOGI("CODEC[%s]: " fmt, __func__, ##arg); \
    } while (0)

#define WM8904_CODEC_LOG_WARNING(fmt, arg...) do { \
    HDF_LOGW("CODEC[%s]: " fmt, __func__, ##arg); \
    } while (0)

#define WM8904_CODEC_LOG_DEBUG(fmt, arg...) do { \
    HDF_LOGD("CODEC[%s]: " fmt, __func__, ##arg); \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* WM8904_CODEC_LOG_H */

