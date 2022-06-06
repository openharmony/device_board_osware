#!/bin/bash
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

set -e

#$1 - kernel build script work dir
#$2 - kernel build script stage dir
#$3 - GN target output dir

echo build_kernel
pushd ${1}
./kernel_module_build.sh ${2} ${4} ${5} ${6} ${7} ${8}
mkdir -p ${3}
rm -rf ${3}/../../../kernel.timestamp
cp ${2}/kernel/src_tmp/${8}/arch/arm64/boot/Image ${3}/Image
if [ -d ${3}/dtb ];then
    echo "${3}/dtb existed!"
else
    mkdir -p ${3}/dtb
fi
cp ${2}/kernel/src_tmp/${8}/arch/arm64/boot/dts/myir/*.dtb ${3}/dtb

if [ -d ${3}/../../phone/system/etc/imx_sdma ];then
    echo "${3}/../../phone/system/etc/imx_sdma existed!"
else
    mkdir -p ${3}/../../phone/system/etc/imx_sdma
fi

if [ -d ${3}/../../phone/system/etc/imx_camera ];then
    echo "${3}/../../phone/system/etc/imx_camera existed!"
else
    mkdir -p ${3}/../../phone/system/etc/imx_camera
fi
cp ${2}/kernel/src_tmp/${8}/TEST/lib/modules/5.10.72/kernel/drivers/dma/imx-sdma.ko ${3}/../../phone/system/etc/imx_sdma
cp ${2}/kernel/src_tmp/${8}/TEST/lib/modules/5.10.72/kernel/drivers/media/platform/mxc/capture/imx-sdma.ko ${3}/../../phone/system/etc/imx_camera
popd
