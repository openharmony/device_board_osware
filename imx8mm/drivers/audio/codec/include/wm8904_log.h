/*
 * Copyright (C) 2021–2022 Beijing OSWare Technology Co., Ltd
 * This file contains confidential and proprietary information of
 * OSWare Technology Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

