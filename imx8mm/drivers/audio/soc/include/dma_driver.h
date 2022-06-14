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

#ifndef DMA_DRIVER_H
#define DMA_DRIVER_H

int32_t DMAInitTxBuff(struct PlatformData *platformData);
int32_t DMAInitRxBuff(struct PlatformData *platformData);
int32_t DMAEnableTx(const struct PlatformData *platformData);
int32_t DMAEnableRx(const struct PlatformData *platformData);
int32_t DMADisableTx(struct PlatformData *platformData);
int32_t DMADisableRx(struct PlatformData *platformData);
int32_t DMAPauseTx(struct PlatformData *platformData);
int32_t DMAPauseRx(struct PlatformData *platformData);
int32_t DmaInit(struct PlatformData *platformData);
int32_t DMADeinitTxBuff(struct PlatformData *platformData);
int32_t DMADeinitRxBuff(struct PlatformData *platformData);

#endif
