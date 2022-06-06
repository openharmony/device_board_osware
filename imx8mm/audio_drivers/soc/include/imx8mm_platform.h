/*
 *
 *
 */

#ifndef IMX8MM_PLATFORM_H
#define IMX8MM_PLATFORM_H

#include <linux/platform_device.h>
#include <linux/dmaengine.h>
#include "audio_core.h"
#include "sai_driver.h"
#include "dma_driver.h"

#define AIAO_BUFF_OFFSET               (128)
#define AIAO_BUFF_POINT                (320)
#define AIAO_BUFF_TRANS                (16 * 1024)
#define AIAO_BUFF_SIZE                 (128 * 1024)
#define RENDER_TRAF_BUF_SIZE           (1024)
#define CAPTURE_TRAF_BUF_SIZE          (1024 * 16)

#define AUDIO_BUFF_MIN                 (128)
#define AUDIO_RECORD_MIN               (1024 * 16)

#define MAX_PERIOD_SIZE                (1024 * 16)
#define MIN_PERIOD_COUNT               (8)
#define MAX_PERIOD_COUNT               (32)
#define MAX_AIAO_BUFF_SIZE             (128 * 1024)
#define MIN_AIAO_BUFF_SIZE             (16 * 1024)
#define MMAP_MAX_FRAME_SIZE            (4096 * 2 * 3)
#define AUDIO_CACHE_ALIGN_SIZE         (64)
#define DELAY_TIME                     (5)
#define LOOP_COUNT                     (100)

#define TRIGGER_TX                     (0)
#define TRIGGER_RX                     (1)
#define TRIGGER_START                  (0)
#define TRIGGER_STOP                   (1)

struct PrivPlatformData {
    struct platform_device *pdev;
    const char *sai_name;
    struct fsl_sai sai;
    dma_cookie_t cookie_tx;
    dma_cookie_t cookie_rx;
    struct dma_chan *dma_chan_rx;
    struct dma_chan *dma_chan_tx;
    dma_addr_t dma_addr_src;
    dma_addr_t dma_addr_dst;
    u32 dma_maxburst_rx;
    u32 dma_maxburst_tx;
    struct dma_async_tx_descriptor *rx_desc;
    struct dma_async_tx_descriptor *tx_desc;
    unsigned int rx_dma_pos;
    unsigned int tx_dma_pos;
    unsigned int rx_read_pos;
    unsigned int tx_write_pos;
};

#endif
