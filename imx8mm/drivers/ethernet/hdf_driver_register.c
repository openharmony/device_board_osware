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

#include "devsvc_manager_clnt.h"
#include "eth_chip_driver.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "nxpeth_mac.h"
#include "nxpeth_phy.h"
#include "osal_mem.h"

static const char* NXP_ETHERNET_DRIVER_NAME = "nxpeth-fec";

static int32_t HdfEthRegNxpDriverFactory(void)
{
    static struct HdfEthChipDriverFactory tmpFactory = { 0 };
    struct HdfEthChipDriverManager *driverMgr = HdfEthGetChipDriverMgr();

    if (driverMgr == NULL && driverMgr->RegChipDriver == NULL) {
        HDF_LOGE("%s fail: driverMgr is NULL", __func__);
        return HDF_FAILURE;
    }
    tmpFactory.driverName = NXP_ETHERNET_DRIVER_NAME;
    tmpFactory.InitEthDriver = InitNxpethDriver;
    tmpFactory.GetMacAddr = EthNxpRandomAddr;
    tmpFactory.DeinitEthDriver = DeinitNxpethDriver;
    tmpFactory.BuildMacDriver = BuildNxpMacDriver;
    tmpFactory.ReleaseMacDriver = ReleaseNxpMacDriver;
    if (driverMgr->RegChipDriver(&tmpFactory) != HDF_SUCCESS) {
        HDF_LOGE("%s fail: driverMgr is NULL", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGI("nxp eth driver register success");
    return HDF_SUCCESS;
}

static int32_t HdfEthNxpChipDriverInit(struct HdfDeviceObject *device)
{
    (void)device;
    return HdfEthRegNxpDriverFactory();
}

static int32_t HdfEthNxpDriverBind(struct HdfDeviceObject *dev)
{
    (void)dev;
    return HDF_SUCCESS;
}

static void HdfEthNxpChipRelease(struct HdfDeviceObject *object)
{
    (void)object;
}

struct HdfDriverEntry g_hdfnxpethchipentry = {
    .moduleVersion = 1,
    .Bind = HdfEthNxpDriverBind,
    .Init = HdfEthNxpChipDriverInit,
    .Release = HdfEthNxpChipRelease,
    .moduleName = "HDF_ETHERNET_CHIPS"
};

HDF_INIT(g_hdfnxpethchipentry);
