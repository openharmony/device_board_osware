/*
 * Copyright© 2021–2022 Beijing OSWare Technology Co., Ltd
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
#include "hdf_wifi_product.h"
#include "wifi_mac80211_ops.h"
#include "hdf_wlan_utils.h"

/***********************************************************/
/*      marco declare                                      */
/***********************************************************/
#define AP_SUCCESS (0)
#define HDF_LOG_TAG Ap6212Driver

/***********************************************************/
/*      variable and function declare                      */
/***********************************************************/
struct NetDevice *g_hdf_netdevice;
struct net_device *g_save_kernel_net = NULL;

/***********************************************************/
/*      variable and function declare                      */
/***********************************************************/
extern struct net_device_ops dhd_ops_pri;
extern int get_dhd_priv_data_size(void);
extern int32_t hdf_netdev_init(struct NetDevice *netDev);
extern int32_t hdf_netdev_open(struct NetDevice *netDev);
extern int32_t hdf_netdev_stop(struct NetDevice *netDev);

int32_t InitAp6212Chip(struct HdfWlanDevice *device)
{
    HDF_LOGE("%s: start...", __func__);
    return HDF_SUCCESS;
}

int32_t DeinitAp6212Chip(struct HdfWlanDevice *device)
{
    int32_t ret = 0;

    (void)device;
    HDF_LOGE("%s: start...", __func__);
    if (ret != 0) {
        HDF_LOGE("%s:Deinit failed!ret=%d", __func__, ret);
    }
    return ret;
}

int32_t Ap6212Init(struct HdfChipDriver *chipDriver, struct NetDevice *netDevice)
{
    int32_t ret = 0;
    int private_data_size = 0;
    struct HdfWifiNetDeviceData *data = NULL;

    (void)chipDriver;
    HDF_LOGE("%s: start...", __func__);

    if (netDevice == NULL) {
        HDF_LOGE("%s:para is null!", __func__);
        return HDF_FAILURE;
    }

    data = GetPlatformData(netDevice);
    if (data == NULL) {
        HDF_LOGE("%s:netdevice data null!", __func__);
        return HDF_FAILURE;
    }

    /* set netdevice ops to netDevice */
    hdf_netdev_init(netDevice);
    netDevice->classDriverName = netDevice->classDriverName;
    netDevice->classDriverPriv = data;
    g_hdf_netdevice = netDevice;

    private_data_size = get_dhd_priv_data_size();
    g_hdf_netdevice->mlPriv = kzalloc(private_data_size, GFP_KERNEL);
    if (g_hdf_netdevice->mlPriv == NULL) {
        printk("%s:kzalloc mlPriv failed\n", __func__);
        return HDF_FAILURE;
    }
    strcpy_s(netDevice->name, sizeof("wlan0"), "wlan0");
    dhd_module_init(netDevice);
    NetDeviceAdd(netDevice);

    HDF_LOGE("%s:NetDeviceAdd success", __func__);

    ret = hdf_netdev_open(netDevice);
    return HDF_SUCCESS;
}

int32_t Ap6212Deinit(struct HdfChipDriver *chipDriver, struct NetDevice *netDevice)
{
    HDF_LOGE("%s: start...", __func__);
    (void)netDevice;
    (void)chipDriver;
    hdf_netdev_stop(netDevice);
    return HDF_SUCCESS;
}

void set_krn_netdev(struct net_device *dev)
{
    g_save_kernel_net = dev;
}

struct net_device *get_krn_netdev(void)
{
    return g_save_kernel_net;
}

void* getDevicePrivateData(void)
{
    return g_hdf_netdevice->mlPriv;
}

struct NetDevice* get_hdf_netdev(void)
{
    return g_hdf_netdevice;
}

EXPORT_SYMBOL(get_hdf_netdev);
EXPORT_SYMBOL(set_krn_netdev);
EXPORT_SYMBOL(get_krn_netdev);
EXPORT_SYMBOL(getDevicePrivateData);