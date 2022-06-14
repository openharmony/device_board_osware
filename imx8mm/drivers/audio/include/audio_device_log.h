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

#ifndef AUDIO_DEVICE_LOG_H
#define AUDIO_DEVICE_LOG_H
#include "hdf_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define AUDIO_DEVICE_LOG_ERR(fmt, arg...) do { \
    HDF_LOGE("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define AUDIO_DEVICE_LOG_INFO(fmt, arg...) do { \
    HDF_LOGI("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define AUDIO_DEVICE_LOG_WARNING(fmt, arg...) do { \
    HDF_LOGW("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define AUDIO_DEVICE_LOG_DEBUG(fmt, arg...) do { \
    HDF_LOGD("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* AUDIO_DEVICE_LOG_H */
