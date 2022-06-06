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

#ifndef NXPETH_MAC_H
#define NXPETH_MAC_H

#include "eth_chip_driver.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct HdfEthMacChipDriver *BuildNxpMacDriver(void);
void ReleaseNxpMacDriver(struct HdfEthMacChipDriver *chipDriver);
struct HdfEthMacChipDriver *GetNxpEthMacChipDriver(const struct NetDevice *netDev);
void EthNxpRandomAddr(uint8_t *addr, int32_t len);
extern void nxp_fec_get_mac(unsigned char *mac_address);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* NXPETH_MAC_H */
