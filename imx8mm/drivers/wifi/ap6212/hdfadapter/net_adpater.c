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

/* ****************************************************************************
  1 头文件包含
**************************************************************************** */
#include <net/cfg80211.h>
#include <linux/netdevice.h>

#include "hdf_base.h"
#include "net_device.h"
#include "eapol.h"
#include "net_adpater.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* ****************************************************************************
  2 全局变量声明
**************************************************************************** */
NetDevice *g_gphdfnetdev;
extern struct net_device_ops dhd_ops_pri;
extern struct wiphy* wrap_get_wiphy(void);
extern struct NetDeviceInterFace *wal_get_net_dev_ops(void);
extern int dhd_netdev_changemtu_wrapper(struct net_device *netdev, int mtu);
extern struct wireless_dev *wl_cfg80211_add_monitor_if_wrap(struct net_device *netdev,
                                                            struct wiphy *wiphy,
                                                            const char *name);
/* ****************************************************************************
  2 全局变量定义
**************************************************************************** */
#define VAL_NUM_0 (0)
#define VAL_NUM_1 (1)
#define VAL_NUM_2 (2)
#define VAL_NUM_3 (3)
#define VAL_NUM_4 (4)
#define VAL_NUM_5 (5)
#define PROTOCOL_12BIT (12)
#define PROTOCOL_13BIT (13)

/* ****************************************************************************
  3 函数实现
**************************************************************************** */
int32_t hdf_netdev_init(struct NetDevice *netDev)
{
    HDF_LOGE("%s: start...", __func__);
    if (netDev == NULL) {
        HDF_LOGE("%s: netDev null!", __func__);
        return HDF_FAILURE;
    }

    HDF_LOGE("%s: netDev->name:%s\n", __func__, netDev->name);
    netDev->netDeviceIf = wal_get_net_dev_ops();
    CreateEapolData(netDev);

    return HDF_SUCCESS;
}

void hdf_netdev_deinit(struct NetDevice *netDev)
{
    HDF_LOGE("%s: start...", __func__);
    (void)netDev;
}

int32_t hdf_netdev_open(struct NetDevice *netDev)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    HDF_LOGE("%s: ndo_stop...", __func__);
    retVal = (int32_t)dhd_ops_pri.ndo_stop(netDev);
    if (retVal < 0) {
        HDF_LOGE("%s: hdf net device stop failed! ret = %d", __func__, retVal);
    }

    retVal = (int32_t)dhd_ops_pri.ndo_open(netDev);
    if (retVal < 0) {
        HDF_LOGE("%s: hdf net device open failed! ret = %d", __func__, retVal);
    }

    dhd_get_mac_address(netDev, netDev->macAddr);
    HDF_LOGE("%s: %02x:%02x:%02x:%02x:%02x:%02x", __func__,
        netDev->macAddr[VAL_NUM_0],
        netDev->macAddr[VAL_NUM_1],
        netDev->macAddr[VAL_NUM_2],
        netDev->macAddr[VAL_NUM_3],
        netDev->macAddr[VAL_NUM_4],
        netDev->macAddr[VAL_NUM_5]);

    struct wireless_dev *wdev = GET_NET_DEV_CFG80211_WIRELESS(netDev);
    g_gphdfnetdev = netDev;

    return retVal;
}

int32_t hdf_netdev_stop(struct NetDevice *netDev)
{
    int32_t retVal = 0;
    HDF_LOGE("%s: start...", __func__);

    if (netDev == NULL) {
        HDF_LOGE("%s: netDev null!", __func__);
        return HDF_FAILURE;
    }

    retVal = (int32_t)dhd_ops_pri.ndo_stop(netDev);
    if (retVal < 0) {
        HDF_LOGE("%s: hdf net device stop failed! ret = %d", __func__, retVal);
    }

    return retVal;
}

int32_t hdf_netdev_xmit(struct NetDevice *netDev, NetBuf *netBuff)
{
    int32_t retVal = 0;

    (void)netDev;
    HDF_LOGI("%s: start...", __func__);
    retVal = (int32_t)dhd_ops_pri.ndo_start_xmit((struct sk_buff *)netBuff, netDev);
    if (retVal < 0) {
        HDF_LOGE("%s: hdf net device xmit failed! ret = %d", __func__, retVal);
    }

    return retVal;
}

int32_t hdf_netdev_ioctl(struct NetDevice *netDev, IfReq *req, int32_t cmd)
{
    int32_t retVal = 0;
    struct ifreq *dhd_req = NULL;
    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    if (netDev == NULL || req == NULL) {
        HDF_LOGE("%s: netdev or req null!", __func__);
        return HDF_FAILURE;
    }

    dhd_req->ifr_ifru.ifru_data = req->ifrData;

    retVal = (int32_t)dhd_ops_pri.ndo_do_ioctl(netDev, dhd_req, cmd);
    if (retVal < 0) {
        HDF_LOGE("%s: hdf net device ioctl failed! ret = %d", __func__, retVal);
    }

    return retVal;
}

int32_t hdf_netdev_setmacaddr(struct NetDevice *netDev, void *addr)
{
    int32_t retVal = 0;
    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    if ((netDev == NULL) || (addr == NULL)) {
        HDF_LOGE("%s: netDev or addr null", __func__);
        return HDF_FAILURE;
    }

    retVal = (int32_t)dhd_ops_pri.ndo_set_mac_address(netDev, addr);
    if (retVal < 0) {
        HDF_LOGE("%s: hdf net device setmacaddr failed! ret = %d", __func__, retVal);
    }

    return retVal;
}

struct NetDevStats *hdf_netdev_getstats(struct NetDevice *netDev)
{
    static struct NetDevStats devStat = {0};
    struct net_device_stats *kdevStat = NULL;

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    if (netDev == NULL) {
        HDF_LOGE("%s: netDev null!", __func__);
        return NULL;
    }

    kdevStat = dhd_ops_pri.ndo_get_stats(netDev);
    if (kdevStat == NULL) {
        HDF_LOGE("%s: ndo_get_stats return null!", __func__);
        return NULL;
    }

    devStat.rxPackets = kdevStat->rx_packets;
    devStat.txPackets = kdevStat->tx_packets;
    devStat.rxBytes = kdevStat->rx_bytes;
    devStat.txBytes = kdevStat->tx_bytes;
    devStat.rxErrors = kdevStat->rx_errors;
    devStat.txErrors = kdevStat->tx_errors;
    devStat.rxDropped = kdevStat->rx_dropped;
    devStat.txDropped = kdevStat->tx_dropped;

    return &devStat;
}

void hdf_netdev_setnetifstats(struct NetDevice *netDev, NetIfStatus status)
{
    HDF_LOGE("%s: start...", __func__);
    (void)netDev;
    (void)status;
}

uint16_t hdf_netdev_selectqueue(struct NetDevice *netDev, NetBuf *netBuff)
{
    HDF_LOGE("%s: start...", __func__);
    (void)netDev;
    (void)netBuff;
    return HDF_SUCCESS;
}

uint32_t hdf_netdev_netifnotify(struct NetDevice *netDev, NetDevNotify *notify)
{
    HDF_LOGE("%s: start...", __func__);
    (void)netDev;
    (void)notify;
    return HDF_SUCCESS;
}

int32_t hdf_netdev_changemtu(struct NetDevice *netDev, int32_t mtu)
{
    int32_t retVal = 0;
    HDF_LOGE("%s: start...", __func__);

    (void)netDev;
    if (netDev == NULL) {
        HDF_LOGE("%s: netdev null!", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: change mtu to %d\n", __FUNCTION__, mtu);

    retVal = (int32_t)dhd_netdev_changemtu_wrapper(netDev, mtu);
    if (retVal < 0) {
        HDF_LOGE("%s: hdf net device chg mtu failed! ret = %d", __func__, retVal);
    }

    return retVal;
}

void hdf_netdev_linkstatuschanged(struct NetDevice *netDev)
{
    HDF_LOGE("%s: start...", __func__);
    (void)netDev;
}

#define BYTE_NUM  (8)
ProcessingResult hdf_netdev_specialethertypeprocess(const struct NetDevice *netDev, NetBuf *buff)
{
    struct EtherHeader *header = NULL;
    uint16_t etherType;
    const struct Eapol *eapolInstance = NULL;
    int ret = HDF_SUCCESS;
    uint16_t protocol;

    HDF_LOGE("%s: start...", __func__);

    if (netDev == NULL || buff == NULL) {
        return PROCESSING_ERROR;
    }

    header = (struct EtherHeader *)NetBufGetAddress(buff, E_DATA_BUF);

    protocol = (buff->data[PROTOCOL_12BIT] << BYTE_NUM) | buff->data[PROTOCOL_13BIT];
    if (protocol != ETHER_TYPE_PAE) {
        HDF_LOGE("%s: return PROCESSING_CONTINUE", __func__);
        return PROCESSING_CONTINUE;
    }
    if (netDev->specialProcPriv == NULL) {
        HDF_LOGE("%s: return PROCESSING_ERROR", __func__);
        return PROCESSING_ERROR;
    }

    eapolInstance = EapolGetInstance();
    ret = eapolInstance->eapolOp->writeEapolToQueue(netDev, buff);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: writeEapolToQueue failed", __func__);
        NetBufFree(buff);
    }
    return PROCESSING_COMPLETE;
}

/*****************************************************************************
  net_device上挂接的net_device_ops函数
**************************************************************************** */
struct NetDeviceInterFace g_wal_net_dev_ops = {
    .init       = hdf_netdev_init,
    .deInit     = hdf_netdev_deinit,
    .open       = hdf_netdev_open,
    .stop       = hdf_netdev_stop,
    .xmit       = hdf_netdev_xmit,
    .ioctl      = hdf_netdev_ioctl,
    .setMacAddr = hdf_netdev_setmacaddr,
    .getStats   = hdf_netdev_getstats,
    .setNetIfStatus     = hdf_netdev_setnetifstats,
    .selectQueue        = hdf_netdev_selectqueue,
    .netifNotify        = hdf_netdev_netifnotify,
    .changeMtu          = hdf_netdev_changemtu,
    .linkStatusChanged  = hdf_netdev_linkstatuschanged,
    .specialEtherTypeProcess  = hdf_netdev_specialethertypeprocess,
};

struct NetDeviceInterFace *wal_get_net_dev_ops(void)
{
    return &g_wal_net_dev_ops;
}

void wal_netif_rx_ni(struct sk_buff *skb)
{
    NetIfRxNi(g_gphdfnetdev, skb);
}

void wal_netif_rx(struct sk_buff *skb)
{
    NetIfRx(g_gphdfnetdev, skb);
}

NetDevice *get_netDev(void)
{
    return g_gphdfnetdev;
}

EXPORT_SYMBOL(get_netDev);
EXPORT_SYMBOL(wal_netif_rx);
EXPORT_SYMBOL(wal_netif_rx_ni);

// #ifdef CHG_FOR_HDF
EXPORT_SYMBOL(hdf_netdev_open);
EXPORT_SYMBOL(hdf_netdev_stop);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
