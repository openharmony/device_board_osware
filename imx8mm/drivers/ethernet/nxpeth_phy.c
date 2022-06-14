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

#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/version.h>
#include <linux/delay.h>

#include "net_adapter.h"
#include "eth_chip_driver.h"
#include "net_device.h"
#include "net_device_adapter.h"
#include "osal_mem.h"
#include "nxpeth_phy.h"

struct NetDevice *g_hdf_netdev;
extern int fec_driver_init(void);
extern int32_t GetSizeOfPrivData(void);
int32_t InitNxpethDriver(struct EthDevice *ethDevice)
{
    int32_t ret;
    int32_t size = GetSizeOfPrivData();

    if (ethDevice == NULL) {
        HDF_LOGE("%s input is NULL!", __func__);
        return HDF_FAILURE;
    }

    g_hdf_netdev = ethDevice->netdev;
    g_hdf_netdev->mlPriv = (struct fec_enet_private *)OsalMemCalloc(size);
    if (g_hdf_netdev->mlPriv == NULL) {
        HDF_LOGE("%s kzalloc mlPriv failed!", __func__);
        return HDF_FAILURE;
    } else {
        HDF_LOGE("%s kzalloc mlPriv ok!", __func__);
    }

    fec_driver_init();
    ret = EthernetInitNetdev(ethDevice->netdev);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s failed to init ethernet netDev", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t DeinitNxpethDriver(struct EthDevice *ethDevice)
{
    return HDF_SUCCESS;
}

int32_t GetSizeOfPrivData(void)
{
    return sizeof(struct fec_enet_private);
}

struct fec_enet_private *
fec_get_priv_data(struct NetDevice *netDev)
{
    return (struct fec_enet_private*)g_hdf_netdev->mlPriv;
}

struct NetDevice *fec_get_hdfnet(void)
{
    return g_hdf_netdev;
}
EXPORT_SYMBOL(GetSizeOfPrivData);
EXPORT_SYMBOL(fec_get_priv_data);
EXPORT_SYMBOL(fec_get_hdfnet);