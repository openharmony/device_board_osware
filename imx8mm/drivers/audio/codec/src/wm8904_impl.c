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

#include "gpio_if.h"
#include "i2c_if.h"
#include "audio_device_log.h"
#include "imx8mm_common.h"
#include "wm8904_log.h"
#include "wm8904.h"
#include "audio_codec_base.h"
#include "audio_core.h"
#define WM8904_I2C_BUS_NUM (1)
#define WM8904_I2C_REG_DATA_LEN  (2)

#define DATA_LEN_1  (1)
#define DATA_LEN_2  (2)

DevHandle WM8904I2cOpen(void)
{
    DevHandle i2cHandle;
    i2cHandle = I2cOpen(WM8904_I2C_BUS_NUM);
    if (i2cHandle == NULL) {
        WM8904_CODEC_LOG_ERR("open i2cBus:%u fail! i2cHandle:%p", WM8904_I2C_BUS_NUM, i2cHandle);
        return NULL;
    }
    return i2cHandle;
}

void WM8904I2cClose(DevHandle handle)
{
    I2cClose(handle);
}

#define gwm8904i2cDevAddr (0x1a)
#define WM8904RegAddrLen  (1)       // unit byte
struct I2cTransferParam g_transferParam;


/************************************************************************************************
    __WM8904I2cRead: Read any Length byte from i2 device reg
    struct DevHandle i2cHandle[in]  : i2c bus Handle
    unsigned int regaddr [in]       : max supports 16 bits i2c device reg address
    unsigned int regLen  [in]       : reg address length(8 or 16 bits)
    uint8_t  *regdata    [out]      : buffer
    unsigned int dataLen [in]       : buffer length
    return : status
************************************************************************************************/
static int __WM8904I2cRead(DevHandle i2cHandle, uint16_t regaddr,
                           unsigned int regLen, uint8_t *regdata, unsigned int dataLen)
{
    int index = 0;
    unsigned char regBuf[4] = {0};
    struct I2cMsg msgs[2] = {0};

    if (regLen == 1) {
        regBuf[index++] = regaddr & 0xFF;
    } else {
         // i2c first write reg high 8 bits
        regBuf[index++] = (regaddr >> 8) & 0xFF;
        regBuf[index++] = regaddr & 0xFF;
    }

    // msgs[0] write regaddr to adapter
    msgs[0].addr = gwm8904i2cDevAddr;
    msgs[0].flags = 0;
    msgs[0].len = regLen;
    msgs[0].buf = regBuf;

    // msgs[1] read dataLen byte regvalue to *regdata
    msgs[1].addr = gwm8904i2cDevAddr;
    msgs[1].flags = I2C_FLAG_READ;
    msgs[1].len = dataLen;
    msgs[1].buf = regdata;

    if (I2cTransfer(i2cHandle, msgs, 2) != 2) {
        WM8904_CODEC_LOG_ERR("i2c I2cTransfer read msg err");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int WM8904RegRead(DevHandle i2cHandle, unsigned int reg, unsigned int *val, unsigned int dataLen)
{
    int32_t ret;
    unsigned char regBuf[2] = {0};
    WM8904_CODEC_LOG_DEBUG("entry");
    if (val == NULL) {
        WM8904_CODEC_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    if (dataLen == DATA_LEN_2) {
        ret = __WM8904I2cRead(i2cHandle, reg, WM8904RegAddrLen, regBuf, dataLen);
        if (ret != HDF_SUCCESS) {
            WM8904_CODEC_LOG_ERR("WM8904RegSeqRead fail.");
            return HDF_FAILURE;
        }
        *val = 0;
        *val = (regBuf[0] << 8) | regBuf[1];
    } else if (dataLen == DATA_LEN_1) {
        ret = __WM8904I2cRead(i2cHandle, reg, WM8904RegAddrLen, regBuf, dataLen);
        if (ret != HDF_SUCCESS) {
            WM8904_CODEC_LOG_ERR("WM8904RegSeqRead fail.");
            return HDF_FAILURE;
        }
        *val = regBuf[0];
    } else {
        WM8904_CODEC_LOG_ERR("input para is NULL. dataLen > 2");
    }
    return HDF_SUCCESS;
}

/*********************************************************************************************
    __WM8904I2cWrite: Write any Length byte from i2 device reg
    struct DevHandle i2cHandle[in]  : i2c bus Handle
    unsigned int regaddr [in]       : max supports 16 bits i2c device reg address
    unsigned int regLen  [in]       : reg address length(8 or 16 bits)
    uint8_t  *regdata    [in]       : buffer
    unsigned int dataLen [in]       : buffer length
    return : status
*********************************************************************************************/
static int __WM8904I2cWrite(DevHandle i2cHandle, uint16_t regaddr,
                            unsigned int regLen, uint8_t *regdata, unsigned int dataLen)
{
    int index = 0;
    unsigned char regBuf[4] = {0};
    struct I2cMsg msgs[1] = {0};

    if (regLen == 1) {
        regBuf[index++] = regaddr & 0xFF;
    } else {
         // i2c first write reg high 8 bits
        regBuf[index++] = (regaddr >> 8) & 0xFF;
        regBuf[index++] = regaddr & 0xFF;
    }

    for (int i = 0; i < dataLen; i++) {
        regBuf[index++] = regdata[i];
    }

    // write regaddr to adapter
    msgs[0].addr = gwm8904i2cDevAddr;
    msgs[0].flags = 0;
    msgs[0].len = regLen + dataLen;
    msgs[0].buf = regBuf;

    if (I2cTransfer(i2cHandle, msgs, 1) != 1) {
        WM8904_CODEC_LOG_ERR("i2c I2cTransfer write msg err");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int WM8904RegWrite(DevHandle i2cHandle, unsigned int reg, unsigned int val, unsigned int dataLen)
{
    int32_t ret;
    unsigned char regBuf[2] = {0};

    WM8904_CODEC_LOG_DEBUG("entry");

    if (dataLen == DATA_LEN_2) {
        regBuf[0] = (val >> 8) & 0xFF;
        regBuf[1] =  val & 0xFF;

        ret = __WM8904I2cWrite(i2cHandle, reg, WM8904RegAddrLen, regBuf, dataLen);
        if (ret != HDF_SUCCESS) {
            WM8904_CODEC_LOG_ERR("datalen=2 fail.");
            return HDF_FAILURE;
        }
    } else if (dataLen == DATA_LEN_1) {
        regBuf[0] = val;
        ret = __WM8904I2cWrite(i2cHandle, reg, WM8904RegAddrLen, regBuf, dataLen);
        if (ret != HDF_SUCCESS) {
            WM8904_CODEC_LOG_DEBUG("datalen=1 fail.");
            return HDF_FAILURE;
        }
    } else {
        WM8904_CODEC_LOG_DEBUG("input para is NULL. dataLen > 2");
    }

    return HDF_SUCCESS;
}

int WM8904RegUpdateBits(DevHandle i2cHandle, unsigned int reg, unsigned int mask,
                        unsigned int val, unsigned int dataLen)
{
    unsigned int ret = 0, tmp = 0, orig = 0;

    ret = WM8904RegRead(i2cHandle, reg, &orig, dataLen);
    if (ret != HDF_SUCCESS)
        return ret;

    tmp = orig & ~mask;
    tmp |= val & mask;

    if (tmp != orig) {
        ret = WM8904RegWrite(i2cHandle, reg, tmp, dataLen);
    }
    return ret;
}

#define CODEC_REG_ADDR_DUMMY (0x00)

static const struct AudioMixerControl g_audioRegParams = {
    .reg = CODEC_REG_ADDR_DUMMY,
    .rreg = CODEC_REG_ADDR_DUMMY,
    .shift = 0,
    .rshift = 0,
    .min = 0x0,
    .max = 0xFF,
    .mask = 0x1,
    .invert = 1,
};

static const struct AudioKcontrol g_audioControls[] = {
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_ACODEC,
        .name = "External Codec Enable",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams,
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_ACODEC,
        .name = "Internally Codec Enable",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams,
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_AIAO,
        .name = "Render Channel Mode",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams,
    }, {
        .iface = AUDIODRV_CTL_ELEM_IFACE_AIAO,
        .name = "Captrue Channel Mode",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams,
    },
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_PGA,
        .name = "LPGA MIC Switch",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams,
    },
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_PGA,
        .name = "RPGA MIC Switch",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams,
    },
    /* for sapm test */
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_DAC,
        .name = "Dacl enable",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams,
    },
    {
        .iface = AUDIODRV_CTL_ELEM_IFACE_DAC,
        .name = "Dacr enable",
        .Info = AudioInfoCtrlOps,
        .Get = AudioCodecGetCtrlOps,
        .Set = AudioCodecSetCtrlOps,
        .privateValue = (unsigned long)&g_audioRegParams,
    },
};

int32_t Wm8904DevInit(struct AudioCard *audioCard, const struct CodecDevice *device)
{
    int32_t ret;

    if ((audioCard == NULL) || (device == NULL)) {
        WM8904_CODEC_LOG_ERR("input para is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    g_transferParam.i2cDevAddr = gwm8904i2cDevAddr;
    g_transferParam.i2cBusNumber = WM8904_I2C_BUS_NUM;
    g_transferParam.i2cRegDataLen = WM8904_I2C_REG_DATA_LEN;
    device->devData->privateParam = &g_transferParam;
    ret = AudioAddControls(audioCard, device->devData->controls, device->devData->numControls);
    if (ret != HDF_SUCCESS) {
        WM8904_CODEC_LOG_ERR("AudioAddControls failed.");
        return HDF_FAILURE;
    }

    if (AudioAddControls(audioCard, g_audioControls, HDF_ARRAY_SIZE(g_audioControls)) != HDF_SUCCESS) {
        WM8904_CODEC_LOG_DEBUG("add controls fail.");
    }
    WM8904_CODEC_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t Wm8904DaiStart(const struct AudioCard *card, const struct DaiDevice *device)
{
    (void)card;
    (void)device;
    return HDF_SUCCESS;
}

int32_t Wm8904DaiDevInit(struct AudioCard *card, const struct DaiDevice *device)
{
    if (device == NULL || device->devDaiName == NULL) {
        WM8904_CODEC_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }
    (void)card;
    return HDF_SUCCESS;
}

