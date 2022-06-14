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
#include "hdf_device_desc.h"
#include "hdf_wifi_product.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "hdf_wlan_chipdriver_manager.h"
#include "securec.h"
#include "wifi_module.h"

#define HDF_LOG_TAG Ap6212Driver
static const char * const AP6212_DRIVER_NAME = "ap6212";

int32_t InitAp6212Chip(struct HdfWlanDevice *device);
int32_t DeinitAp6212Chip(struct HdfWlanDevice *device);
int32_t Ap6212Deinit(struct HdfChipDriver *chipDriver, struct NetDevice *netDevice);
int32_t Ap6212Init(struct HdfChipDriver *chipDriver, struct NetDevice *netDevice);
void ApMac80211Init(struct HdfChipDriver *chipDriver);

static struct HdfChipDriver *BuildAp6212Driver(struct HdfWlanDevice *device, uint8_t ifIndex)
{
    struct HdfChipDriver *specificDriver = NULL;

    HDF_LOGE("%s: Enter ", __func__);

    if (device == NULL) {
        HDF_LOGE("%s fail : channel is NULL", __func__);
        return NULL;
    }
    (void)device;
    (void)ifIndex;
    specificDriver = (struct HdfChipDriver *)OsalMemCalloc(sizeof(struct HdfChipDriver));
    if (specificDriver == NULL) {
        HDF_LOGE("%s fail: OsalMemCalloc fail!", __func__);
        return NULL;
    }
    if (memset_s(specificDriver, sizeof(struct HdfChipDriver), 0, sizeof(struct HdfChipDriver)) != EOK) {
        HDF_LOGE("%s fail: memset_s fail!", __func__);
        OsalMemFree(specificDriver);
        return NULL;
    }

    if (strcpy_s(specificDriver->name, MAX_WIFI_COMPONENT_NAME_LEN, AP6212_DRIVER_NAME) != EOK) {
        HDF_LOGE("%s fail : strcpy_s fail", __func__);
        OsalMemFree(specificDriver);
        return NULL;
    }
    specificDriver->init = Ap6212Init;
    specificDriver->deinit = Ap6212Deinit;

    ApMac80211Init(specificDriver);

    return specificDriver;
}

static void ReleaseAp6212Driver(struct HdfChipDriver *chipDriver)
{
    HDF_LOGE("%s: Enter ", __func__);
    return;

    if (chipDriver == NULL) {
        return;
    }
    if (strcmp(chipDriver->name, AP6212_DRIVER_NAME) != 0) {
        HDF_LOGE("%s:Not my driver!", __func__);
        return;
    }
    OsalMemFree(chipDriver);
}

static uint8_t GetAp6212GetMaxIFCount(struct HdfChipDriverFactory *factory)
{
    (void)factory;
    HDF_LOGE("%s: Enter ", __func__);

    return 1;
}

/* ap6212's register */
static int32_t HDFWlanRegApDriverFactory(void)
{
    static struct HdfChipDriverFactory tmpFactory = { 0 };
    struct HdfChipDriverManager *driverMgr = NULL;

    HDF_LOGE("%s: Enter ", __func__);
    driverMgr = HdfWlanGetChipDriverMgr();
    if (driverMgr == NULL) {
        HDF_LOGE("%s fail: driverMgr is NULL!", __func__);
        return HDF_FAILURE;
    }
    tmpFactory.driverName = AP6212_DRIVER_NAME;
    tmpFactory.GetMaxIFCount = GetAp6212GetMaxIFCount;
    tmpFactory.InitChip = InitAp6212Chip;
    tmpFactory.DeinitChip = DeinitAp6212Chip;
    tmpFactory.Build = BuildAp6212Driver;
    tmpFactory.Release = ReleaseAp6212Driver;
    tmpFactory.ReleaseFactory = NULL;
    if (driverMgr->RegChipDriver(&tmpFactory) != HDF_SUCCESS) {
        HDF_LOGE("%s fail: driverMgr is NULL!", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t HdfWlanApChipDriverInit(struct HdfDeviceObject *device)
{
    (void)device;
    HDF_LOGE("%s: Enter ", __func__);
    return HDFWlanRegApDriverFactory();
}

static int HdfWlanApDriverBind(struct HdfDeviceObject *dev)
{
    (void)dev;
    HDF_LOGE("%s: Enter ", __func__);
    return HDF_SUCCESS;
}

static void HdfWlanApChipRelease(struct HdfDeviceObject *object)
{
    (void)object;
    HDF_LOGE("%s: Enter ", __func__);
}

struct HdfDriverEntry g_hdfapchipentry = {
    .moduleVersion = 1,
    .Bind = HdfWlanApDriverBind,
    .Init = HdfWlanApChipDriverInit,
    .Release = HdfWlanApChipRelease,
    .moduleName = "HDF_WLAN_CHIPS"
};

HDF_INIT(g_hdfapchipentry);
