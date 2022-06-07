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

MODULE_NAME := hdf_ethernet_model

ETHERNET_PATH := $(shell pwd)
$(warning ETHERNET_PATH=$(ETHERNET_PATH))

include $(ETHERNET_PATH)/../../../../../device/board/osware/imx8mm/drivers/ethernet/ethernet_config.mk

ETHERNET_FW_PATH := ../../../../../../out/KERNEL_OBJ/kernel/src_tmp/linux-5.10/drivers/hdf/framework/model/network/ethernet/src
ETHERNET_AP_PATH := ../../../../../../out/KERNEL_OBJ/kernel/src_tmp/linux-5.10/drivers/hdf/khdf/network/src
ETHERNET_COMMON_PATH := ../../../../../../out/KERNEL_OBJ/kernel/src_tmp/linux-5.10/drivers/hdf/framework/model/network/common/netdevice
# for compile 
obj-y += $(ETHERNET_FW_PATH)/eth_chip_driver.o \
	     $(ETHERNET_FW_PATH)/eth_device.o \
		 $(ETHERNET_FW_PATH)/hdf_eth_core.o

obj-y += $(MODULE_NAME).o
$(MODULE_NAME)-objs := hdf_driver_register.o \
                       net_adapter.o \
                       nxpeth_mac.o \
                       nxpeth_phy.o
ccflags-y += \
    $(HDF_FRAMEWORKS_INC) \
    $(HDF_ETHERNET_FRAMEWORKS_INC) \
    $(SECURE_LIB_INC) \
    $(HDF_ETHERNET_ADAPTER_INC) \
    $(HDF_ETHERNET_FLAGS)

