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

#include <sound/soc.h>
#include <sound/jack.h>
#include <sound/control.h>
#include <sound/pcm_params.h>
#include <sound/soc-dapm.h>
#include <linux/clk.h>
#include <linux/clk/clk-conf.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#include <linux/mfd/syscon.h>

#include "imx8mm_common.h"
#include "audio_core.h"
#include "audio_driver_log.h"
#include "osal_io.h"
#include "audio_dai_base.h"
#include "audio_platform_base.h"
#include "imx8mm_platform_ops.h"
#include "dma_driver.h"

#define HDF_LOG_TAG imx8mm_dai_adapter

#define DAI_FIND_NEXT 0
#define DAI_FIND_SUCCESS 2

#define    AUDIO_DRV_PCM_IOCTL_RENDER_START   5
#define    AUDIO_DRV_PCM_IOCTL_RENDER_STOP    6
#define    AUDIO_DRV_PCM_IOCTL_CAPTURE_START  7
#define    AUDIO_DRV_PCM_IOCTL_CAPTURE_STOP   8
#define    AUDIO_DRV_PCM_IOCTL_RENDER_PAUSE   9
#define    AUDIO_DRV_PCM_IOCTL_CAPTURE_PAUSE  10
#define    AUDIO_DRV_PCM_IOCTL_RENDER_RESUME  11
#define    AUDIO_DRV_PCM_IOCTL_CAPTURE_RESUME 12

#define TRIGGER_TX                     (0)
#define TRIGGER_RX                     (1)
#define TRIGGER_START                  (0)
#define TRIGGER_STOP                   (1)

struct PrivDaiData {
    struct platform_device *pdev;
    const char *dai_dev_name;
    struct clk *mclk;
    struct regmap *gpr;
};

extern int32_t DaiHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param);
extern int32_t DaiTrigger(const struct AudioCard *card, int cmd, const struct DaiDevice *device);
extern int32_t DaiDeviceInit(struct AudioCard *audioCard, const struct DaiDevice *dai);
extern int32_t SaiTrigger(const struct DaiData *pd, int cmd, int isTx);
extern int32_t SaiRuntimeSuspend(const struct DaiData *daiData);
extern int32_t SaiHwFree(const struct DaiData *pd, int isTx);

struct AudioDaiOps g_dai_device_ops = {
    .HwParams = DaiHwParams,
    .Trigger = DaiTrigger,
};

struct DaiData g_dai_data = {
    .DaiInit = DaiDeviceInit,
    .ops = &g_dai_device_ops,
};


static int32_t DaiDriverBind(struct HdfDeviceObject *device)
{
    struct DaiHost *daiHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry!");

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    daiHost = (struct DaiHost *)OsalMemCalloc(sizeof(*daiHost));
    if (daiHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("malloc host fail!");
        return HDF_FAILURE;
    }

    daiHost->device = device;
    g_dai_data.daiInitFlag = false;
    device->service = &daiHost->service;

    AUDIO_DRIVER_LOG_ERR("success!");
    return HDF_SUCCESS;
}

int32_t DaiDeviceInit(struct AudioCard *audioCard, const struct DaiDevice *dai)
{
    struct DaiHost *daiHost = NULL;

    if (dai == NULL || dai->device == NULL || dai->devDaiName == NULL) {
        AUDIO_DRIVER_LOG_ERR("dai is nullptr.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_ERR("cpu dai device name: %s\n", dai->devDaiName);
    (void)audioCard;

    daiHost = (struct DaiHost *)dai->device->service;
    if (daiHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("dai host is NULL.");
        return HDF_FAILURE;
    }

    if (g_dai_data.daiInitFlag == true) {
        AUDIO_DRIVER_LOG_DEBUG("dai init complete!");
        return HDF_SUCCESS;
    }

    g_dai_data.daiInitFlag = true;

    return HDF_SUCCESS;
}

static int32_t CheckSampleRate(unsigned int sampleRates)
{
    bool checkSampleRate = (sampleRates == AUDIO_SAMPLE_RATE_8000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_12000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_11025 ||
                           sampleRates == AUDIO_SAMPLE_RATE_16000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_22050 ||
                           sampleRates == AUDIO_SAMPLE_RATE_24000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_32000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_44100 ||
                           sampleRates == AUDIO_SAMPLE_RATE_48000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_64000 ||
                           sampleRates == AUDIO_SAMPLE_RATE_96000);
    if (checkSampleRate) {
        return HDF_SUCCESS;
    } else {
        AUDIO_DRIVER_LOG_ERR("FramatToSampleRate fail: Invalid sampling rate: %d.", sampleRates);
        return HDF_ERR_NOT_SUPPORT;
    }
}

int32_t DaiHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param)
{
    struct DaiHost *daiHost = NULL;
    struct PrivDaiData *pdd = NULL;
    int ret = 0;

    AUDIO_DRIVER_LOG_DEBUG("entry.");

    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL ||
        param == NULL || param->cardServiceName == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is nullptr.");
        return HDF_FAILURE;
    }

    ret = CheckSampleRate(param->rate);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("CheckSampleRate:  fail.");
        return HDF_ERR_NOT_SUPPORT;
    }

    daiHost = (struct DaiHost *)card->rtd->cpuDai->device->service;
    if (daiHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformHost is nullptr.");
        return HDF_FAILURE;
    }

    pdd = (struct PrivDaiData *)daiHost->priv;
    if (pdd == NULL) {
        AUDIO_DRIVER_LOG_ERR("daiHost is null");
        return HDF_FAILURE;
    }

    if (pdd->mclk) {
        AUDIO_DRIVER_LOG_ERR("enable mclk");
    } else {
        AUDIO_DRIVER_LOG_ERR("daihost mclk nullptr");
        return HDF_FAILURE;
    }

    if (ret) {
        AUDIO_DRIVER_LOG_ERR("mclk enable failed");
        return HDF_FAILURE;
    }

    struct DaiData *data = DaiDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformHost is nullptr.");
        return HDF_FAILURE;
    }

    data->pcmInfo.channels = param->channels;
    data->pcmInfo.rate = param->rate;
    data->pcmInfo.streamType = param->streamType;

    return HDF_SUCCESS;
}
extern int32_t g_dmaRequestChannel;
int32_t DaiTrigger(const struct AudioCard *card, int cmd, const struct DaiDevice *device)
{
    struct DaiData *data = device->devData;
    bool tx = data->pcmInfo.streamType == AUDIO_CAPTURE_STREAM;
    int32_t ret = 0;
    struct PlatformData *pData = PlatformDataFromCard(card);

    switch (cmd) {
        case AUDIO_DRV_PCM_IOCTL_RENDER_START:
        case AUDIO_DRV_PCM_IOCTL_CAPTURE_START:
        case AUDIO_DRV_PCM_IOCTL_RENDER_RESUME:
        case AUDIO_DRV_PCM_IOCTL_CAPTURE_RESUME:
            if (g_dmaRequestChannel == 0) {
                Imx8mmDmaRequestChannel(pData, data->pcmInfo.streamType);

                if (pData->renderBufInfo.virtAddr == NULL) {
                    ret = DMAInitTxBuff(pData);
                    if (ret != HDF_SUCCESS) {
                    AUDIO_DRIVER_LOG_ERR("DMAAoInit: fail");
                    return HDF_FAILURE;
                    }
                }

                ret = DMAEnableTx(pData);
                if (ret != HDF_SUCCESS) {
                    AUDIO_DRIVER_LOG_ERR("DMAEnableTx failed");
                    return HDF_FAILURE;
                }
            }

            g_dmaRequestChannel = 0;
            ret = SaiTrigger(data, TRIGGER_START, tx);
            if (ret != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("SaiTrigger failed");
                return HDF_FAILURE;
            }
            break;
        case AUDIO_DRV_PCM_IOCTL_RENDER_STOP:
        case AUDIO_DRV_PCM_IOCTL_CAPTURE_STOP:
        case AUDIO_DRV_PCM_IOCTL_RENDER_PAUSE:
        case AUDIO_DRV_PCM_IOCTL_CAPTURE_PAUSE:
            ret = SaiTrigger(data, TRIGGER_STOP, tx);
            if (ret != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("SaiTrigger failed");
                return HDF_FAILURE;
            }

            ret = SaiRuntimeSuspend(data);
            if (ret != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("runtime suspend failed");
                return HDF_FAILURE;
            }
            break;
        default:
            AUDIO_DRIVER_LOG_ERR("cmd is error");
            break;
    }
    return HDF_SUCCESS;
}

static int32_t DaiGetInfoFromHcs(struct PrivDaiData *p_data, const struct DeviceResourceNode *node)
{
    struct DeviceResourceIface *iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (p_data == NULL || node == NULL || iface == NULL || iface->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("%s: face is invalid", __func__);
        return HDF_FAILURE;
    }

    if (iface->GetString(node, "dai_dev_name", &p_data->dai_dev_name, NULL) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("%s: read dai name fail", __func__);
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_ERR("%s: dai_name = %s", __func__, p_data->dai_dev_name);

    return HDF_SUCCESS;
}

static int32_t DaiFindDeviceFromBus(struct device *dev, void *para)
{
    struct platform_device *pdev = NULL;
    struct PrivDaiData *pdd = (struct PrivDaiData*)para;

    if (dev == NULL || para == NULL) {
        AUDIO_DRIVER_LOG_ERR("invalid param.\n");
        return HDF_ERR_INVALID_PARAM;
    }

    pdev = to_platform_device(dev);
    if (pdev->name == NULL) {
        AUDIO_DRIVER_LOG_ERR("pdev name NULL\n");
        return HDF_ERR_INVALID_PARAM;
    }

    if (!strstr(pdev->name, pdd->dai_dev_name)) {
        return DAI_FIND_NEXT;
    }

    pdd->pdev = pdev;

    return DAI_FIND_SUCCESS;
}

static int32_t DaiInit(struct DaiHost * daiHost, struct HdfDeviceObject *device)
{
    int32_t ret;
    struct PrivDaiData *pdd;
    struct device_node *codec_np = NULL;
    struct device_node *gpr_np;
    struct i2c_client *codec_dev;

    AUDIO_DRIVER_LOG_ERR("entry\n");

    if (device->property == NULL || daiHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("invalid param");
        return HDF_FAILURE;
    }

    pdd = (struct PrivDaiData *)OsalMemCalloc(sizeof(struct PrivDaiData));
    if (pdd == NULL) {
        AUDIO_DRIVER_LOG_ERR("OsalMemCalloc configInfo error");
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = DaiGetInfoFromHcs(pdd, device->property);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("error");
        return HDF_FAILURE;
    }

    daiHost->priv = pdd;

    ret = bus_for_each_dev(&platform_bus_type, NULL, (void*)pdd, DaiFindDeviceFromBus);
    if (ret != DAI_FIND_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("dai find fail, ret is %d", ret);
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_ERR("dai name = %s", pdd->dai_dev_name);

    ret = of_clk_set_defaults(pdd->pdev->dev.of_node, false);
    if (ret < 0) {
        AUDIO_DRIVER_LOG_ERR("set clk default failed");
        return HDF_FAILURE;
    }

    codec_np = of_parse_phandle(pdd->pdev->dev.of_node, "audio-codec", 0);
    if (!codec_np) {
        AUDIO_DRIVER_LOG_ERR("phandle missing or invalid\n");
        return HDF_FAILURE;
    }

    codec_dev = of_find_i2c_device_by_node(codec_np);
    if (!codec_dev) {
        AUDIO_DRIVER_LOG_ERR("codec_dev invalid\n");
        if (codec_np) {
            of_node_put(codec_np);
        }

        return HDF_FAILURE;
    }

    pdd->mclk = devm_clk_get(&codec_dev->dev, "mclk");

    if (IS_ERR(pdd->mclk)) {
        AUDIO_DRIVER_LOG_ERR("mclk get failed\n");
        if (codec_np) {
            of_node_put(codec_np);
        }

        return HDF_FAILURE;
    }

    gpr_np = of_parse_phandle(pdd->pdev->dev.of_node, "gpr", 0);
    if (gpr_np) {
        pdd->gpr = syscon_node_to_regmap(gpr_np);
        if (IS_ERR(pdd->gpr)) {
            ret = PTR_ERR(pdd->gpr);
            AUDIO_DRIVER_LOG_ERR("failed to get gpr regmap\n");
            if (codec_np) {
                of_node_put(codec_np);
            }

            return HDF_FAILURE;
        }

        /* set SAI2_MCLK_DIR to enable codec MCLK for imx7d */
        regmap_update_bits(pdd->gpr, 4, 1 << 20, 1 << 20);
    }

    return HDF_SUCCESS;
}

static int32_t DaiGetServiceName(const struct HdfDeviceObject *device)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is nullptr.");
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("drs node is nullptr.");
        return HDF_FAILURE;
    }
    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("invalid drs ops fail!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetString(node, "serviceName", &g_dai_data.drvDaiName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read serviceName fail!");
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t DaiDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct DaiHost *daiHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry.\n");

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is nullptr.");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = DaiGetServiceName(device);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get service name fail.");
        return ret;
    }

    ret = AudioSocRegisterDai(device, (void *)&g_dai_data);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("register dai fail.");
        return ret;
    }

    daiHost = (struct DaiHost *)device->service;

    ret = DaiInit(daiHost, device);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("Dai init fail.");
        return ret;
    }

    AUDIO_DRIVER_LOG_DEBUG("success.\n");
    return HDF_SUCCESS;
}

struct HdfDriverEntry g_dai_driver_entry = {
    .moduleVersion = 1,
    .moduleName = "DAI_IMX8",
    .Bind = DaiDriverBind,
    .Init = DaiDriverInit,
    .Release = NULL,
};
HDF_INIT(g_dai_driver_entry);
