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

MODULE_NAME := ap6212

VENDOR_WIFI_ROOT = .

WIFI_PATH := $(shell pwd)
$(warning WIFI_PATH=$(WIFI_PATH))

include $(WIFI_PATH)/../../../../../device/board/osware/imx8mm/drivers/wifi/ap6212/wifi_config.mk

$(warning HDF_FRAMEWORKS_INC=$(HDF_FRAMEWORKS_INC))
$(warning HDF_WIFI_FRAMEWORKS_INC=$(HDF_WIFI_FRAMEWORKS_INC))
$(warning SECURE_LIB_INC=$(SECURE_LIB_INC))
$(warning HDF_WIFI_ADAPTER_INC=$(HDF_WIFI_ADAPTER_INC))

obj-y += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(VENDOR_WIFI_ROOT)/hdfadapter/hdf_driver_register.o \
		 $(VENDOR_WIFI_ROOT)/hdfadapter/hdfinit_ap6212.o \
         $(VENDOR_WIFI_ROOT)/hdfadapter/net_adpater.o \
         $(VENDOR_WIFI_ROOT)/hdfadapter/hdf_mac80211.o

ccflags-y += \
    $(HDF_FRAMEWORKS_INC) \
    $(HDF_WIFI_FRAMEWORKS_INC) \
    $(SECURE_LIB_INC) \
	$(HDF_WIFI_ADAPTER_INC)
