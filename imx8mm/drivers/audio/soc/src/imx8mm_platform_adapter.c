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

#include "audio_core.h"
#include "audio_driver_log.h"
#include "imx8mm_platform.h"
#include "imx8mm_common.h"
#include "osal_mem.h"
#include "imx8mm_platform_ops.h"

#define HDF_LOG_TAG imx8mm_platform_adapter

#define PLATFORM_FIND_SUCCESS 2
#define PLATFORM_FIND_NEXT    0

static int32_t AudioPlatformDeviceInit(const struct AudioCard *card, const struct PlatformDevice *platform);
static int32_t PlatformDmaPrvDataInit(struct PlatformData *data, struct HdfDeviceObject *device);

struct AudioDmaOps g_platform_device_ops = {
    .DmaBufAlloc = Imx8mmDmaBufAlloc,
    .DmaBufFree = Imx8mmDmaBufFree,
    .DmaRequestChannel = Imx8mmDmaRequestChannel,
    .DmaConfigChannel = Imx8mmDmaConfigChannel,
    .DmaPrep = Imx8mmDmaPrep,
    .DmaSubmit = Imx8mmDmaSubmit,
    .DmaPending = Imx8mmDmaPending,
    .DmaPause = Imx8mmDmaPause,
    .DmaResume = Imx8mmDmaResume,
    .DmaPointer = Imx8mmDmaPointer,
};

struct PlatformData g_platform_data = {
    .PlatformInit = AudioPlatformDeviceInit,
    .ops = &g_platform_device_ops,
    .platformInitFlag = false,
};

/* HdfDriverEntry implementations */
static int32_t PlatformDriverBind(struct HdfDeviceObject *device)
{
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry!");

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    platformHost = (struct PlatformHost *)OsalMemCalloc(sizeof(*platformHost));
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("malloc host fail!");
        return HDF_FAILURE;
    }

    platformHost->device = device;
    device->service = &platformHost->service;

    AUDIO_DRIVER_LOG_DEBUG("success!");
    return HDF_SUCCESS;
}

static int32_t PlatformGetServiceName(const struct HdfDeviceObject *device)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("para is NULL.");
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("node is NULL.");
        return HDF_FAILURE;
    }

    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("get drsops object instance fail!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetString(node, "serviceName", &g_platform_data.drvPlatformName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read serviceName fail!");
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t PlatformDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;

    AUDIO_DRIVER_LOG_DEBUG("entry.\n");
    struct PlatformHost *platformHost = NULL;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = PlatformGetServiceName(device);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get service name fail.");
        return ret;
    }

    ret = AudioSocRegisterPlatform(device, (struct PlatformData *)(&g_platform_data));
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("register dai fail.");
        return ret;
    }

    platformHost = (struct PlatformHost *)device->service;
    if (platformHost != NULL) {
        OsalMutexInit(&g_platform_data.renderBufInfo.buffMutex);
        OsalMutexInit(&g_platform_data.captureBufInfo.buffMutex);
    }

    ret = PlatformDmaPrvDataInit(&g_platform_data, device);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("platform soc init fail");
        return ret;
    }

    AUDIO_DRIVER_LOG_INFO("success.\n");
    return HDF_SUCCESS;
}

static void PlatformDriverRelease(struct HdfDeviceObject *device)
{
    struct PlatformHost *platformHost = NULL;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL");
        return;
    }

    platformHost = (struct PlatformHost *)device->service;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformHost is NULL");
        return;
    }

    OsalMutexDestroy(&g_platform_data.renderBufInfo.buffMutex);
    OsalMutexDestroy(&g_platform_data.captureBufInfo.buffMutex);
    OsalMemFree(platformHost);
}

static int32_t AudioPlatformDeviceInit(const struct AudioCard *card, const struct PlatformDevice *platform)
{
    AUDIO_DRIVER_LOG_ERR("ENTRY");
    if (card == NULL || platform == NULL || platform->devData == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform is NULL.");
        return HDF_FAILURE;
    }
    if (platform->devData->platformInitFlag == true) {
        AUDIO_DRIVER_LOG_DEBUG("platform init complete!");
        return HDF_SUCCESS;
    }
    platform->devData->platformInitFlag = true;

    SaiDaiProbe(platform->devData);

    return HDF_SUCCESS;
}

static int32_t PlatformGetInfoFromHcs(struct PrivPlatformData *ppd,
                                      const struct DeviceResourceNode *node)
{
    struct DeviceResourceIface *iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (ppd == NULL || node == NULL || iface == NULL || iface->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("face is invalid");
        return HDF_FAILURE;
    }

    if (iface->GetString(node, "sai_name", &ppd->sai_name, NULL) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read sai_name fail");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_ERR("sai name = %s", ppd->sai_name);

    return HDF_SUCCESS;
}

static int32_t PlatformFindDeviceFromBus(struct device *dev, void *para)
{
    struct platform_device *pdev = NULL;
    struct PrivPlatformData *ppd = (struct PrivPlatformData*)para;

    if (dev == NULL || para == NULL) {
        AUDIO_DRIVER_LOG_ERR("invalid param");
        return HDF_ERR_INVALID_PARAM;
    }

    pdev = to_platform_device(dev);
    if (pdev->name == NULL) {
        AUDIO_DRIVER_LOG_ERR("pdev name NULL");
        return HDF_ERR_INVALID_PARAM;
    }

    if (!strstr(pdev->name, ppd->sai_name)) {
        return PLATFORM_FIND_NEXT;
    }
    ppd->pdev = pdev;

    return PLATFORM_FIND_SUCCESS;
}

static int32_t PlatformDmaPrvDataInit(struct PlatformData *data, struct HdfDeviceObject *device)
{
    int32_t ret;
    struct PrivPlatformData *ppd = NULL;

    if (device->property == NULL|| data == NULL) {
        AUDIO_DRIVER_LOG_ERR("property is null");
        return HDF_FAILURE;
    }

    ppd = (struct PrivPlatformData *)OsalMemCalloc(sizeof(struct PrivPlatformData));
    if (ppd == NULL) {
        AUDIO_DRIVER_LOG_ERR("OsalMemCalloc malloc error");
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = PlatformGetInfoFromHcs(ppd, device->property);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformGetInfoFromHcs error");
        OsalMemFree(ppd);
        return HDF_FAILURE;
    }

    data->dmaPrv = ppd;

    ret = bus_for_each_dev(&platform_bus_type, NULL, (void*)ppd, PlatformFindDeviceFromBus);
    if (ret != PLATFORM_FIND_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("platform find fail, ret is %d", ret);
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_ERR("sai_name = %s", ppd->pdev->name);

    ret = SaiDriverInit(data);
    AUDIO_DRIVER_LOG_ERR("RET = %d", ret);
    ret = DmaInit(data);
    AUDIO_DRIVER_LOG_ERR("RET = %d", ret);

    return HDF_SUCCESS;
}

/* HdfDriverEntry definitions */
struct HdfDriverEntry g_platform_driver_entry = {
    .moduleVersion = 1,
    .moduleName = "SAI_IMX8",
    .Bind = PlatformDriverBind,
    .Init = PlatformDriverInit,
    .Release = PlatformDriverRelease,
};
HDF_INIT(g_platform_driver_entry);
