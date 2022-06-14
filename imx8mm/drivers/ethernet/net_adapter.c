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
#include "eth_chip_driver.h"
#include "net_device.h"
#include "netbuf_adapter.h"
#include "net_device_adapter.h"
#include "nxpeth_phy.h"
#include "net_adapter.h"

struct net_device *g_save_eth_net = NULL;
extern const struct Net_device_ops fec_netDev_ops;
extern struct net_device *get_eth_netdev(void);
extern int nxp_fec_set_mac(unsigned char *mac_address, struct NetDevice *netDev);
static int32_t EthSetMacAddr(struct NetDevice *netDev, void *addr)
{
    nxp_fec_set_mac((unsigned char *)addr, netDev);
    HDF_LOGE("%s SetMac successful !", __func__);
    return HDF_SUCCESS;
}

static int32_t EthXmit(struct NetDevice *netDev, NetBuf *netbuf)
{
    int32_t ret = 0;

    fec_netDev_ops.ndo_start_xmit(netbuf, netDev);

    return ret;
}

static int32_t EthOpen(struct NetDevice *netDev)
{
    int32_t ret = 0;

    fec_netDev_ops.ndo_open(netDev);
    HDF_LOGE("%s open successful !", __func__);
    return ret;
}

static int32_t EthStop(struct NetDevice *netDev)
{
    int32_t ret = 0;

    fec_netDev_ops.ndo_stop(netDev);
    HDF_LOGE("%s stop successful !", __func__);

    return ret;
}

struct NetDeviceInterFace g_ethnetdevops = {
    .xmit = EthXmit,
    .setMacAddr = EthSetMacAddr,
    .open = EthOpen,
    .stop = EthStop,
};

int32_t EthernetInitNetdev(NetDevice *netdev)
{
    int32_t ret = HDF_FAILURE;

    if (netdev == NULL) {
        HDF_LOGE("%s netdev is null!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    netdev->netDeviceIf = &g_ethnetdevops;

    ret = NetDeviceAdd(netdev);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s NetDeviceAdd return error code %d!", __func__, ret);
        return ret;
    }

    // open netdevice
    HDF_LOGE("%s netdev->netDeviceIf->open!", __func__);
    netdev->netDeviceIf->open(netdev);

    return ret;
}

void set_eth_netdev(struct net_device *dev)
{
    g_save_eth_net = dev;
}

struct net_device *get_eth_netdev(void)
{
    return g_save_eth_net;
}

EXPORT_SYMBOL(set_eth_netdev);
EXPORT_SYMBOL(get_eth_netdev);
