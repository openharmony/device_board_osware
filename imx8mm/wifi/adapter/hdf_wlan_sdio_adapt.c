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

#include "hdf_wlan_sdio.h"
#include "hdf_wlan_config.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_wlan_chipdriver_manager.h"
int32_t HdfWlanConfigSDIO(uint8_t busId)
{
    struct HdfConfigWlanRoot *config = HdfWlanGetModuleConfigRoot();
    if (config == NULL || config->wlanConfig.hostChipName == NULL) {
        HDF_LOGE("%s: No config or chip name is NULL!", __func__);
        return HDF_FAILURE;
    }
    if (strcmp("ap6212", config->wlanConfig.hostChipName) == 0) {
        HDF_LOGE("%s: platform chip AP6212!!", __func__);
        return HDF_SUCCESS;
    }
    HDF_LOGE("%s: platform chip not supported!", __func__);
    return HDF_FAILURE;
}
