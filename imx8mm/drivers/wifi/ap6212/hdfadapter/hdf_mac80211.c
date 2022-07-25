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

#include <net/cfg80211.h>
#include <net/regulatory.h>

#include "wifi_mac80211_ops.h"
#include "net_adpater.h"
#include "hdf_wlan_utils.h"
#include "wifi_module.h"
#include "osal_mem.h"
#include "hdf_wifi_event.h"

#define HDF_LOG_TAG Ap6212Driver
#ifndef errno_t
typedef int errno_t;
#endif

extern struct net_device_ops dhd_ops_pri;
extern struct cfg80211_ops wl_cfg80211_ops;

extern NetDevice *get_netDev(void);
extern struct wiphy* wrap_get_wiphy(void);
extern struct wireless_dev* wrap_get_widev(void);
extern const struct ieee80211_regdomain *wrp_get_regdomain(void);
extern int32_t wl_get_all_sta(struct net_device *ndev, uint32_t *num);
extern void dhd_get_mac_address(struct net_device *dev, void **addr);
extern s32 wl_get_all_sta_info(struct net_device *ndev, char* mac, uint32_t num);
extern int32_t wal_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy, struct net_device *netDev);
extern int32_t wl_cfg80211_set_wps_p2p_ie_wrp(struct net_device *ndev, char *buf, int len, int8_t type);
extern int32_t wal_cfg80211_remain_on_channel(struct wiphy *wiphy, struct net_device *netDev, int32_t freq,
                                              unsigned int duration);
extern void wl_cfg80211_add_virtual_iface_wrap(struct wiphy *wiphy, char *name,
    enum nl80211_iftype type, struct vif_params *params);
extern int32_t wl_cfg80211_set_country_code(struct net_device *net, char *country_code,
    bool notify, bool user_enforced, int revinfo);
extern int32_t HdfWifiEventInformBssFrame(const struct NetDevice *netDev,
                                          const struct WlanChannel *channel,
                                          const struct ScannedBssInfo *bssInfo);
extern int32_t wl_cfg80211_change_beacon_wrap(struct wiphy *wiphy, struct net_device *dev,
    struct cfg80211_beacon_data *info, int interval, int dtimPeriod, bool hidden_ssid);

extern errno_t memcpy_s(void *dest, size_t dest_max, const void *src, size_t count);
extern int snprintf_s(char *dest, size_t dest_max, size_t count, const char *format, ...);

typedef enum {
    WLAN_BAND_2G,
    WLAN_BAND_BUTT
} wlan_channel_band_enum;

#define WIFI_24G_CHANNEL_NUMS   (14)
#define WAL_MIN_CHANNEL_2G      (1)
#define WAL_MAX_CHANNEL_2G      (14)
#define WAL_MIN_FREQ_2G         (2412 + 5*(WAL_MIN_CHANNEL_2G - 1))
#define WAL_MAX_FREQ_2G         (2484)
#define WAL_FREQ_2G_INTERVAL    (5)
#define WLAN_WPS_IE_MAX_SIZE    (352) // (WLAN_MEM_EVENT_SIZE2 - 32)   /* 32表示事件自身占用的空间 */
/* Driver supports AP mode */
#define HISI_DRIVER_FLAGS_AP                         0x00000040
/* Driver supports concurrent P2P operations */
#define HISI_DRIVER_FLAGS_P2P_CONCURRENT             0x00000200

#define HISI_DRIVER_FLAGS_P2P_DEDICATED_INTERFACE    0x00000400
/* P2P capable (P2P GO or P2P Client) */
#define HISI_DRIVER_FLAGS_P2P_CAPABLE                0x00000800
/* Driver supports a dedicated interface for P2P Device */
#define HISI_DRIVER_FLAGS_DEDICATED_P2P_DEVICE       0x20000000
#define WAL_MAC_ADDR_LEN         (6)
/*--------------------------------------------------------------------------------------------------*/
/* public */
/*--------------------------------------------------------------------------------------------------*/
static int32_t GetIfName(enum nl80211_iftype type, char *ifName, uint32_t len)
{
    if (ifName == NULL || len == 0) {
        HDF_LOGE("%s:para is null!", __func__);
        return HDF_FAILURE;
    }
    switch (type) {
        case NL80211_IFTYPE_P2P_DEVICE:
            if (snprintf_s(ifName, len, len -1, "p2p%d", 0) < 0) {
                HDF_LOGE("%s:format ifName failed!", __func__);
                return HDF_FAILURE;
            }
            break;
        case NL80211_IFTYPE_P2P_CLIENT:
            /*  fall-through */
        case NL80211_IFTYPE_P2P_GO:
            if (snprintf_s(ifName, len, len -1, "p2p-p2p0-%d", 0) < 0) {
                HDF_LOGE("%s:format ifName failed!", __func__);
                return HDF_FAILURE;
            }
            break;
        default:
            HDF_LOGE("%s:GetIfName::not supported dev type!", __func__);
            return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

void WalReleaseHwCapability(struct WlanHwCapability *self)
{
    uint8_t i;
    if (self == NULL) {
        return;
    }
    for (i = 0; i < IEEE80211_NUM_BANDS; i++) {
        if (self->bands[i] != NULL) {
            OsalMemFree(self->bands[i]);
            self->bands[i] = NULL;
        }
    }
    if (self->supportedRates != NULL) {
        OsalMemFree(self->supportedRates);
        self->supportedRates = NULL;
    }
    OsalMemFree(self);
}

static struct ieee80211_channel *GetChannelByFreq(struct wiphy* wiphy, uint16_t center_freq)
{
    enum Ieee80211Band band;
    struct ieee80211_supported_band *currentBand = NULL;
    int32_t loop;
    for (band = (enum Ieee80211Band)0; band < IEEE80211_NUM_BANDS; band++) {
        currentBand = wiphy->bands[band];
        if (currentBand == NULL) {
            continue;
        }
        for (loop = 0; loop < currentBand->n_channels; loop++) {
            if (currentBand->channels[loop].center_freq == center_freq) {
                return &currentBand->channels[loop];
            }
        }
    }
    return NULL;
}

static struct ieee80211_channel *WalGetChannel(struct wiphy *wiphy, int32_t freq)
{
    int32_t loop = 0;
    enum Ieee80211Band band = IEEE80211_BAND_2GHZ;
    struct ieee80211_supported_band *currentBand = NULL;
    
    if (wiphy == NULL) {
        HDF_LOGE("%s: capality is NULL!", __func__);
        return NULL;
    }

    for (band = (enum Ieee80211Band)0; band < IEEE80211_NUM_BANDS; band++) {
        currentBand = wiphy->bands[band];
        if (currentBand == NULL) {
            continue;
        }

        for (loop = 0; loop < currentBand->n_channels; loop++) {
            if (currentBand->channels[loop].center_freq == freq) {
                return &currentBand->channels[loop];
            }
        }
    }

    return NULL;
}

void inform_bss_frame(struct ieee80211_channel *channel, int32_t signal,
                      int16_t freq, struct ieee80211_mgmt *mgmt, uint32_t mgmtLen)
{
    int32_t retVal = 0;
    NetDevice *netDev = get_netDev();
    struct ScannedBssInfo bssInfo;
    struct WlanChannel hdfchannel;

    if (channel == NULL || netDev == NULL || mgmt == NULL) {
        HDF_LOGE("%s: inform_bss_frame channel = null or netDev = null!", __func__);
        return;
    }

    bssInfo.signal = signal;
    bssInfo.freq = freq;
    bssInfo.mgmtLen = mgmtLen;
    bssInfo.mgmt = (struct Ieee80211Mgmt *)mgmt;

    hdfchannel.flags = channel->flags;
    hdfchannel.channelId = channel->hw_value;
    hdfchannel.centerFreq = channel->center_freq;

    HDF_LOGE("%s: hdfchannel flags:%d--channelId:%d--centerFreq:%d--dstMac:%02x:%02x:%02x:%02x:%02x:%02x!",
        __func__, hdfchannel.flags, hdfchannel.channelId, hdfchannel.centerFreq,
        bssInfo.mgmt->bssid[0], bssInfo.mgmt->bssid[1], bssInfo.mgmt->bssid[2],
        bssInfo.mgmt->bssid[3], bssInfo.mgmt->bssid[4], bssInfo.mgmt->bssid[5]);

    retVal = HdfWifiEventInformBssFrame(netDev, &hdfchannel, &bssInfo);
    if (retVal < 0) {
        HDF_LOGE("%s: hdf wifi event inform bss frame failed!", __func__);
    }
}

#define HDF_ETHER_ADDR_LEN (6)
void inform_connect_result(uint8_t *bssid, uint8_t *rspIe,
                           uint8_t *reqIe, uint32_t reqIeLen,
                           uint32_t rspIeLen, uint16_t connectStatus)
{
    int32_t retVal = 0;
    NetDevice *netDev = get_netDev();
    struct ConnetResult connResult;

    if (netDev == NULL || bssid == NULL || rspIe == NULL || reqIe == NULL) {
        HDF_LOGE("%s: netDev / bssid / rspIe / reqIe  null!", __func__);
        return;
    }

    memcpy_s(&connResult.bssid[0], HDF_ETHER_ADDR_LEN, bssid, HDF_ETHER_ADDR_LEN);
    HDF_LOGE("%s: connResult:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
        connResult.bssid[0], connResult.bssid[1], connResult.bssid[2],
        connResult.bssid[3], connResult.bssid[4], connResult.bssid[5]);

    connResult.rspIe = rspIe;
    connResult.rspIeLen = rspIeLen;

    connResult.reqIe = reqIe;
    connResult.reqIeLen = reqIeLen;

    connResult.connectStatus = connectStatus;

    connResult.freq = 0;
    connResult.statusCode = connectStatus;

    retVal = HdfWifiEventConnectResult(netDev, &connResult);
    if (retVal < 0) {
        HDF_LOGE("%s: hdf wifi event inform connect result failed!", __func__);
    }
}

struct wireless_dev g_ap_wireless_dev;
struct ieee80211_channel g_ap_ieee80211_channel;
#define GET_DEV_CFG80211_WIRELESS(dev) ((struct wireless_dev*)((dev)->ieee80211Ptr))
static int32_t SetupWireLessDev(struct NetDevice *netDev, struct WlanAPConf *apSettings)
{
    if (netDev->ieee80211Ptr == NULL) {
        netDev->ieee80211Ptr = &g_ap_wireless_dev;
    }

    if (GET_DEV_CFG80211_WIRELESS(netDev)->preset_chandef.chan == NULL) {
        GET_DEV_CFG80211_WIRELESS(netDev)->preset_chandef.chan = &g_ap_ieee80211_channel;
    }
    GET_DEV_CFG80211_WIRELESS(netDev)->iftype = NL80211_IFTYPE_AP;
    GET_DEV_CFG80211_WIRELESS(netDev)->preset_chandef.width = (enum nl80211_channel_type)apSettings->width;
    GET_DEV_CFG80211_WIRELESS(netDev)->preset_chandef.center_freq1 = apSettings->centerFreq1;
    GET_DEV_CFG80211_WIRELESS(netDev)->preset_chandef.center_freq2 = apSettings->centerFreq2;
    GET_DEV_CFG80211_WIRELESS(netDev)->preset_chandef.chan->hw_value = apSettings->channel;
    GET_DEV_CFG80211_WIRELESS(netDev)->preset_chandef.chan->band = IEEE80211_BAND_2GHZ;
    GET_DEV_CFG80211_WIRELESS(netDev)->preset_chandef.chan->center_freq = apSettings->centerFreq1;

    return HDF_SUCCESS;
}

/*--------------------------------------------------------------------------------------------------*/
/* HdfMac80211BaseOps */
/*--------------------------------------------------------------------------------------------------*/
int32_t WalSetMode(NetDevice *netDev, enum WlanWorkMode iftype)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    HDF_LOGE("%s: start... iftype=%d ", __func__, iftype);
    retVal = (int32_t)wl_cfg80211_ops.change_virtual_intf(wiphy, netDev,
        (enum nl80211_iftype)iftype, NULL);
    if (retVal < 0) {
        HDF_LOGE("%s: set mode failed!", __func__);
    }

    return retVal;
}

struct key_params gKeyParameter;
int32_t WalAddKey(struct NetDevice *netDev, uint8_t keyIndex, bool pairwise, const uint8_t *macAddr,
    struct KeyParams *params)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    HDF_LOGE("%s: start...", __func__);

    gKeyParameter.key = params->key;
    gKeyParameter.seq = params->seq;

    gKeyParameter.key_len = params->keyLen;
    gKeyParameter.seq_len = params->seqLen;

    gKeyParameter.cipher = params->cipher;
    (void)netDev;
    retVal = (int32_t)wl_cfg80211_ops.add_key(wiphy, netDev, keyIndex, pairwise, macAddr,
                                              (struct key_params *)(&gKeyParameter));
    if (retVal < 0) {
        HDF_LOGE("%s: add key failed!", __func__);
    }

    return retVal;
}

int32_t WalDelKey(struct NetDevice *netDev, uint8_t keyIndex, bool pairwise, const uint8_t *macAddr)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    HDF_LOGE("%s: start...", __func__);

    (void)netDev;
    retVal = (int32_t)wl_cfg80211_ops.del_key(wiphy, netDev, keyIndex, pairwise, macAddr);
    if (retVal < 0) {
        HDF_LOGE("%s: delete key failed!", __func__);
    }

    return retVal;
}

int32_t WalSetDefaultKey(struct NetDevice *netDev, uint8_t keyIndex, bool unicast, bool multicas)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    HDF_LOGE("%s: start...", __func__);
    
    retVal = (int32_t)wl_cfg80211_ops.set_default_key(wiphy, netDev, keyIndex, unicast, multicas);
    if (retVal < 0) {
        HDF_LOGE("%s: set default key failed!", __func__);
    }

    return retVal;
}

int32_t WalGetDeviceMacAddr(NetDevice *netDev, int32_t type, uint8_t *mac, uint8_t len)
{
    (void)len;
    (void)type;
    (void)netDev;
    HDF_LOGE("%s: start...", __func__);
    
    dhd_get_mac_address(netDev, (void**)&mac);

    return HDF_SUCCESS;
}

int32_t WalSetMacAddr(NetDevice *netDev, uint8_t *mac, uint8_t len)
{
    int32_t retVal = 0;
    uint8_t zero_mac_addr[WAL_MAC_ADDR_LEN] = {0};

    (void)len;
    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    if ((memcmp(zero_mac_addr, mac, WAL_MAC_ADDR_LEN) == 0)
        || ((mac[0] & 0x1) == 0x1)) {
        HDF_LOGE("%s: mac address error !", __func__);
        return HDF_FAILURE;
    }

    retVal = (int32_t)dhd_ops_pri.ndo_set_mac_address(netDev, mac);
    if (retVal < 0) {
        HDF_LOGE("%s: set mac address failed!", __func__);
    }

    return retVal;
}

int32_t WalSetTxPower(NetDevice *netDev, int32_t power)
{
    int retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();
    struct wireless_dev *wdev = GET_NET_DEV_CFG80211_WIRELESS(netDev);

    HDF_LOGE("%s: start...", __func__);

    retVal = (int32_t)wl_cfg80211_ops.set_tx_power(wiphy, wdev, NL80211_TX_POWER_FIXED, power);
    if (retVal < 0) {
        HDF_LOGE("%s: set_tx_power failed!", __func__);
    }

    return HDF_SUCCESS;
}

int32_t WalGetValidFreqsWithBand(NetDevice *netDev, int32_t band, int32_t *freqs, uint32_t *num)
{
    uint32_t freqIndex = 0;
    uint32_t channelNumber;
    uint32_t freqTmp;
    uint32_t minFreq;
    uint32_t maxFreq;
    const struct ieee80211_regdomain *regdom = wrp_get_regdomain();
    if (regdom == NULL) {
        HDF_LOGE("%s: wal_get_cfg_regdb failed!", __func__);
        return HDF_FAILURE;
    }

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    minFreq = regdom->reg_rules[0].freq_range.start_freq_khz / MHZ_TO_KHZ(1);
    maxFreq = regdom->reg_rules[0].freq_range.end_freq_khz / MHZ_TO_KHZ(1);
    switch (band) {
        case WLAN_BAND_2G:
            for (channelNumber = 1; channelNumber <= WIFI_24G_CHANNEL_NUMS; channelNumber++) {
                if (channelNumber < WAL_MAX_CHANNEL_2G) {
                    freqTmp = WAL_MIN_FREQ_2G + (channelNumber - 1) * WAL_FREQ_2G_INTERVAL;
                } else if (channelNumber == WAL_MAX_CHANNEL_2G) {
                    freqTmp = WAL_MAX_FREQ_2G;
                }
                if (freqTmp < minFreq || freqTmp > maxFreq) {
                    continue;
                }
                freqs[freqIndex] = freqTmp;
                freqIndex++;
            }
            *num = freqIndex;
            break;
        default:
            HDF_LOGE("%s: no support band!", __func__);
            return HDF_ERR_NOT_SUPPORT;
    }
    return HDF_SUCCESS;
}

int32_t WalGetHwCapability(struct NetDevice *netDev, struct WlanHwCapability **capability)
{
    uint8_t loop = 0;
    struct wiphy* wiphy = wrap_get_wiphy();
    struct ieee80211_supported_band *band = wiphy->bands[IEEE80211_BAND_2GHZ];
    struct WlanHwCapability *hwCapability = (struct WlanHwCapability *)OsalMemCalloc(sizeof(struct WlanHwCapability));

    (void)netDev;
    if (hwCapability == NULL) {
        HDF_LOGE("%s: oom!\n", __func__);
        return HDF_FAILURE;
    }
    hwCapability->Release = WalReleaseHwCapability;
    if (hwCapability->bands[IEEE80211_BAND_2GHZ] == NULL) {
        hwCapability->bands[IEEE80211_BAND_2GHZ] =
            OsalMemCalloc(sizeof(struct WlanBand) + (sizeof(struct WlanChannel) * band->n_channels));
        if (hwCapability->bands[IEEE80211_BAND_2GHZ] == NULL) {
            HDF_LOGE("%s: oom!\n", __func__);
            WalReleaseHwCapability(hwCapability);
            return HDF_FAILURE;
        }
    }
    hwCapability->htCapability = band->ht_cap.cap;
    hwCapability->bands[IEEE80211_BAND_2GHZ]->channelCount = band->n_channels;
    for (loop = 0; loop < band->n_channels; loop++) {
        hwCapability->bands[IEEE80211_BAND_2GHZ]->channels[loop].centerFreq = band->channels[loop].center_freq;
        hwCapability->bands[IEEE80211_BAND_2GHZ]->channels[loop].flags = band->channels[loop].flags;
        hwCapability->bands[IEEE80211_BAND_2GHZ]->channels[loop].channelId = band->channels[loop].hw_value;
    }
    hwCapability->supportedRateCount = band->n_bitrates;
    hwCapability->supportedRates = OsalMemCalloc(sizeof(uint16_t) * band->n_bitrates);
    if (hwCapability->supportedRates == NULL) {
        HDF_LOGE("%s: oom!\n", __func__);
        WalReleaseHwCapability(hwCapability);
        return HDF_FAILURE;
    }
    for (loop = 0; loop < band->n_bitrates; loop++) {
        hwCapability->supportedRates[loop] = band->bitrates[loop].bitrate;
    }
    *capability = hwCapability;
    return HDF_SUCCESS;
}

int32_t WalRemainOnChannel(struct NetDevice *netDev, WifiOnChannel *onChannel)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);
    if (netDev == NULL || onChannel == NULL) {
        HDF_LOGE("%s:NULL ptr!", __func__);
        return HDF_FAILURE;
    }

    retVal = (int32_t)wal_cfg80211_remain_on_channel(wiphy, netDev, onChannel->freq, onChannel->duration);
    if (retVal < 0) {
        HDF_LOGE("%s: remain on channel failed!", __func__);
    }

    return retVal;
}

int32_t WalCancelRemainOnChannel(struct NetDevice *netDev)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);
    if (netDev == NULL) {
        HDF_LOGE("%s:NULL ptr!", __func__);
        return HDF_FAILURE;
    }

    retVal = (int32_t)wal_cfg80211_cancel_remain_on_channel(wiphy, netDev);
    if (retVal < 0) {
        HDF_LOGE("%s: cancel remain on channel failed!", __func__);
    }

    return retVal;
}

int32_t WalProbeReqReport(struct NetDevice *netDev, int32_t report)
{
    // NO SUPPORT
    (void)report;
    HDF_LOGE("%s: start...", __func__);
    if (netDev == NULL) {
        HDF_LOGE("%s:NULL ptr!", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t WalAddIf(struct NetDevice *netDev, WifiIfAdd *ifAdd)
{
    char ifName[16] = {0};
    struct wiphy* wiphy = wrap_get_wiphy();

    HDF_LOGE("%s: start...", __func__);

    GetIfName(ifAdd->type, ifName, 16);
    wl_cfg80211_ops.add_virtual_intf(wiphy, ifName, 0, ifAdd->type, NULL);

    return HDF_SUCCESS;
}

int32_t WalRemoveIf(struct NetDevice *netDev, WifiIfRemove *ifRemove)
{
    struct NetDevice *removeNetdev = NULL;
    struct wireless_dev *wdev = NULL;
    struct wiphy* wiphy = wrap_get_wiphy();

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);
    if (ifRemove == NULL) {
        HDF_LOGE("%s:NULL ptr!", __func__);
        return HDF_FAILURE;
    }

    removeNetdev = NetDeviceGetInstByName((const char*)(ifRemove->ifName));
    if (removeNetdev == NULL) {
        return HDF_FAILURE;
    }

    wdev = GET_NET_DEV_CFG80211_WIRELESS(removeNetdev);
    wl_cfg80211_ops.del_virtual_intf(wiphy, wdev);

    return HDF_SUCCESS;
}

int32_t WalSetApWpsP2pIe(struct NetDevice *netDev, WifiAppIe *appIe)
{
    int32_t retVal = 0;

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);
    if (netDev == NULL || appIe == NULL) {
        HDF_LOGE("%s:NULL ptr!", __func__);
        return HDF_FAILURE;
    }

    if (appIe->ieLen > WLAN_WPS_IE_MAX_SIZE) {
        HDF_LOGE("%s:app ie length is too large!", __func__);
        return HDF_FAILURE;
    }

    retVal = (int32_t)wl_cfg80211_set_wps_p2p_ie_wrp(netDev, appIe->ie, appIe->ieLen, (int8_t)appIe->appIeType);
    if (retVal < 0) {
        HDF_LOGE("%s: set_wps_p2p_ie failed!", __func__);
    }

    return retVal;
}

int32_t WalGetDriverFlag(struct NetDevice *netDev, WifiGetDrvFlags **params)
{
    struct wireless_dev *wdev = GET_NET_DEV_CFG80211_WIRELESS(netDev);
    WifiGetDrvFlags *getDrvFlag = (WifiGetDrvFlags *)OsalMemCalloc(sizeof(WifiGetDrvFlags));

    if (netDev == NULL || params == NULL) {
        HDF_LOGE("%s:NULL ptr!", __func__);
        return HDF_FAILURE;
    }

    HDF_LOGE("%s: start...", __func__);

    if (wdev == NULL) {
        HDF_LOGE("%s: wdev NULL", __func__);
        return HDF_FAILURE;
    } else {
        HDF_LOGE("%s: p_wdev:%p", __func__, wdev);
    }

    if (getDrvFlag == NULL) {
        HDF_LOGE("%s: getDrvFlag NULL", __func__);
    }
    switch (wdev->iftype) {
        case NL80211_IFTYPE_P2P_CLIENT:
             /* fall-through */
        case NL80211_IFTYPE_P2P_GO:
            HDF_LOGE("%s: NL80211_IFTYPE_P2P_GO case!", __func__);
            getDrvFlag->drvFlags = HISI_DRIVER_FLAGS_AP;
            break;
        case NL80211_IFTYPE_P2P_DEVICE:
            HDF_LOGE("%s: NL80211_IFTYPE_P2P_DEVICE case!", __func__);
            getDrvFlag->drvFlags = (HISI_DRIVER_FLAGS_P2P_DEDICATED_INTERFACE |
                                            HISI_DRIVER_FLAGS_P2P_CONCURRENT |
                                            HISI_DRIVER_FLAGS_P2P_CAPABLE);
            break;
        default:
            HDF_LOGE("%s: getDrvFlag->drvFlags default to 0 case!", __func__);
            getDrvFlag->drvFlags = 0;
    }
    *params = getDrvFlag;
    return HDF_SUCCESS;
}

int32_t WalSendAction(struct NetDevice *netDev, WifiActionData *actionData)
{
    (void)netDev;
    (void)actionData;
    HDF_LOGE("%s: start...", __func__);
    return HDF_ERR_NOT_SUPPORT;
}

int32_t WalGetIftype(struct NetDevice *netDev, uint8_t *iftype)
{
    iftype = (uint8_t *)(&(GET_NET_DEV_CFG80211_WIRELESS(netDev)->iftype));
    HDF_LOGE("%s: start...", __func__);
    return HDF_SUCCESS;
}

/*--------------------------------------------------------------------------------------------------*/
/* HdfMac80211STAOps */
/*--------------------------------------------------------------------------------------------------*/
static int32_t WalConnect(NetDevice *netDev, WlanConnectParams *param)
{
    int ret = 0;
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();
    struct cfg80211_connect_params cfg80211_params = { 0 };

    HDF_LOGE("%s: start ...!", __func__);

    (void)netDev;

    if (param->centerFreq != WLAN_FREQ_NOT_SPECFIED) {
        cfg80211_params.channel = WalGetChannel(wiphy, param->centerFreq);
        if ((cfg80211_params.channel == NULL) || (cfg80211_params.channel->flags & WIFI_CHAN_DISABLED)) {
            HDF_LOGE("%s:illegal channel.flags=%u", __func__,
                (cfg80211_params.channel == NULL) ? 0 : cfg80211_params.channel->flags);
            return HDF_FAILURE;
        }
    }
    cfg80211_params.bssid = param->bssid;
    cfg80211_params.ssid = param->ssid;
    cfg80211_params.ie = param->ie;
    cfg80211_params.ssid_len = param->ssidLen;
    cfg80211_params.ie_len = param->ieLen;
    ret = memcpy_s(&cfg80211_params.crypto, sizeof(cfg80211_params.crypto), &param->crypto, sizeof(param->crypto));
    if (ret != 0) {
        HDF_LOGE("%s:Copy crypto info failed!ret=%d", __func__, ret);
        return HDF_FAILURE;
    }
    cfg80211_params.key = param->key;
    cfg80211_params.auth_type = (u_int8_t)param->authType;
    cfg80211_params.privacy = param->privacy;
    cfg80211_params.key_len = param->keyLen;
    cfg80211_params.key_idx = param->keyIdx;
    cfg80211_params.mfp = (u_int8_t)param->mfp;

    retVal = wl_cfg80211_ops.connect(wiphy, netDev, &cfg80211_params);
    if (retVal < 0) {
        HDF_LOGE("%s: connect failed!\n", __func__);
    }

    return retVal;
}

int32_t WalDisconnect(NetDevice *netDev, uint16_t reasonCode)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    (void)netDev;
    HDF_LOGE("%s: start...reasonCode:%d", __func__, reasonCode);

    retVal = (int32_t)wl_cfg80211_ops.disconnect(wiphy, netDev, reasonCode);
    if (retVal < 0) {
        HDF_LOGE("%s: sta disconnect failed!", __func__);
    }

    return retVal;
}

int32_t WalBuildChannelInfo(struct wiphy* wiphy, struct WlanScanRequest *scanParam,
                            struct cfg80211_scan_request **request)
{
    int32_t loop;
    int32_t count = 0;
    struct ieee80211_channel *chan = NULL;
    enum Ieee80211Band band = IEEE80211_BAND_2GHZ;

    // channel info
    if ((scanParam->freqs == NULL) || (scanParam->freqsCount == 0)) {
        if (wiphy->bands[band] == NULL) {
            HDF_LOGE("%s: wiphy->bands[band] = NULL!\n", __func__);
            return HDF_FAILURE;
        }

        for (loop = 0; loop < (int32_t)wiphy->bands[band]->n_channels; loop++) {
            chan = &wiphy->bands[band]->channels[loop];
            if ((chan->flags & WIFI_CHAN_DISABLED) != 0) {
                continue;
            }
            (*request)->channels[count++] = chan;
        }
    } else {
        for (loop = 0; loop < scanParam->freqsCount; loop++) {
            chan = GetChannelByFreq(wiphy, (uint16_t)(scanParam->freqs[loop]));
            if (chan == NULL) {
                continue;
            }

            (*request)->channels[count++] = chan;
        }
    }

    if (count == 0) {
        HDF_LOGE("%s: invalid freq info!\n", __func__);
        return HDF_FAILURE;
    }
    (*request)->n_channels = count;

    return HDF_SUCCESS;
}

int32_t WalBuildSSidInfo(struct WlanScanRequest *scanParam, struct cfg80211_scan_request **request)
{
    int32_t loop, count = 0, retVal = 0;
    if (scanParam->ssidCount > WPAS_MAX_SCAN_SSIDS) {
        HDF_LOGE("%s:unexpected numSsids!numSsids=%u", __func__, scanParam->ssidCount);
        return HDF_FAILURE;
    }

    if (scanParam->ssidCount == 0) {
        HDF_LOGE("%s:ssid number is 0!", __func__);
        return HDF_SUCCESS;
    }

    (*request)->ssids = (struct cfg80211_ssid *)OsalMemCalloc(scanParam->ssidCount * sizeof(struct cfg80211_ssid));
    if ((*request)->ssids == NULL) {
        HDF_LOGE("%s: calloc request->ssids null", __func__);
        return HDF_FAILURE;
    }

    for (loop = 0; loop < scanParam->ssidCount; loop++) {
        if (count >= DRIVER_MAX_SCAN_SSIDS) {
            break;
        }

        if (scanParam->ssids[loop].ssidLen > IEEE80211_MAX_SSID_LEN) {
            continue;
        }

        (*request)->ssids[count].ssid_len = scanParam->ssids[loop].ssidLen;
        if (memcpy_s((*request)->ssids[count].ssid, OAL_IEEE80211_MAX_SSID_LEN, scanParam->ssids[loop].ssid,
            scanParam->ssids[loop].ssidLen) != 0) {
            continue;
        }
        count++;
    }
    (*request)->n_ssids = count;

    return HDF_SUCCESS;
}

struct cfg80211_scan_request g_request;
int32_t WalStartScan(NetDevice *netDev, struct WlanScanRequest *scanParam)
{
    int32_t loop, count = 0, retVal = 0;
    enum Ieee80211Band band = IEEE80211_BAND_2GHZ;
    struct ieee80211_channel *chan = NULL;
    struct wiphy* wiphy = wrap_get_wiphy();
    struct cfg80211_scan_request *request = &g_request;
    
    if (request == NULL) {
        return HDF_FAILURE;
    }

    // basic info
    request->wiphy = wiphy;
    request->wdev = GET_NET_DEV_CFG80211_WIRELESS(netDev);
    request->n_ssids = scanParam->ssidCount;

    if (WalBuildChannelInfo(wiphy, scanParam, &request) == HDF_FAILURE) {
        return HDF_FAILURE;
    }

    // ssid info
    if (WalBuildSSidInfo(scanParam, &request) == HDF_FAILURE) {
        return HDF_FAILURE;
    }

    // User Ie Info
    if (scanParam->extraIEsLen > 512) {
        return HDF_FAILURE;
    }

    if ((scanParam->extraIEs != NULL) && (scanParam->extraIEsLen != 0)) {
        request->ie = (uint8_t *)OsalMemCalloc(scanParam->extraIEsLen);
        if (request->ie == NULL) {
            if (request->ie != NULL) {
                OsalMemFree((void*)request->ie);
                request->ie = NULL;
            }

            return HDF_FAILURE;
        }
        (void)memcpy_s((void*)request->ie, scanParam->extraIEsLen, scanParam->extraIEs, scanParam->extraIEsLen);
        request->ie_len = scanParam->extraIEsLen;
    }

    retVal = (int32_t)wl_cfg80211_ops.scan(wiphy, request);
    if (retVal < 0) {
        if (request != NULL) {
            if ((request)->ie != NULL) {
                OsalMemFree((void*)(request->ie));
                request->ie = NULL;
            }
            if ((request)->ssids != NULL) {
                OsalMemFree(request->ssids);
                (request)->ssids = NULL;
            }
            OsalMemFree(request);
            request = NULL;
        }
    }

    return retVal;
}

int32_t WalAbortScan(NetDevice *netDev)
{
    struct wiphy* wiphy = wrap_get_wiphy();
    struct wireless_dev* wdev = wrap_get_widev();

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);
    wl_cfg80211_ops.abort_scan(wiphy, wdev);

    return HDF_SUCCESS;
}

int32_t WalSetScanningMacAddress(NetDevice *netDev, unsigned char *mac, uint32_t len)
{
    (void)netDev;
    (void)mac;
    (void)len;
    return HDF_ERR_NOT_SUPPORT;
}

/*--------------------------------------------------------------------------------------------------*/
/* HdfMac80211APOps */
/*--------------------------------------------------------------------------------------------------*/
struct cfg80211_ap_settings g_ap_setting_info;
u8 g_ap_ssid[IEEE80211_MAX_SSID_LEN];
struct ieee80211_channel g_ap_chan;
int32_t WalConfigAp(NetDevice *netDev, struct WlanAPConf *apConf)
{
    int32_t ret = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    (void)netDev;
    ret = SetupWireLessDev(netDev, apConf);
    if (ret != 0) {
        HDF_LOGE("%s: set up wireless device failed!", __func__);
        return HDF_FAILURE;
    }

    HDF_LOGE("%s: before iftype = %d!", __func__, ((struct wireless_dev*)(netDev->ieee80211Ptr))->iftype);
    ((struct wireless_dev*)(netDev->ieee80211Ptr))->iftype = NL80211_IFTYPE_AP;
    HDF_LOGE("%s: after  iftype = %d!", __func__, ((struct wireless_dev*)(netDev->ieee80211Ptr))->iftype);

    g_ap_setting_info.ssid_len = apConf->ssidConf.ssidLen;
    memcpy_s(&g_ap_ssid[0], apConf->ssidConf.ssidLen, &apConf->ssidConf.ssid[0], apConf->ssidConf.ssidLen);

    g_ap_setting_info.ssid = &g_ap_ssid[0];
    g_ap_setting_info.chandef.center_freq1 = apConf->centerFreq1;
    g_ap_setting_info.chandef.center_freq2 = apConf->centerFreq2;
    g_ap_setting_info.chandef.width = apConf->width;

    g_ap_chan.center_freq = apConf->centerFreq1;
    g_ap_chan.hw_value = apConf->channel;
    g_ap_chan.band = apConf->band;
    g_ap_chan.band = IEEE80211_BAND_2GHZ;
    g_ap_setting_info.chandef.chan = &g_ap_chan;

    ret = (int32_t)wl_cfg80211_ops.change_virtual_intf(wiphy, netDev,
        (enum nl80211_iftype)(((struct wireless_dev*)(netDev->ieee80211Ptr))->iftype), NULL);
    if (ret < 0) {
        HDF_LOGE("%s: set mode failed!", __func__);
    }

    return HDF_SUCCESS;
}

int32_t WalStartAp(NetDevice *netDev)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);
    GET_DEV_CFG80211_WIRELESS(netDev)->preset_chandef.chan->center_freq = \
                             ((struct wireless_dev*)(netDev->ieee80211Ptr))->preset_chandef.center_freq1;
    retVal = (int32_t)wl_cfg80211_ops.start_ap(wiphy, netDev, &g_ap_setting_info);
    if (retVal < 0) {
        HDF_LOGE("%s: start_ap failed!", __func__);
    }

    return retVal;
}

int32_t WalStopAp(NetDevice *netDev)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    retVal = (int32_t)wl_cfg80211_ops.stop_ap(wiphy, netDev);
    if (retVal < 0) {
        HDF_LOGE("%s: stop_ap failed!", __func__);
    }

    return retVal;
}

int32_t WalChangeBeacon(NetDevice *netDev, struct WlanBeaconConf *param)
{
    int32_t retVal = 0;
    struct cfg80211_beacon_data info;
    struct wiphy* wiphy = wrap_get_wiphy();

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    memset_s(&info, sizeof(info), 0x00, sizeof(info));
    info.head = param->headIEs;
    info.head_len = (size_t)param->headIEsLength;
    info.tail = param->tailIEs;
    info.tail_len = (size_t)param->tailIEsLength;

    info.beacon_ies = NULL;
    info.proberesp_ies = NULL;
    info.assocresp_ies = NULL;
    info.probe_resp = NULL;
    info.lci = NULL;
    info.civicloc = NULL;
    info.ftm_responder = 0X00;
    info.beacon_ies_len = 0X00;
    info.proberesp_ies_len = 0X00;
    info.assocresp_ies_len = 0X00;
    info.probe_resp_len = 0X00;
    info.lci_len = 0X00;
    info.civicloc_len = 0X00;

    // add beacon data for start ap
    g_ap_setting_info.dtim_period = param->DTIMPeriod;
    g_ap_setting_info.hidden_ssid = param->hiddenSSID;
    g_ap_setting_info.beacon_interval = param->interval;

    g_ap_setting_info.beacon.head = param->headIEs;
    g_ap_setting_info.beacon.head_len = param->headIEsLength;
    g_ap_setting_info.beacon.tail = param->tailIEs;
    g_ap_setting_info.beacon.tail_len = param->tailIEsLength;

    g_ap_setting_info.beacon.beacon_ies = NULL;
    g_ap_setting_info.beacon.proberesp_ies = NULL;
    g_ap_setting_info.beacon.assocresp_ies = NULL;
    g_ap_setting_info.beacon.probe_resp = NULL;
    g_ap_setting_info.beacon.lci = NULL;
    g_ap_setting_info.beacon.civicloc = NULL;
    g_ap_setting_info.beacon.ftm_responder = 0X00;
    g_ap_setting_info.beacon.beacon_ies_len = 0X00;
    g_ap_setting_info.beacon.proberesp_ies_len = 0X00;
    g_ap_setting_info.beacon.assocresp_ies_len = 0X00;
    g_ap_setting_info.beacon.probe_resp_len = 0X00;
    g_ap_setting_info.beacon.lci_len = 0X00;
    g_ap_setting_info.beacon.civicloc_len = 0X00;

    HDF_LOGE("%s: headIEsLen:%d---tailIEsLen:%d!", __func__, param->headIEsLength, param->tailIEsLength);

    retVal = (int32_t)wl_cfg80211_ops.change_beacon(wiphy, netDev, &info);
    if (retVal < 0) {
        HDF_LOGE("%s: change_beacon failed!", __func__);
    }

    return HDF_SUCCESS;
}

int32_t WalDelStation(NetDevice *netDev, const uint8_t *macAddr)
{
    int32_t retVal = 0;
    struct wiphy* wiphy = wrap_get_wiphy();
    struct station_del_parameters del_param = {macAddr, 10, 0};

    (void)netDev;
    (void)macAddr;
    HDF_LOGE("%s: start...", __func__);

    retVal = (int32_t)wl_cfg80211_ops.del_station(wiphy, netDev, &del_param);
    if (retVal < 0) {
        HDF_LOGE("%s: del_station failed!", __func__);
    }

    return retVal;
}

int32_t WalSetCountryCode(NetDevice *netDev, const char *code, uint32_t len)
{
    int32_t retVal = 0;

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    retVal = (int32_t)wl_cfg80211_set_country_code(netDev, (char*)code, false, true, len);
    if (retVal < 0) {
        HDF_LOGE("%s: set_country_code failed!", __func__);
    }

    return retVal;
}

int32_t WalGetAssociatedStasCount(NetDevice *netDev, uint32_t *num)
{
    int32_t retVal = 0;

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    retVal = (int32_t)wl_get_all_sta(netDev, num);
    if (retVal < 0) {
        HDF_LOGE("%s: wl_get_all_sta failed!", __func__);
    }
    return retVal;
}

int32_t WalGetAssociatedStasInfo(NetDevice *netDev, WifiStaInfo *staInfo, uint32_t num)
{
    int32_t retVal = 0;

    (void)netDev;
    HDF_LOGE("%s: start...", __func__);

    retVal = (int32_t)wl_get_all_sta_info(netDev, staInfo->mac, num);
    if (retVal < 0) {
        HDF_LOGE("%s: wl_get_all_sta_info failed!", __func__);
    }

    return retVal;
}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
static struct HdfMac80211BaseOps g_baseOps = {
    .SetMode = WalSetMode,
    .AddKey = WalAddKey,
    .DelKey = WalDelKey,
    .SetDefaultKey = WalSetDefaultKey,
    .GetDeviceMacAddr = WalGetDeviceMacAddr,
    .SetMacAddr = WalSetMacAddr,
    .SetTxPower = WalSetTxPower,
    .GetValidFreqsWithBand = WalGetValidFreqsWithBand,
    .GetHwCapability = WalGetHwCapability,
    .SendAction = WalSendAction,
    .GetIftype = WalGetIftype,
};

static struct HdfMac80211P2POps g_p2pOps = {
    .ProbeReqReport = WalProbeReqReport,
    .AddIf = WalAddIf,
    .RemoveIf = WalRemoveIf,
    .SetApWpsP2pIe = WalSetApWpsP2pIe,
    .GetDriverFlag = WalGetDriverFlag,
    .RemainOnChannel = WalRemainOnChannel,
    .CancelRemainOnChannel = WalCancelRemainOnChannel,
};

static struct HdfMac80211STAOps g_staOps = {
    .Connect = WalConnect,
    .Disconnect = WalDisconnect,
    .StartScan = WalStartScan,
    .AbortScan = WalAbortScan,
    .SetScanningMacAddress = WalSetScanningMacAddress,
};

static struct HdfMac80211APOps g_apOps = {
    .ConfigAp = WalConfigAp,
    .StartAp = WalStartAp,
    .StopAp = WalStopAp,
    .ConfigBeacon = WalChangeBeacon,
    .DelStation = WalDelStation,
    .SetCountryCode = WalSetCountryCode,
    .GetAssociatedStasCount = WalGetAssociatedStasCount,
    .GetAssociatedStasInfo = WalGetAssociatedStasInfo
};

void ApMac80211Init(struct HdfChipDriver *chipDriver)
{
    HDF_LOGE("%s: start...", __func__);

    if (chipDriver == NULL) {
        HDF_LOGE("%s: input is NULL", __func__);
        return;
    }
    chipDriver->ops = &g_baseOps;
    chipDriver->staOps = &g_staOps;
    chipDriver->apOps = &g_apOps;
    chipDriver->p2pOps = &g_p2pOps;
}

EXPORT_SYMBOL(inform_bss_frame);
EXPORT_SYMBOL(inform_connect_result);