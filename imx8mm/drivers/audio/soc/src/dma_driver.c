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

#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/time.h>
#include <linux/delay.h>

#include "audio_driver_log.h"
#include "audio_platform_if.h"
#include "imx8mm_platform.h"
#include "sai_driver.h"
#include "osal_mem.h"
#include "imx8mm_common.h"

#define HDF_LOG_TAG imx8mm_dma_driver
static void DMATxComplete(void *arg)
{
    struct dma_tx_state state;
    enum dma_status status;

    struct PlatformData *platformData = (struct PlatformData *)arg;
    struct PrivPlatformData *ppd = (struct PrivPlatformData *)platformData->dmaPrv;

    ppd->tx_dma_pos += platformData->renderBufInfo.periodSize;
    if (ppd->tx_dma_pos == platformData->renderBufInfo.cirBufSize) {
        ppd->tx_dma_pos = 0;
    }
#ifdef RING_BUFFER_DEBUG
    AUDIO_DRIVER_LOG_INFO("dma tx complete. pos:%x", ppd->tx_dma_pos);
#endif
    status = dmaengine_tx_status(ppd->dma_chan_tx, ppd->cookie_tx, &state);
}

int32_t DMADeinitTxBuff(struct PlatformData *platformData)
{
    struct PrivPlatformData *ppd;
    AUDIO_DRIVER_LOG_ERR("%s", __func__);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("privplatform data is null");
        return HDF_FAILURE;
    }

    ppd = (struct PrivPlatformData *)platformData->dmaPrv;
    if (ppd->dma_chan_tx) {
        dmaengine_terminate_sync(ppd->dma_chan_tx);
        dma_release_channel(ppd->dma_chan_tx);
        ppd->dma_chan_tx = NULL;
        dma_free_wc(&ppd->pdev->dev, platformData->renderBufInfo.cirBufSize,
                    platformData->renderBufInfo.virtAddr,
                    platformData->renderBufInfo.phyAddr);
        platformData->renderBufInfo.phyAddr = 0;
        platformData->renderBufInfo.virtAddr = NULL;
    } else {
        AUDIO_DRIVER_LOG_ERR("dma tx chan is null");
    }

    return HDF_SUCCESS;
}

int32_t DMAInitTxBuff(struct PlatformData *platformData)
{
    uint64_t buffSize;
    
    struct device *dev = NULL;
    struct PrivPlatformData *ppd = NULL;
    gfp_t gfp_flags;

    if (platformData == NULL) {
        return HDF_FAILURE;
    }

    ppd = (struct PrivPlatformData *)platformData->dmaPrv;
    dev = &ppd->pdev->dev;

    gfp_flags = GFP_KERNEL | __GFP_COMP | __GFP_NORETRY | __GFP_NOWARN;
    if (platformData->renderBufInfo.virtAddr != NULL) {
        return HDF_SUCCESS;
    }

    buffSize = platformData->renderBufInfo.cirBufMax;

    if (buffSize < MIN_AIAO_BUFF_SIZE || buffSize > MAX_AIAO_BUFF_SIZE) {
        AUDIO_DRIVER_LOG_ERR("buffsize error size:%d", buffSize);
        return HDF_FAILURE;
    }

    platformData->renderBufInfo.phyAddr = 0;
    platformData->renderBufInfo.virtAddr = dma_alloc_coherent(dev, buffSize,
        (dma_addr_t *)&platformData->renderBufInfo.phyAddr, gfp_flags);
    if (platformData->renderBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("dma alloc failed");
        return HDF_FAILURE;
    }

    platformData->renderBufInfo.cirBufSize = buffSize;
    return HDF_SUCCESS;
}

int32_t DMAConfigTxBuff(struct PlatformData *platformData)
{
    int32_t ret;
    struct device *dev = NULL;
    struct PrivPlatformData *ppd = NULL;
    struct dma_slave_config slave_config = {};

    if (platformData == NULL) {
        return HDF_FAILURE;
    }

    ppd = (struct PrivPlatformData *)platformData->dmaPrv;
    dev = &ppd->pdev->dev;

    ppd->dma_chan_tx = dma_request_slave_channel(dev, "tx");
    if (!ppd->dma_chan_tx) {
        AUDIO_DRIVER_LOG_ERR("slave channel failed");
        return HDF_FAILURE;
    }

    slave_config.direction = DMA_MEM_TO_DEV;
    slave_config.dst_addr = ppd->dma_addr_dst;
    if (platformData->renderPcmInfo.bitWidth == BIT_WIDTH16) {
        slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
    } else if (platformData->renderPcmInfo.bitWidth == BIT_WIDTH32) {
        slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    } else {
        AUDIO_DRIVER_LOG_ERR("slave config biwidth is error");
        return HDF_FAILURE;
    }

    slave_config.dst_maxburst = ppd->dma_maxburst_tx;

    ret = dmaengine_slave_config(ppd->dma_chan_tx, &slave_config);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("slave config failed");
        return HDF_FAILURE;
    }

    ppd->tx_dma_pos = 0;

    return HDF_SUCCESS;
}

int32_t DMAEnableTx(const struct PlatformData *platformData)
{
    struct PrivPlatformData *ppd = NULL;
    enum dma_status status;
    struct dma_tx_state state;
    struct dma_async_tx_descriptor *desc;

    AUDIO_DRIVER_LOG_ERR("%s", __func__);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL");
        return HDF_FAILURE;
    }
    ppd = (struct PrivPlatformData *)platformData->dmaPrv;

    desc = dmaengine_prep_dma_cyclic(ppd->dma_chan_tx,
                                     platformData->renderBufInfo.phyAddr,
                                     platformData->renderBufInfo.cirBufSize,
                                     platformData->renderBufInfo.periodSize,
                                     DMA_MEM_TO_DEV, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
    if (!desc) {
        AUDIO_DRIVER_LOG_ERR("failed to prepare slave DMA tx");
        return HDF_FAILURE;
    }
    desc->callback = DMATxComplete;
    desc->callback_param = (void*)platformData;

    ppd->cookie_tx = dmaengine_submit(desc);

    status = dmaengine_tx_status(ppd->dma_chan_tx, ppd->cookie_tx, &state);

    dma_async_issue_pending(ppd->dma_chan_tx);

    return HDF_SUCCESS;
}

int32_t DMADisableTx(struct PlatformData *platformData)
{
    struct PrivPlatformData *ppd = NULL;
    int32_t ret;
    AUDIO_DRIVER_LOG_ERR("%s", __func__);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL");
        return HDF_FAILURE;
    }
    ppd = (struct PrivPlatformData *)platformData->dmaPrv;

    if (ppd->dma_chan_tx) {
        dmaengine_terminate_all(ppd->dma_chan_tx);
    } else {
        AUDIO_DRIVER_LOG_ERR("dma chan not available");
    }

    ret = DMADeinitTxBuff(platformData);

    return ret;
}

int32_t DMAPauseTx(struct PlatformData *platformData)
{
    struct PrivPlatformData *ppd = NULL;
    AUDIO_DRIVER_LOG_ERR("%s", __func__);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL");
        return HDF_FAILURE;
    }
    ppd = (struct PrivPlatformData *)platformData->dmaPrv;

    if (ppd->dma_chan_tx) {
        dmaengine_terminate_all(ppd->dma_chan_tx);
    } else {
        AUDIO_DRIVER_LOG_ERR("dma chan not available");
    }

    return HDF_SUCCESS;
}

static void DMARxComplete(void *arg)
{
    struct PlatformData *platformData = (struct PlatformData *)arg;
    struct PrivPlatformData *ppd = (struct PrivPlatformData *)platformData->dmaPrv;
#ifdef RING_BUFFER_DEBUG
    unsigned int owptr = ppd->rx_dma_pos;
#endif
    ppd->rx_dma_pos += platformData->captureBufInfo.periodSize;
    if (ppd->rx_dma_pos >= platformData->captureBufInfo.cirBufSize) {
        ppd->rx_dma_pos -= platformData->captureBufInfo.cirBufSize;
    }

#ifdef RING_BUFFER_DEBUG
    AUDIO_DRIVER_LOG_INFO("dma rx complete. pos:%x", ppd->rx_dma_pos);

    unsigned int wptr = 0, rptr = 0;
    unsigned int free = 0, use = 0;
    wptr = ppd->rx_dma_pos;
    rptr = ppd->rx_read_pos;
    if (wptr >= rptr) {
        use  = wptr - rptr;
        free = platformData->captureBufInfo.cirBufSize - use;
    } else {
        free = rptr - wptr;
        use  = platformData->captureBufInfo.cirBufSize - free;
    }
    printk("[PRODUCTER]:virtAddr(0x%x) size=%u use=%u free=%u :rptr=%u owptr=%u cwptr=%u product=%d",
           platformData->captureBufInfo.virtAddr, platformData->captureBufInfo.cirBufSize, use, free,
           ppd->rx_read_pos, owptr, ppd->rx_dma_pos, platformData->captureBufInfo.periodSize);
#endif
}

int32_t DMADeinitRxBuff(struct PlatformData *platformData)
{
    struct PrivPlatformData *ppd;
    AUDIO_DRIVER_LOG_ERR("%s", __func__);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("privplatform data is null");
        return HDF_FAILURE;
    }

    ppd = (struct PrivPlatformData *)platformData->dmaPrv;
    if (ppd->dma_chan_rx) {
        dmaengine_terminate_sync(ppd->dma_chan_rx);
        dma_release_channel(ppd->dma_chan_rx);
        ppd->dma_chan_rx = NULL;
        dma_free_wc(&ppd->pdev->dev, platformData->captureBufInfo.cirBufSize,
                    platformData->captureBufInfo.virtAddr,
                    platformData->captureBufInfo.phyAddr);
        platformData->captureBufInfo.phyAddr = 0;
        platformData->captureBufInfo.virtAddr = NULL;
    } else {
        AUDIO_DRIVER_LOG_ERR("dma rx chan is null");
    }

    return HDF_SUCCESS;
}

int32_t DMAInitRxBuff(struct PlatformData *platformData)
{
    uint64_t buffSize;
    struct device *dev = NULL;
    struct PrivPlatformData *ppd = NULL;
    gfp_t gfp_flags;

    AUDIO_DRIVER_LOG_ERR("%s", __func__);
    if (platformData == NULL) {
        return HDF_FAILURE;
    }
    ppd = (struct PrivPlatformData *)platformData->dmaPrv;
    dev = &ppd->pdev->dev;

    gfp_flags = GFP_KERNEL | __GFP_COMP | __GFP_NORETRY | __GFP_NOWARN;
    if (platformData->captureBufInfo.virtAddr != NULL) {
        return HDF_SUCCESS;
    }

    buffSize = platformData->captureBufInfo.cirBufMax;
    if (buffSize < MIN_AIAO_BUFF_SIZE || buffSize > MAX_AIAO_BUFF_SIZE) {
        return HDF_FAILURE;
    }

    platformData->captureBufInfo.phyAddr = 0;
    platformData->captureBufInfo.virtAddr = dma_alloc_coherent(dev, buffSize,
        (dma_addr_t *)&platformData->captureBufInfo.phyAddr, gfp_flags);
    if (platformData->captureBufInfo.virtAddr == NULL) {
        return HDF_FAILURE;
    }
    platformData->captureBufInfo.cirBufSize = buffSize;

    return HDF_SUCCESS;
}

int32_t DMAConfigRxBuff(struct PlatformData *platformData)
{
    int32_t ret;
    struct device *dev = NULL;
    struct PrivPlatformData *ppd = NULL;
    struct dma_slave_config slave_config = {};

    ppd->dma_chan_rx = dma_request_slave_channel(dev, "rx");
    if (!ppd->dma_chan_rx) {
        return HDF_FAILURE;
    }

    slave_config.direction = DMA_DEV_TO_MEM;
    slave_config.src_addr = ppd->dma_addr_src;
    if (platformData->capturePcmInfo.bitWidth == BIT_WIDTH16) {
        slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
    } else if (platformData->capturePcmInfo.bitWidth == BIT_WIDTH32) {
        slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    } else {
        return HDF_FAILURE;
    }

    slave_config.src_maxburst = ppd->dma_maxburst_rx;
    ret = dmaengine_slave_config(ppd->dma_chan_rx, &slave_config);
    if (ret) {
        return HDF_FAILURE;
    }

    ppd->rx_dma_pos = 0;
    ppd->rx_read_pos = 0;

    return HDF_SUCCESS;
}

int32_t DMAEnableRx(const struct PlatformData *platformData)
{
    struct PrivPlatformData *ppd = NULL;
    struct dma_async_tx_descriptor *desc;

    AUDIO_DRIVER_LOG_ERR("%s", __func__);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL");
        return HDF_FAILURE;
    }
    ppd = (struct PrivPlatformData *)platformData->dmaPrv;

    desc = dmaengine_prep_dma_cyclic(ppd->dma_chan_rx,
                                     platformData->captureBufInfo.phyAddr,
                                     platformData->captureBufInfo.cirBufSize,
                                     platformData->captureBufInfo.periodSize,
                                     DMA_DEV_TO_MEM, DMA_CTRL_ACK | DMA_PREP_INTERRUPT);
    if (!desc) {
        AUDIO_DRIVER_LOG_ERR("failed to prepare slave DMA rx");
        return HDF_FAILURE;
    }
    desc->callback = DMARxComplete;
    desc->callback_param = (void*)platformData;
    ppd->cookie_rx = dmaengine_submit(desc);

    dma_async_issue_pending(ppd->dma_chan_rx);

    return HDF_SUCCESS;
}

int32_t DMADisableRx(struct PlatformData *platformData)
{
    struct PrivPlatformData *ppd = NULL;
    int32_t ret;
    AUDIO_DRIVER_LOG_ERR("%s", __func__);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL");
        return HDF_FAILURE;
    }
    ppd = (struct PrivPlatformData *)platformData->dmaPrv;

    if (ppd->dma_chan_rx) {
        dmaengine_terminate_all(ppd->dma_chan_rx);
    } else {
        AUDIO_DRIVER_LOG_ERR("dma chan not available");
    }

    ret = DMADeinitRxBuff(platformData);

    return ret;
}

int32_t DMAPauseRx(struct PlatformData *platformData)
{
    struct PrivPlatformData *ppd = NULL;
    AUDIO_DRIVER_LOG_ERR("%s", __func__);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL");
        return HDF_FAILURE;
    }
    ppd = (struct PrivPlatformData *)platformData->dmaPrv;

    if (ppd->dma_chan_rx) {
        dmaengine_terminate_all(ppd->dma_chan_rx);
    } else {
        AUDIO_DRIVER_LOG_ERR("dma chan not available");
    }

    return HDF_SUCCESS;
}

int32_t DmaInit(struct PlatformData *platformData)
{
    struct PrivPlatformData *ppd = NULL;
    int32_t ret = 0;

    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL");
        return HDF_FAILURE;
    }

    ppd = (struct PrivPlatformData *)platformData->dmaPrv;
    ret = dma_coerce_mask_and_coherent(&ppd->pdev->dev, DMA_BIT_MASK(32));

    return ret;
}
