# OSware MILOS_Standard0 iMX 8m mini

## 介绍

基于 NXP i.MX8M Mini处理器，1.8G Hz主频。接⼝外设丰富：LVDS显⽰、MIPI-DSI信号引出、 MIPI-CSI摄像头接⼝、⽀持⾳频输⼊输出、千兆⽹、多路USB、多串⼝、蓝⽛模块等多种通信接⼝；提供OpenHarmony系统。⼴泛应⽤于⾼性能仪器仪表（⼯业及医疗）、⼯业控制及⼈机互动装置、智能交通、智慧消防、智慧楼宇等领域。

### 开发板外观图片

OSware MILOS_Standard0开发板外观图如图1所示：

**![MILOS_Standard0产品图01](./common/figures/MILOS_Standard_0.png)**

**一、开发板详情**

**1、OSware MILOS_Standard0开发板正面外观图**

**![MILOS_Standard0接口说明_02](./common/figures/MILOS_Standard_0_Front.png)**

2、**OSware MILOS_Standard0开发板反面外观图**

**![MILOS_Standard0接口说明_03](./common/figures/MILOS_Standard_0_Back.png)**

## 开发板规格

NXP iMX 8m mini采用四核64位Cortex-A53
处理器，主频高达1.8GHz；具有低功耗高性能的特点，可广泛应用于嵌入式人工智能领域。

**1、OSware MILOS_Standard0开发板规格图**

**![MILOS_Standard0接口说明_04](./common/figures/MILOS_Standard_0_Spec.png)**
|  组件名	|  能力介绍|
|  ----  | ----  |
|  Main CPU	|  1x,2x or 4x Cortex-A53 @ 1.8GHz, 512kB L2 |
|  Micro-controller	|  Cortex-M4 400MHz|
|  DDR	|  x16/x32 LPDDR4/DDR4/DDR3L|
|  GPU	|  GC NanoUltra 3D (1 shader) + GC320 2OpenGL ES 2.0|
|  Display Features	|  LCDIF|
|  Display Interfaces	|  1x MIPI-DSI|
|  Video Decode  	|  1080p60 HEVC H.265, VP8, H.264,VP9|
|  Video Encode	|  1080p60 H.264 VP8|
|  Audio Interface	|  5x SAI (12Tx + 16Rx external I2S lanes)Each lane up to 24.576MHz BCLK (32-bit, 2-ch 384KHz, up to 32-ch TDM); 4Tx + 4Rx support 49.152MHz BCLK for 768KHz|
|  Digital Mic Input	|  8ch PDM DMIC input|
|  Camera Interface	|  1x MIPI-CSI (4-lanes each)|
|  USB	|  2x USB2.0|
|  PCIe	|  1x PCIe 2.0|
|  Ethernet	|  1x GbE|
|  SDIO/eMMC	|  3x SDIO/eMMC|
|  Process	Samsung	|  14LPC FinFET|
|  Packages	|  14x14mm, 0.5p|
|  I2C	| 4 |
|  Temperature	|  -40°C to 105°C (Tj)|

### 开发板功能

**·** 支持OpenHarmony所有原生应用，比如Camera，Audio等

**·** 支持OpenHarmony硬件加速显示

· 支持OpenHarmony 软总线等功能

## 关键特性
|  组件名	|  能力介绍|
|  ----  | ----  |
|  WLAN服务	|  提供WLAN服务能力。包括：station和hotspot模式的连接、断开、状态查询等。|
|  BLUETOOTH 服务	|  提供蓝牙 BT BLE 等功能。|
|  模组外设控制	|  提供操作外设的能力。包括：I2C、UART、SPI、SDIO、GPIO、PWM、FLASH等。|
|  基础加解密	|  提供密钥管理、加解密等能力。|
|  系统服务管理	|  系统服务管理基于面向服务的架构，提供了OpenHarmony统一化的系统服务开发框架。|
|  启动引导	|  提供系统服务的启动入口标识。在系统服务管理启动时，调用boostrap标识的入口函数，并启动系统服务。|
|  基础库  	|  提供公共基础库能力。包括：文件操作、KV存储管理等。|
|  XTS	|  提供OpenHarmony生态认证测试套件的集合能力。|
|  HDF	|  提供OpenHarmony硬件配置驱动的能力。|
|  Kconfig	|  提供内核配置能力。|

## 引脚定义
**![MILOS_Standard0接口说明_04](./common/figures/MILOS_Standard_0_Arch_Data.png)**

## 搭建开发环境

### 系统要求

1. Windows 7操作系统及以上版本，用于固件烧录;
2. Ubuntu 18.04 LTS版本, 用于代码编译。

### 工具要求

安装命令如下：

    sudo apt-get update && sudo apt-get install binutils git git-lfs gnupg flex
    bison gperf build-essential zip curl zlib1g-dev gcc-multilib g++-multilib
    libc6-dev-i386 lib32ncurses5-dev x11proto-core-dev libx11-dev lib32z1-dev ccache
    libgl1-mesa-dev libxml2-utils xsltproc unzip m4 bc gnutls-bin python3.8 python3-pip ruby

### 搭建过程

#### 开发板配件安装

**1、摄像头模组安装如下图：**

![MILOS_Standard0配件安装_01](./common/figures/MILOS_Standard_0_Camera.png)

**2、LVDS 屏幕安装如下图：**

![MILOS_Standard0配件安装_02](./common/figures/MILOS_Standard_0_LVDS.png)

## 编译调试

### 编译

#### 前提条件

1）注册码云gitee账号。

2）注册码云SSH公钥，请参考[码云帮助中心](https://gitee.com/help/articles/4191)。

3）安装[git客户端](http://git-scm.com/book/zh/v2/%E8%B5%B7%E6%AD%A5-%E5%AE%89%E8%A3%85-Git)和[git-lfs](https://gitee.com/vcs-all-in-one/git-lfs?_from=gitee_search#downloading)并配置用户信息。

    git config --global user.name "yourname"

    git config --global user.email "your-email-address"

    git config --global credential.helper store

4）安装码云repo工具，可以执行如下命令。

    curl -s https://gitee.com/oschina/repo/raw/fork_flow/repo-py3 \>
    /usr/local/bin/repo \#如果没有权限，可下载至其他目录，并将其配置到环境变量中

    chmod a+x /usr/local/bin/repo

    pip3 install -i https://repo.huaweicloud.com/repository/pypi/simple requests

#### 获取源码操作步骤

1） 通过repo + ssh 下载（需注册公钥，请参考码云帮助中心）。

    repo init -u git@gitee.com:openharmony/manifest.git -b master --no-repo-verify

    repo sync -c

    repo forall -c 'git lfs pull'

2） 通过repo + https 下载。

    repo init -u https://gitee.com/openharmony/manifest.git -b master
    \--no-repo-verify

    repo sync -c

    repo forall -c 'git lfs pull'

#### 执行prebuilts

在源码根目录下执行脚本，安装编译器及二进制工具。

    bash build/prebuilts_download.sh

下载的prebuilts二进制默认存放在与OpenHarmony同目录下的OpenHarmony_2.0_canary_prebuilts下。

#### 执行编译

在Linux环境进行如下操作:

1） 进入源码根目录，执行如下命令进行版本编译。

    ./build.sh --product-name imx8mm –ccache

2） 检查编译结果。编译完成后，log中显示如下：

    post_process

    =====build imx8mm successful.

    2022-06-1 09:22:28

编译所生成的文件都归档在out/imx8mm/ohos-arm-release/目录下，结果镜像输出在
out/imx8mm/ohos-arm-release/packages/phone/images/ 目录下。

3） 编译源码完成，请进行镜像烧录。

### 烧录

烧写工具下载及使用。
[参考](https://gitee.com/osware_admin_admin/burning-tools-and-guidelines)