# Copyright© 2021–2022 Beijing OSWare Technology Co., Ltd
# This file contains confidential and proprietary information of
# OSWare Technology Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ETHERNET_CONFIG_PATH := $(shell pwd)
$(warning ETHERNET_CONFIG_PATH=$(ETHERNET_CONFIG_PATH))

HDF_FRAMEWORKS_INC := \
    -I./ \
    -Idrivers/hdf/framework/ability/sbuf/include \
    -Idrivers/hdf/framework/core/common/include/host \
    -Idrivers/hdf/framework/core/host/include \
    -Idrivers/hdf/framework/core/manager/include \
    -Idrivers/hdf/framework/core/shared/include \
    -Idrivers/hdf/framework/include \
    -Idrivers/hdf/framework/include/config \
    -Idrivers/hdf/framework/include/core \
    -Idrivers/hdf/framework/include/platform \
    -Idrivers/hdf/framework/include/utils \
    -Idrivers/hdf/framework/support/platform/include \
    -Idrivers/hdf/framework/support/platform/include/platform \
    -Idrivers/hdf/framework/utils/include \
    -Idrivers/hdf/khdf/osal/include \
    -Idrivers/hdf/khdf/config/include \
    -Iinclude/hdf \
    -Iinclude/hdf/osal \
    -Iinclude/hdf/utils \
    -Idrivers/hdf/framework/include/ethernet\
    -Idrivers/hdf/framework/include/net\
    -Idrivers/hdf/framework/model/network/common/netdevice\

HDF_ETHERNET_FRAMEWORKS_INC := \
    -Idrivers/hdf/framework/model/network/ethernet/include

SECURE_LIB_INC := \
    -I./../../../../../third_party/bounds_checking_function/include

HDF_ETHERNET_ADAPTER_INC := \
    -I./../../../../../device/board/osware/imx8mm/drivers/ethernet \
    -Idrivers/hdf/khdf/network/include \
    -Idrivers/net/ethernet/freescale/

HDF_ETHERNET_FLAGS +=-D_PRE_OS_VERSION_LINUX=1
HDF_ETHERNET_FLAGS +=-D_PRE_OS_VERSION_LITEOS=2
HDF_ETHERNET_FLAGS +=-D_PRE_OS_VERSION=_PRE_OS_VERSION_LINUX
