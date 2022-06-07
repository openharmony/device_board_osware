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

#include "nxpeth_mac.h"
#include "osal_mem.h"

void NxpethMacCoreInit(void)
{
    return;
}

int32_t NxpethPortReset(struct EthDevice *ethDevice)
{
    return HDF_SUCCESS;
}

int32_t NxpethPortInit(struct EthDevice *ethDevice)
{
    return HDF_SUCCESS;
}

static struct EthMacOps g_macOps = {
    .MacInit = NxpethMacCoreInit,
    .PortReset = NxpethPortReset,
    .PortInit = NxpethPortInit,
};

struct HdfEthMacChipDriver *BuildNxpMacDriver(void)
{
    HDF_LOGE("BuildNxpMacDriver start !!!!!!!!!!!!\n");
    struct HdfEthMacChipDriver *macChipDriver = (struct HdfEthMacChipDriver *)OsalMemCalloc(
        sizeof(struct HdfEthMacChipDriver));

    if (macChipDriver == NULL) {
        HDF_LOGE("%s fail: OsalMemCalloc fail!", __func__);
        return NULL;
    }
    macChipDriver->ethMacOps = &g_macOps;
    return macChipDriver;
}

void ReleaseNxpMacDriver(struct HdfEthMacChipDriver *chipDriver)
{
    if (chipDriver == NULL) {
        HDF_LOGE("%s fail: chipDriver == NULL!", __func__);
        return;
    }
    OsalMemFree(chipDriver);
}

struct HdfEthMacChipDriver *GetNxpEthMacChipDriver(const struct NetDevice *netDev)
{
    struct HdfEthNetDeviceData *data = GetEthNetDeviceData(netDev);
    if (data != NULL) {
        return data->macChipDriver;
    }
    return NULL;
}

void EthNxpRandomAddr(uint8_t *addr, int32_t len)
{
    nxp_fec_get_mac(addr);

    return;
}

