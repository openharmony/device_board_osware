# Copyright (C) 2021–2022 Beijing OSWare Technology Co., Ltd
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

PRODUCT_NAME=$(TARGET_PRODUCT)
OHOS_BUILD_HOME := $(realpath $(shell pwd)/../../../../../)
KERNEL_SRC_TMP_PATH := $(OUT_DIR)/kernel/${KERNEL_VERSION}
KERNEL_OBJ_TMP_PATH := $(OUT_DIR)/kernel/OBJ/${KERNEL_VERSION}
ifeq ($(BUILD_TYPE), standard)
    BOOT_IMAGE_PATH = $(OHOS_BUILD_HOME)/device/board/hisilicon/hispark_taurus/uboot/prebuilts
    KERNEL_SRC_TMP_PATH := $(OUT_DIR)/kernel/src_tmp/${KERNEL_VERSION}
endif

KERNEL_SRC_PATH := $(OHOS_BUILD_HOME)/kernel/linux/${KERNEL_VERSION}
KERNEL_PATCH_PATH := $(OHOS_BUILD_HOME)/kernel/linux/patches/${KERNEL_VERSION}
KERNEL_CONFIG_PATH := $(OHOS_BUILD_HOME)/kernel/linux/config/${KERNEL_VERSION}
PREBUILTS_GCC_DIR := $(OHOS_BUILD_HOME)/prebuilts/gcc
CLANG_HOST_TOOLCHAIN := $(OHOS_BUILD_HOME)/prebuilts/clang/ohos/linux-x86_64/llvm/bin
KERNEL_HOSTCC := $(CLANG_HOST_TOOLCHAIN)/clang
KERNEL_PREBUILT_MAKE := make
CLANG_CC := $(CLANG_HOST_TOOLCHAIN)/clang

HARMONY_CONFIG_PATH := $(OHOS_BUILD_HOME)/kernel/linux/config/$(KERNEL_VERSION)
DEVICE_CONFIG_PATH  := $(OHOS_BUILD_HOME)/kernel/linux/config/$(KERNEL_VERSION)/$(DEVICE_NAME)
DEFCONFIG_BASE_FILE := $(HARMONY_CONFIG_PATH)/base_defconfig
DEFCONFIG_TYPE_FILE := $(HARMONY_CONFIG_PATH)/type/$(BUILD_TYPE)_defconfig
DEFCONFIG_FORM_FILE := $(HARMONY_CONFIG_PATH)/form/$(KERNEL_FORM)_defconfig
DEFCONFIG_ARCH_FILE := $(DEVICE_CONFIG_PATH)/arch/$(KERNEL_ARCH)_defconfig
DEFCONFIG_PROC_FILE := $(DEVICE_CONFIG_PATH)/product/$(KERNEL_PROD)_defconfig

ifneq ($(shell test -e $DEFCONFIG_FORM_FILE), 0)
    DEFCONFIG_FORM_FILE :=
    $(warning no form config file $(DEFCONFIG_FORM_FILE))
endif

ifneq ($(shell test -e $DEFCONFIG_PROC_FILE), 0)
    DEFCONFIG_PROC_FILE :=
    $(warning no product config file $(DEFCONFIG_PROC_FILE))
endif

ifeq ($(KERNEL_ARCH), arm)
    KERNEL_TARGET_TOOLCHAIN := $(PREBUILTS_GCC_DIR)/linux-x86/arm/gcc-linaro-7.5.0-arm-linux-gnueabi/bin
    KERNEL_TARGET_TOOLCHAIN_PREFIX := $(KERNEL_TARGET_TOOLCHAIN)/arm-linux-gnueabi-
else ifeq ($(KERNEL_ARCH), arm64)
    KERNEL_TARGET_TOOLCHAIN := $(PREBUILTS_GCC_DIR)/linux-x86/aarch64/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin
    KERNEL_TARGET_TOOLCHAIN_PREFIX := $(KERNEL_TARGET_TOOLCHAIN)/aarch64-linux-gnu-
endif

KERNEL_CROSS_COMPILE :=
KERNEL_CROSS_COMPILE += CC="$(CLANG_CC)"
KERNEL_CROSS_COMPILE += CROSS_COMPILE="$(KERNEL_TARGET_TOOLCHAIN_PREFIX)"

KERNEL_MAKE := \
    PATH="$(BOOT_IMAGE_PATH):$$PATH" \
    $(KERNEL_PREBUILT_MAKE)

DEVICE_PATCH_DIR := $(OHOS_BUILD_HOME)/kernel/linux/patches/${KERNEL_VERSION}/$(DEVICE_NAME)_patch
DEVICE_PATCH_FILE := $(DEVICE_PATCH_DIR)/$(DEVICE_NAME).patch
HDF_PATCH_FILE := $(DEVICE_PATCH_DIR)/hdf.patch
SMALL_PATCH_FILE := $(DEVICE_PATCH_DIR)/$(DEVICE_NAME)_$(BUILD_TYPE).patch
KERNEL_IMAGE_FILE := $(KERNEL_SRC_TMP_PATH)/arch/$(KERNEL_ARCH)/boot/$(KERNEL_IMAGE)
DEFCONFIG_FILE := myd_imx8mm_defconfig

$(KERNEL_IMAGE_FILE):
	$(hide) echo "build kernel..."
	$(hide) rm -rf $(KERNEL_SRC_TMP_PATH);mkdir -p $(KERNEL_SRC_TMP_PATH);cp -arfL $(KERNEL_SRC_PATH)/* $(KERNEL_SRC_TMP_PATH)/
	$(hide) $(OHOS_BUILD_HOME)/device/board/osware/imx8mm/kernel/patch_imx.sh $(KERNEL_PATCH_PATH) $(KERNEL_SRC_TMP_PATH)
	$(hide) $(OHOS_BUILD_HOME)/drivers/hdf_core/adapter/khdf/linux/patch_hdf.sh $(OHOS_BUILD_HOME) $(KERNEL_SRC_TMP_PATH) $(KERNEL_PATCH_PATH) ${DEVICE_NAME}
	sh $(OHOS_BUILD_HOME)/kernel/linux/$(KERNEL_VERSION)/scripts/kconfig/merge_config.sh -O $(KERNEL_SRC_TMP_PATH)/arch/$(KERNEL_ARCH)/configs/ -m $(DEFCONFIG_TYPE_FILE) $(DEFCONFIG_FORM_FILE) $(DEFCONFIG_ARCH_FILE) $(DEFCONFIG_PROC_FILE) $(DEFCONFIG_BASE_FILE)
	mv $(KERNEL_SRC_TMP_PATH)/arch/$(KERNEL_ARCH)/configs/.config $(KERNEL_SRC_TMP_PATH)/arch/$(KERNEL_ARCH)/configs/$(DEFCONFIG_FILE)
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) distclean
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) $(DEFCONFIG_FILE)
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) dtbs Image modules -j24
	$(hide) $(KERNEL_MAKE) -C $(KERNEL_SRC_TMP_PATH) ARCH=$(KERNEL_ARCH) $(KERNEL_CROSS_COMPILE) modules_install INSTALL_MOD_PATH=./TEST
.PHONY: build-kernel
build-kernel: $(KERNEL_IMAGE_FILE)
