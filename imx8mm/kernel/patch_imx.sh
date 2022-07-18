#!/bin/bash
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

set -e

OHOS_SOURCE_ROOT=$1
KERNEL_BUILD_ROOT=$2

function patch_imx()
{
    cd $KERNEL_BUILD_ROOT
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0001_linux_arch.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0002_linux_block.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0003_linux_crypto.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0004_linux_fs.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0005_linux_include.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0006_linux_init.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0007_linux_kernel.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0008_linux_net.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0009_linux_sound.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/0010_linux_tools.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0011_linux_drivers_acpi.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0012_linux_drivers_ata.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0013_linux_drivers_base.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0014_linux_drivers_bus.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0015_linux_drivers_clk.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0016_linux_drivers_crypto.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0017_linux_drivers_dma_dmabuf.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0018_linux_drivers_firmware.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0019_linux_drivers_gpio.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0020_linux_drivers_gpu.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0021_linux_drivers_hwmon.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0022_linux_drivers_i2c.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0023_linux_drivers_iio.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0024_linux_drivers_input.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0025_linux_drivers_irqchip.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0026_linux_drivers_mailbox.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0027_linux_drivers_media.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0028_linux_drivers_memory.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0029_linux_drivers_mfd.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0030_linux_drivers_pci_misc_nvmem_of_mtd_mmc.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0031_linux_drivers_perf_phy_pinctrl_ptp_pwm.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0032_linux_drivers_regulator_remoteproc_reset_rpmsg_rtc.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0033_linux_drivers_soc_scsi_spi_tee_thermal.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0034_linux_drivers_usb_vfio_wdt_tty_uio.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0035_linux_drivers_video.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0036_linux_drivers_staging.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0037_linux_drivers_net.patch
    patch -p1 < $OHOS_SOURCE_ROOT/imx8mm_patch/patches/drivers/0038_linux_drivers_mxc.patch
    cd -
}

function patch_link()
{
    cd $KERNEL_BUILD_ROOT/scripts/dtc/include-prefixes
    # delete the dtc contents
    rm -rf arm64 dt-bindings

    # redo the dtc contents
    ln -s ../../../arch/arm64 .
    ln -s ../../../include/dt-bindings .
    cd -
}

function main()
{
    patch_imx
    patch_link
}

main
