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

#ifndef NXPETH_PHY_H
#define NXPETH_PHY_H

#include <linux/phy.h>
#include <linux/fec.h>
#include "eth_chip_driver.h"
#include "net_device_impl.h"
#include "fec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

int32_t InitNxpethDriver(struct EthDevice *ethDevice);
int32_t DeinitNxpethDriver(struct EthDevice *ethDevice);
bool NxpethHwInit(struct EthDevice *ethDevice);
int32_t NxpethInit(struct EthDevice *ethDevice);

struct FullNetDeviceImx8mmEthPriv {
    struct net_device *dev;
    struct NetDeviceImpl *impl;
    struct fec_enet_private *lfecp;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* NXPETH_PHY_H */
