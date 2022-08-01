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

#include <linux/dma-mapping.h>

#include "imx8mm_platform.h"
#include "imx8mm_common.h"
#include "audio_platform_base.h"
#include "audio_sapm.h"
#include "audio_driver_log.h"
#include "osal_io.h"
#include "osal_uaccess.h"
#include "osal_time.h"
#include "dma_driver.h"

#define RATE_48000  (48000)
#define RATE_44100  (44100)
#define RATE_24576000  (24576000)
#define RATE_22579200  (22579200)
#define HDF_LOG_TAG imx8mm_platform_ops
int32_t g_dmaRequestChannel = 0;

int32_t Imx8mmDmaBufAlloc(struct PlatformData *data, const enum AudioStreamType streamType)
{
    (void)data;
    (void)streamType;

    return HDF_SUCCESS;
}

int32_t Imx8mmDmaBufFree(struct PlatformData *data, const enum AudioStreamType streamType)
{
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("data is null");
        return HDF_FAILURE;
    }

    if (streamType == AUDIO_CAPTURE_STREAM) {
        if (data->captureBufInfo.virtAddr != NULL) {
            (void)DMADeinitRxBuff(data);
        }
    } else if (streamType == AUDIO_RENDER_STREAM) {
        if (data->renderBufInfo.virtAddr != NULL) {
            (void)DMADeinitTxBuff(data);
        }
    } else {
        AUDIO_DRIVER_LOG_ERR("stream Type is invalude.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

#define BYTE_NUM  (2)
int32_t  Imx8mmDmaRequestChannel(const struct PlatformData *data, const enum AudioStreamType streamType)
{
    int32_t ret;
    uint32_t fmt = 0, freq = 0, clkId = 0, rate = 0, bitWidth = 0;
    int channel = 0;
    g_dmaRequestChannel = false;

    if (data == NULL) {
        return HDF_FAILURE;
    }

    if (streamType == AUDIO_RENDER_STREAM) {
        channel = data->renderPcmInfo.channels;
        rate = data->renderPcmInfo.rate;
        bitWidth = data->renderPcmInfo.bitWidth;
    } else if (streamType == AUDIO_CAPTURE_STREAM) {
        channel = data->capturePcmInfo.channels;
        rate = data->capturePcmInfo.rate;
        bitWidth = data->capturePcmInfo.bitWidth;
    } else {
        return HDF_FAILURE;
    }

    fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_NB_NF;
    if (rate == RATE_48000) {
        freq = RATE_24576000;
    } else if (rate == RATE_44100) {
        freq = RATE_22579200;
    }

    clkId = FSL_SAI_CLK_MAST1;

    ret = SaiRuntimeResume(data);
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    ret = SaiSetDaiFmt(data, fmt);
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    ret = SaiSetDaiTdmSlot(data, 0, 0, BYTE_NUM, bitWidth);
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    ret = SaiSetSysclk(data, clkId, freq, SOC_CLOCK_OUT);
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    ret = SaiSetHwParams(data, streamType);
    if (ret != HDF_SUCCESS) {
        return ret;
    }

    g_dmaRequestChannel = true;
    return HDF_SUCCESS;
}

int32_t Imx8mmDmaConfigChannel(const struct PlatformData *data, const enum AudioStreamType streamType)
{
    int32_t ret;
    struct PlatformData *platformData;

    platformData = (struct PlatformData *)data;

    if (streamType == AUDIO_CAPTURE_STREAM) {
        if (platformData->captureBufInfo.virtAddr == NULL) {
            ret = DMAInitRxBuff(platformData);
            if (ret != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("DMAAiInit: fail");
                return HDF_FAILURE;
            }
        }
    } else if (streamType == AUDIO_RENDER_STREAM) {
        if (data->renderBufInfo.virtAddr == NULL) {
            ret = DMAInitTxBuff((struct PlatformData *)data);
            if (ret != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("DMAAoInit: fail");
                return HDF_FAILURE;
            }
        }
        ret = DMAEnableTx(data);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("DMAEnableTx failed");
            return HDF_FAILURE;
        }
    } else {
        AUDIO_DRIVER_LOG_ERR("stream Type is invalude.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t Imx8mmDmaPrep(const struct PlatformData *data, const enum AudioStreamType streamType)
{
    (void)data;
    (void)streamType;
    return HDF_SUCCESS;
}

int32_t Imx8mmDmaSubmit(const struct PlatformData *data, const enum AudioStreamType streamType)
{
    int ret = 0;
    struct PlatformData *platformData;

    platformData = (struct PlatformData *)data;

    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("data is null");
        return HDF_FAILURE;
    }

    if (streamType == AUDIO_CAPTURE_STREAM) {
        ret = DMAEnableRx(data);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("DMAEnableRx failed");
            return HDF_FAILURE;
        }
    } else if (streamType == AUDIO_RENDER_STREAM) {
        // delete dmaEnable because 1.after pause remalloc DMA buffer 2.DMA process must before sai trigger
    } else {
        AUDIO_DRIVER_LOG_ERR("stream Type is invalude.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t Imx8mmDmaPending(struct PlatformData *data, const enum AudioStreamType streamType)
{
    (void)data;
    (void)streamType;
    return HDF_SUCCESS;
}

int32_t Imx8mmDmaPause(struct PlatformData *data, const enum AudioStreamType streamType)
{
    struct PrivPlatformData *ppd = NULL;

    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("data is null");
        return HDF_FAILURE;
    }

    ppd = (struct PrivPlatformData *)data->dmaPrv;

    if (streamType == AUDIO_CAPTURE_STREAM) {
        DMAPauseRx(data);
    } else if (streamType == AUDIO_RENDER_STREAM) {
        data->renderBufInfo.runStatus = 0;
        // change Disable DMA becasue 1.after pause remalloc DMA buffer
        DMADisableTx(data);
        ppd->tx_dma_pos = 0;
    } else {
        AUDIO_DRIVER_LOG_ERR("stream Type is invalude.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t Imx8mmDmaResume(const struct PlatformData *data, const enum AudioStreamType streamType)
{
    int ret = 0;

    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("data is null");
        return HDF_FAILURE;
    }

    if (streamType == AUDIO_CAPTURE_STREAM) {
        ret = DMAEnableRx(data);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("DMAEnableRx failed");
            return HDF_FAILURE;
        }
    } else if (streamType == AUDIO_RENDER_STREAM) {
        ret = DMAEnableTx(data);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("DMAEnableTx failed");
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

int32_t Imx8mmDmaPointer(struct PlatformData *data, const enum AudioStreamType streamType, uint32_t *pointer)
{
    struct PrivPlatformData *ppd = NULL;

    if (data == NULL || pointer == NULL) {
        AUDIO_DRIVER_LOG_ERR("data is null");
        return HDF_FAILURE;
    }

    ppd = (struct PrivPlatformData *)data->dmaPrv;

    if (streamType == AUDIO_RENDER_STREAM) {
        OsalMutexLock(&data->renderBufInfo.buffMutex);
        *pointer = (ppd->tx_dma_pos) / (data->renderPcmInfo.frameSize);
        OsalMutexUnlock(&data->renderBufInfo.buffMutex);
    } else if (streamType == AUDIO_CAPTURE_STREAM) {
        OsalMutexLock(&data->captureBufInfo.buffMutex);
        *pointer = (ppd->rx_dma_pos) / (data->capturePcmInfo.frameSize);
        OsalMutexUnlock(&data->captureBufInfo.buffMutex);
    } else {
        AUDIO_DRIVER_LOG_ERR("stream Type is fail.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}
