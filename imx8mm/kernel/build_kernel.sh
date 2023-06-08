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

#$1 - kernel build script work dir
#$2 - kernel build script stage dir
#$3 - GN target output dir

echo build_kernel
pushd ${1}
./kernel_module_build.sh ${2} ${4} ${5} ${6} ${7} ${8} ${9} ${10}
mkdir -p ${3}
rm -rf ${3}/../../../kernel.timestamp
if [ -d ${3}/kernel ];then
    echo "${3}/kernel existed!"
    cd ${3}/kernel
    rm -rf *
    cd -
else
    mkdir -p ${3}/kernel
fi

cp ${2}/kernel/src_tmp/${8}/arch/arm64/boot/Image ${3}/kernel/Image

mkdir -p ${3}/kernel/dtb
cp ${2}/kernel/src_tmp/${8}/arch/arm64/boot/dts/myir/*.dtb ${3}/kernel/dtb

cp -rf ./mkbootimg.sh ${3}/kernel
popd
