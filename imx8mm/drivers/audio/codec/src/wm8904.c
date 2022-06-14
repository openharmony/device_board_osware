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

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <sound/wm8904.h>
#include "gpio_if.h"
#include "i2c_if.h"
#include "audio_device_log.h"
#include "wm8904.h"
#include "wm8904_log.h"

#include "audio_codec_base.h"
#include "audio_core.h"

struct wm8904_priv *gpwm8904;
DevHandle g_wm8904_i2c_handle = NULL;

DevHandle WM8904I2cOpen(void);
void WM8904I2cClose(DevHandle handle);

int  WM8904RegRead(DevHandle i2cHandle, unsigned int reg, unsigned int *val , unsigned int dataLen);
int  WM8904RegWrite(DevHandle i2cHandle, unsigned int reg,  unsigned int val, unsigned int dataLen);
int  WM8904RegUpdateBits(DevHandle i2cHandle, unsigned int reg,
                         unsigned int mask, unsigned int val, unsigned int dataLen);
int32_t Wm8904DaiStart(const struct AudioCard *card, const struct DaiDevice *device);
int32_t Wm8904DaiHwParamsSet(const struct AudioCard *card, const struct AudioPcmHwParams *param);
int32_t Wm8904DaiDevInit(struct AudioCard *card, const struct DaiDevice *device);
int32_t Wm8904DevInit(struct AudioCard *audioCard, const struct CodecDevice *device);

#define AUDIO_DRV_PCMIOCTL_CAPUTRE_START  7
#define AUDIO_DRV_PCM_IOCTL_RENDER_START  5
#define AUDIO_DRV_PCM_IOCTL_RENDER_STOP   6
#define AUDIO_DRV_PCM_IOCTL_CAPTURE_STOP  8
#define AUDIO_DRV_PCM_IOCTL_CAPTURE_PAUSE 10
#define AUDIO_DRV_PCM_IOCTL_RENDER_PAUSE  9
enum wm8904_type {
    WM8904,
    WM8912,
};

#define WM8904_NUM_DCS_CHANNELS 4
#define WM8904_NUM_SUPPLIES 5
static const char *wm8904_supply_names[WM8904_NUM_SUPPLIES] = {
    "DCVDD",
    "DBVDD",
    "AVDD",
    "CPVDD",
    "MICVDD",
};

/* codec private data */
struct wm8904_priv {
    struct regmap *regmap;
    struct clk *mclk;

    enum wm8904_type devtype;

    struct regulator_bulk_data supplies[WM8904_NUM_SUPPLIES];

    struct wm8904_pdata *pdata;

    int deemph;

    /* Platform provided DRC configuration */
    const char **drc_texts;
    int drc_cfg;
    struct soc_enum drc_enum;

    /* Platform provided ReTune mobile configuration */
    int num_retune_mobile_texts;
    const char **retune_mobile_texts;
    int retune_mobile_cfg;
    struct soc_enum retune_mobile_enum;

    /* FLL setup */
    int fll_src;
    int fll_fref;
    int fll_fout;

    /* Clocking configuration */
    unsigned int mclk_rate;
    int sysclk_src;
    unsigned int sysclk_rate;

    int tdm_width;
    int tdm_slots;
    int bclk;
    int fs;

    /* DC servo configuration - cached offset values */
    int dcs_state[WM8904_NUM_DCS_CHANNELS];
};

static const struct reg_default wm8904_reg_defaults[] = {
    { 4,   0x0018 },     /* R4   - Bias Control 0 */
    { 5,   0x0000 },     /* R5   - VMID Control 0 */
    { 6,   0x0000 },     /* R6   - Mic Bias Control 0 */
    { 7,   0x0000 },     /* R7   - Mic Bias Control 1 */
    { 8,   0x0001 },     /* R8   - Analogue DAC 0 */
    { 9,   0x9696 },     /* R9   - mic Filter Control */
    { 10,  0x0001 },     /* R10  - Analogue ADC 0 */
    { 12,  0x0000 },     /* R12  - Power Management 0 */
    { 14,  0x0000 },     /* R14  - Power Management 2 */
    { 15,  0x0000 },     /* R15  - Power Management 3 */
    { 18,  0x0000 },     /* R18  - Power Management 6 */
    { 20,  0x945E },     /* R20  - Clock Rates 0 */
    { 21,  0x0C05 },     /* R21  - Clock Rates 1 */
    { 22,  0x0006 },     /* R22  - Clock Rates 2 */
    { 24,  0x0050 },     /* R24  - Audio Interface 0 */
    { 25,  0x000A },     /* R25  - Audio Interface 1 */
    { 26,  0x00E4 },     /* R26  - Audio Interface 2 */
    { 27,  0x0040 },     /* R27  - Audio Interface 3 */
    { 30,  0x00C0 },     /* R30  - DAC Digital Volume Left */
    { 31,  0x00C0 },     /* R31  - DAC Digital Volume Right */
    { 32,  0x0000 },     /* R32  - DAC Digital 0 */
    { 33,  0x0008 },     /* R33  - DAC Digital 1 */
    { 36,  0x00C0 },     /* R36  - ADC Digital Volume Left */
    { 37,  0x00C0 },     /* R37  - ADC Digital Volume Right */
    { 38,  0x0010 },     /* R38  - ADC Digital 0 */
    { 39,  0x0000 },     /* R39  - Digital Microphone 0 */
    { 40,  0x01AF },     /* R40  - DRC 0 */
    { 41,  0x3248 },     /* R41  - DRC 1 */
    { 42,  0x0000 },     /* R42  - DRC 2 */
    { 43,  0x0000 },     /* R43  - DRC 3 */
    { 44,  0x0085 },     /* R44  - Analogue Left Input 0 */
    { 45,  0x0085 },     /* R45  - Analogue Right Input 0 */
    { 46,  0x0044 },     /* R46  - Analogue Left Input 1 */
    { 47,  0x0044 },     /* R47  - Analogue Right Input 1 */
    { 57,  0x002D },     /* R57  - Analogue OUT1 Left */
    { 58,  0x002D },     /* R58  - Analogue OUT1 Right */
    { 59,  0x0039 },     /* R59  - Analogue OUT2 Left */
    { 60,  0x0039 },     /* R60  - Analogue OUT2 Right */
    { 61,  0x0000 },     /* R61  - Analogue OUT12 ZC */
    { 67,  0x0000 },     /* R67  - DC Servo 0 */
    { 69,  0xAAAA },     /* R69  - DC Servo 2 */
    { 71,  0xAAAA },     /* R71  - DC Servo 4 */
    { 72,  0xAAAA },     /* R72  - DC Servo 5 */
    { 90,  0x0000 },     /* R90  - Analogue HP 0 */
    { 94,  0x0000 },     /* R94  - Analogue Lineout 0 */
    { 98,  0x0000 },     /* R98  - Charge Pump 0 */
    { 104, 0x0004 },     /* R104 - Class W 0 */
    { 108, 0x0000 },     /* R108 - Write Sequencer 0 */
    { 109, 0x0000 },     /* R109 - Write Sequencer 1 */
    { 110, 0x0000 },     /* R110 - Write Sequencer 2 */
    { 111, 0x0000 },     /* R111 - Write Sequencer 3 */
    { 112, 0x0000 },     /* R112 - Write Sequencer 4 */
    { 116, 0x0000 },     /* R116 - FLL Control 1 */
    { 117, 0x0007 },     /* R117 - FLL Control 2 */
    { 118, 0x0000 },     /* R118 - FLL Control 3 */
    { 119, 0x2EE0 },     /* R119 - FLL Control 4 */
    { 120, 0x0004 },     /* R120 - FLL Control 5 */
    { 121, 0x0014 },     /* R121 - GPIO Control 1 */
    { 122, 0x0010 },     /* R122 - GPIO Control 2 */
    { 123, 0x0010 },     /* R123 - GPIO Control 3 */
    { 124, 0x0000 },     /* R124 - GPIO Control 4 */
    { 126, 0x0000 },     /* R126 - Digital Pulls */
    { 128, 0xFFFF },     /* R128 - Interrupt Status Mask */
    { 129, 0x0000 },     /* R129 - Interrupt Polarity */
    { 130, 0x0000 },     /* R130 - Interrupt Debounce */
    { 134, 0x0000 },     /* R134 - EQ1 */
    { 135, 0x000C },     /* R135 - EQ2 */
    { 136, 0x000C },     /* R136 - EQ3 */
    { 137, 0x000C },     /* R137 - EQ4 */
    { 138, 0x000C },     /* R138 - EQ5 */
    { 139, 0x000C },     /* R139 - EQ6 */
    { 140, 0x0FCA },     /* R140 - EQ7 */
    { 141, 0x0400 },     /* R141 - EQ8 */
    { 142, 0x00D8 },     /* R142 - EQ9 */
    { 143, 0x1EB5 },     /* R143 - EQ10 */
    { 144, 0xF145 },     /* R144 - EQ11 */
    { 145, 0x0B75 },     /* R145 - EQ12 */
    { 146, 0x01C5 },     /* R146 - EQ13 */
    { 147, 0x1C58 },     /* R147 - EQ14 */
    { 148, 0xF373 },     /* R148 - EQ15 */
    { 149, 0x0A54 },     /* R149 - EQ16 */
    { 150, 0x0558 },     /* R150 - EQ17 */
    { 151, 0x168E },     /* R151 - EQ18 */
    { 152, 0xF829 },     /* R152 - EQ19 */
    { 153, 0x07AD },     /* R153 - EQ20 */
    { 154, 0x1103 },     /* R154 - EQ21 */
    { 155, 0x0564 },     /* R155 - EQ22 */
    { 156, 0x0559 },     /* R156 - EQ23 */
    { 157, 0x4000 },     /* R157 - EQ24 */
    { 161, 0x0000 },     /* R161 - Control Interface Test 1 */
    { 204, 0x0000 },     /* R204 - Analogue Output Bias 0 */
    { 247, 0x0000 },     /* R247 - FLL NCO Test 0 */
    { 248, 0x0019 },     /* R248 - FLL NCO Test 1 */
};

#define WM8904InitRegCount 93
int WM8904I2cAllRegDefautInit(DevHandle handle)
{
    int i = 0;
    for (i = 0; i < WM8904InitRegCount; i++) {
        WM8904RegWrite(handle, wm8904_reg_defaults[i].reg, wm8904_reg_defaults[i].def, 2);
    }
    return 0;
}

int WM8904I2cAllRegDump(DevHandle handle)
{
    int i = 0, val = 0;
    for (i = 0; i < 256; i++) {
        WM8904RegRead(g_wm8904_i2c_handle, i, &val, 2);
        WM8904_CODEC_LOG_ERR("dump1_reg(0x%x) val=0x%x", i, val);
    }
    return 0;
}

int WM8904StateInit(DevHandle handle)
{
    WM8904RegUpdateBits(handle, WM8904_MIC_BIAS_CONTROL_0, 0x1, 0, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_LEFT_INPUT_0, 0x1f, 0x18, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_RIGHT_INPUT_0, 0x1f, 0x18, 2);

    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_LEFT_INPUT_0, 0x80, 0, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_RIGHT_INPUT_0, 0x80, 0, 2);
    WM8904RegUpdateBits(handle, WM8904_EQ2, 0x1f, 0xc, 2);

    WM8904RegUpdateBits(handle, WM8904_EQ3, 0x1f, 0xc, 2);
    WM8904RegUpdateBits(handle, WM8904_EQ4, 0x1f, 0xc, 2);
    WM8904RegUpdateBits(handle, WM8904_EQ5, 0x1f, 0xc, 2);
    WM8904RegUpdateBits(handle, WM8904_EQ6, 0x1f, 0xc, 2);

    WM8904RegUpdateBits(handle, WM8904_ADC_DIGITAL_VOLUME_LEFT, 0xfe, 0xc0, 2);
    WM8904RegUpdateBits(handle, WM8904_ADC_DIGITAL_VOLUME_RIGHT, 0xfe, 0xc0, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_LEFT_INPUT_1, 0x3, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_RIGHT_INPUT_1, 0x3, 0x0, 2);

    WM8904RegUpdateBits(handle, WM8904_ADC_DIGITAL_0, 0x10, 0x10, 2);
    WM8904RegUpdateBits(handle, WM8904_ADC_DIGITAL_0, 0x60, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_ADC_0, 0x1, 0x1, 2);
    WM8904RegUpdateBits(handle, WM8904_ADC_TEST_0, 0x5, 0x0, 2);

    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT1_RIGHT, 0x3f, 0x2d, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT1_LEFT, 0x100, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT1_RIGHT, 0x100, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT1_LEFT, 0x40, 0x40, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT1_RIGHT, 0x40, 0x40, 2);

    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT2_LEFT, 0x3f, 0x39, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT2_RIGHT, 0x3f, 0x39, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT2_LEFT, 0x100, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT2_RIGHT, 0x100, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT2_LEFT, 0x40, 0x40, 2);

    WM8904RegUpdateBits(handle, WM8904_ANALOGUE_OUT2_RIGHT, 0x40, 0x40, 2);
    WM8904RegUpdateBits(handle, WM8904_EQ1, 0x1, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_DRC_0, 0x8000, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_DRC_0, 0x4000, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_DAC_DIGITAL_1, 0x40, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_DAC_DIGITAL_1, 0x6, 0x0, 2);
    WM8904RegUpdateBits(handle, WM8904_DAC_DIGITAL_0, 0xff0, 0x0, 2);

    return 0;
}

static int WM8904_Configure_Clocking(void)
{
    unsigned int clock0 = 0, clock2 = 0, rate = 0;
    int ret = 0;
    /* Gate the clock while we're updating to avoid misclocking */
    ret = WM8904RegRead(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_2, &clock2, 2);
    if (ret < 0) {
        return -1;
    }
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_2, WM8904_SYSCLK_SRC, 0, 2);

    /* This should be done on init() for bypass paths */
    switch (gpwm8904->sysclk_src) {
    case WM8904_CLK_MCLK:
        WM8904_CODEC_LOG_DEBUG("Using %dHz MCLK", gpwm8904->mclk_rate);

        clock2 &= ~WM8904_SYSCLK_SRC;
        rate = gpwm8904->mclk_rate;

        /* Ensure the FLL is stopped */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_FLL_CONTROL_1,
                    WM8904_FLL_OSC_ENA | WM8904_FLL_ENA, 0, 2);
        break;

    case WM8904_CLK_FLL:
        WM8904_CODEC_LOG_DEBUG("Using %dHz FLL clock",
            gpwm8904->fll_fout);

        clock2 |= WM8904_SYSCLK_SRC;
        rate = gpwm8904->fll_fout;
        break;

    default:
        WM8904_CODEC_LOG_ERR("System clock not configured");
        return -EINVAL;
    }

    /* SYSCLK shouldn't be over 13.5MHz */
    if (rate > 13500000) {
        clock0 = WM8904_MCLK_DIV;
        gpwm8904->sysclk_rate = rate / 2;
    } else {
        clock0 = 0;
        gpwm8904->sysclk_rate = rate;
    }

    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_0, WM8904_MCLK_DIV,
                clock0, 2);

    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_2,
                WM8904_CLK_SYS_ENA | WM8904_SYSCLK_SRC, clock2, 2);

    WM8904_CODEC_LOG_DEBUG("CLK_SYS is %dHz\n", gpwm8904->sysclk_rate);

    return 0;
}

static void WM8904_Set_Retune_Mobile(void)
{
    struct wm8904_pdata *pdata = gpwm8904->pdata;
    int best = 0, best_val = 0, save = 0, i = 0, cfg = 0, ret = 0;

    if (!pdata || !gpwm8904->num_retune_mobile_texts) {
        return;
    }

    /* Find the version of the currently selected configuration
     * with the nearest sample rate. */
    cfg = gpwm8904->retune_mobile_cfg;
    best = 0;
    best_val = INT_MAX;
    for (i = 0; i < pdata->num_retune_mobile_cfgs; i++) {
        if (strcmp(pdata->retune_mobile_cfgs[i].name,
               gpwm8904->retune_mobile_texts[cfg]) == 0 &&
            abs(pdata->retune_mobile_cfgs[i].rate
            - gpwm8904->fs) < best_val) {
            best = i;
            best_val = abs(pdata->retune_mobile_cfgs[i].rate
                       - gpwm8904->fs);
        }
    }

    WM8904_CODEC_LOG_DEBUG("ReTune Mobile %s/%dHz for %dHz sample rate\n",
        pdata->retune_mobile_cfgs[best].name,
        pdata->retune_mobile_cfgs[best].rate,
        gpwm8904->fs);

    /* The EQ will be disabled while reconfiguring it, remember the
     * current configuration
     */
    ret = WM8904RegRead(g_wm8904_i2c_handle, WM8904_EQ1, &save, 2);

    for (i = 0; i < WM8904_EQ_REGS; i++) {
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_EQ1 + i, 0xffff,
                pdata->retune_mobile_cfgs[best].regs[i], 2);
    }

    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_EQ1, WM8904_EQ_ENA, save, 2);
}

static int deemph_settings[] = { 0, 32000, 44100, 48000 };

static int WM8904_Set_Deemph(void)
{
    int val = 0, i = 0, best = 0;

    /* If we're using deemphasis select the nearest available sample
     * rate.
     */
    if (gpwm8904->deemph) {
        best = 1;
        for (i = 2; i < ARRAY_SIZE(deemph_settings); i++) {
            if (abs(deemph_settings[i] - gpwm8904->fs) <
                abs(deemph_settings[best] - gpwm8904->fs)) {
                best = i;
            }
        }

        val = best << WM8904_DEEMPH_SHIFT;
    } else {
        val = 0;
    }

    WM8904_CODEC_LOG_DEBUG("Set deemphasis %d", val);

    return WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_DAC_DIGITAL_1,
                   WM8904_DEEMPH_MASK, val, 2);
}

int out_pga(int reg, int event)
{
    int val = 0;
    int dcs_mask = 0;
    int dcs_l = 0, dcs_r = 0;
    int dcs_l_reg = 0, dcs_r_reg = 0;
    int timeout = 0;
    int pwr_reg = 0;

    /* This code is shared between HP and LINEOUT; we do all our
     * power management in stereo pairs to avoid latency issues so
     * we reuse shift to identify which rather than strcmp() the
     * name. */
    switch (reg) {
    case WM8904_ANALOGUE_HP_0:
        pwr_reg = WM8904_POWER_MANAGEMENT_2;
        dcs_mask = WM8904_DCS_ENA_CHAN_0 | WM8904_DCS_ENA_CHAN_1;
        dcs_r_reg = WM8904_DC_SERVO_8;
        dcs_l_reg = WM8904_DC_SERVO_9;
        dcs_l = 0;
        dcs_r = 1;
        break;
    case WM8904_ANALOGUE_LINEOUT_0:
        pwr_reg = WM8904_POWER_MANAGEMENT_3;
        dcs_mask = WM8904_DCS_ENA_CHAN_2 | WM8904_DCS_ENA_CHAN_3;
        dcs_r_reg = WM8904_DC_SERVO_6;
        dcs_l_reg = WM8904_DC_SERVO_7;
        dcs_l = 2;
        dcs_r = 3;
        break;
    default:
        WARN(1, "Invalid reg %d\n", reg);
        return -EINVAL;
    }

    switch (event) {
    case SND_SOC_DAPM_PRE_PMU:
        /* Power on the PGAs */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, pwr_reg,
                    WM8904_HPL_PGA_ENA | WM8904_HPR_PGA_ENA,
                    WM8904_HPL_PGA_ENA | WM8904_HPR_PGA_ENA, 2);

        /* Power on the amplifier */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, reg,
                    WM8904_HPL_ENA | WM8904_HPR_ENA,
                    WM8904_HPL_ENA | WM8904_HPR_ENA, 2);

        /* Enable the first stage */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, reg,
                    WM8904_HPL_ENA_DLY | WM8904_HPR_ENA_DLY,
                    WM8904_HPL_ENA_DLY | WM8904_HPR_ENA_DLY, 2);

        /* Power up the DC servo */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_DC_SERVO_0,
                    dcs_mask, dcs_mask, 2);

        /* Either calibrate the DC servo or restore cached state
         * if we have that.
         */
        if (gpwm8904->dcs_state[dcs_l] || gpwm8904->dcs_state[dcs_r]) {
            WM8904_CODEC_LOG_DEBUG("Restoring DC servo state");

                        WM8904RegWrite(g_wm8904_i2c_handle, dcs_l_reg,
                      gpwm8904->dcs_state[dcs_l], 2);
            WM8904RegWrite(g_wm8904_i2c_handle, dcs_r_reg,
                      gpwm8904->dcs_state[dcs_r], 2);

            WM8904RegWrite(g_wm8904_i2c_handle, WM8904_DC_SERVO_1, dcs_mask, 2);

            timeout = 20;
        } else {
            WM8904_CODEC_LOG_DEBUG("Calibrating DC servo");

            WM8904RegWrite(g_wm8904_i2c_handle, WM8904_DC_SERVO_1,
                dcs_mask << WM8904_DCS_TRIG_STARTUP_0_SHIFT, 2);

            timeout = 500;
        }

        /* Wait for DC servo to complete */
        dcs_mask <<= WM8904_DCS_CAL_COMPLETE_SHIFT;
        do {
            WM8904RegRead(g_wm8904_i2c_handle, WM8904_DC_SERVO_READBACK_0, &val , 2);
            if ((val & dcs_mask) == dcs_mask) {
                break;
            }
            msleep(1);
        } while (--timeout);

        if ((val & dcs_mask) != dcs_mask) {
            WM8904_CODEC_LOG_DEBUG("DC servo timed out");
        } else {
            WM8904_CODEC_LOG_DEBUG("DC servo ready");
        }

        /* Enable the output stage */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, reg,
                    WM8904_HPL_ENA_OUTP | WM8904_HPR_ENA_OUTP,
                    WM8904_HPL_ENA_OUTP | WM8904_HPR_ENA_OUTP, 2);
        break;

    case SND_SOC_DAPM_POST_PMU:
        /* Unshort the output itself */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, reg,
                    WM8904_HPL_RMV_SHORT |
                    WM8904_HPR_RMV_SHORT,
                    WM8904_HPL_RMV_SHORT |
                    WM8904_HPR_RMV_SHORT, 2);

        break;

    case SND_SOC_DAPM_PRE_PMD:
        /* Short the output */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, reg,
                    WM8904_HPL_RMV_SHORT |
                    WM8904_HPR_RMV_SHORT, 0, 2);
        break;

    case SND_SOC_DAPM_POST_PMD:
        /* Cache the DC servo configuration; this will be
         * invalidated if we change the configuration. */
                WM8904RegRead(g_wm8904_i2c_handle, dcs_l_reg, &(gpwm8904->dcs_state[dcs_l]), 2);
                WM8904RegRead(g_wm8904_i2c_handle, dcs_r_reg, &(gpwm8904->dcs_state[dcs_l]), 2);

        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_DC_SERVO_0,
                    dcs_mask, 0, 2);

        /* Disable the amplifier input and output stages */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, reg,
                    WM8904_HPL_ENA | WM8904_HPR_ENA |
                    WM8904_HPL_ENA_DLY | WM8904_HPR_ENA_DLY |
                    WM8904_HPL_ENA_OUTP | WM8904_HPR_ENA_OUTP,
                    0, 2);

        /* PGAs too */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, pwr_reg,
                    WM8904_HPL_PGA_ENA | WM8904_HPR_PGA_ENA,
                    0, 2);
        break;
    }

    return 0;
}

static struct {
    int ratio;
    unsigned int clk_sys_rate;
} clk_sys_rates[] = {
    {   64,  0 },
    {  128,  1 },
    {  192,  2 },
    {  256,  3 },
    {  384,  4 },
    {  512,  5 },
    {  786,  6 },
    { 1024,  7 },
    { 1408,  8 },
    { 1536,  9 },
};

static struct {
    int rate;
    int sample_rate;
} sample_rates[] = {
    { 8000,  0  },
    { 11025, 1  },
    { 12000, 1  },
    { 16000, 2  },
    { 22050, 3  },
    { 24000, 3  },
    { 32000, 4  },
    { 44100, 5  },
    { 48000, 5  },
};

static struct {
    int div; /* *10 due to .5s */
    int bclk_div;
} bclk_divs[] = {
    { 10,  0  },
    { 15,  1  },
    { 20,  2  },
    { 30,  3  },
    { 40,  4  },
    { 50,  5  },
    { 55,  6  },
    { 60,  7  },
    { 80,  8  },
    { 100, 9  },
    { 110, 10 },
    { 120, 11 },
    { 160, 12 },
    { 200, 13 },
    { 220, 14 },
    { 240, 16 },
    { 200, 17 },
    { 320, 18 },
    { 440, 19 },
    { 480, 20 },
};

static int Frame_To_Bit_Width(enum AudioFormat format, int *bitWidth)
{
    switch (format) {
    case AUDIO_FORMAT_PCM_16_BIT:
        *bitWidth = 16;
        break;
    case AUDIO_FORMAT_PCM_24_BIT:
    case AUDIO_FORMAT_PCM_32_BIT:
        *bitWidth = 32;
        break;
    default:
        WM8904_CODEC_LOG_ERR("format: %d is not define", format);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int WM8904_Set_Sysclk(int clk_id, unsigned int freq);
static int WM8904_Set_Fmt(unsigned int fmt);
int WM8904_SET_BIAS_LEVEL(enum snd_soc_bias_level level);
int g_wm8904HwPara = 0;
struct AudioPcmHwParams g_PcmParams = {0};
int Wm8904DaiHwParamsSet(const struct AudioCard *card, const struct AudioPcmHwParams *param)
{
    int ret = 0, i = 0, best = 0, best_val = 0, cur_val = 0;
    unsigned int aif1 = 0;
    unsigned int aif2 = 0;
    unsigned int aif3 = 0;
    unsigned int clock1 = 0;
    unsigned int dac_digital1 = 0;

    int fs = 0, width = 0, channel = 0, slots = 0, dir = 0;
    memcpy_s(&g_PcmParams, sizeof(struct AudioPcmHwParams), param, sizeof(struct AudioPcmHwParams));

    fs = param->rate;
    Frame_To_Bit_Width(param->format, &width);
    channel = param->channels;
    slots = 1;

    if (param->streamType == AUDIO_RENDER_STREAM) {
        dir = 1;
    } else {
        dir = 0;
    }

    WM8904_Set_Fmt(16385);
    msleep(10);
    WM8904_Set_Sysclk(1, 0);
    msleep(10);

    /* What BCLK do we need? */
    gpwm8904->fs = fs;
    gpwm8904->bclk = fs * width * channel * slots;
    WM8904_CODEC_LOG_ERR("fs1 = %d tdm_width = %d channel = %d,tdm_slots =%d bclk = %d",
                         fs, width, slots, channel, gpwm8904->bclk);

    switch (width) {
    case 16:
        break;
    case 20:
        aif1 |= 0x40;
        break;
    case 24:
        aif1 |= 0x80;
        break;
    case 32:
        aif1 |= 0xc0;
        break;
    default:
        return -EINVAL;
    }

    ret = WM8904_Configure_Clocking();
    if (ret != 0) {
        return ret;
    }

    /* Select nearest CLK_SYS_RATE */
    best = 0;
    best_val = abs((gpwm8904->sysclk_rate / clk_sys_rates[0].ratio)
               - gpwm8904->fs);
    for (i = 1; i < ARRAY_SIZE(clk_sys_rates); i++) {
        cur_val = abs((gpwm8904->sysclk_rate /
                   clk_sys_rates[i].ratio) - gpwm8904->fs);
        if (cur_val < best_val) {
            best = i;
            best_val = cur_val;
        }
    }
    WM8904_CODEC_LOG_DEBUG("Selected CLK_SYS_RATIO of %d", clk_sys_rates[best].ratio);
        clock1 |= (clk_sys_rates[best].clk_sys_rate
           << WM8904_CLK_SYS_RATE_SHIFT);

    /* SAMPLE_RATE */
    best = 0;
    best_val = abs(gpwm8904->fs - sample_rates[0].rate);
    for (i = 1; i < ARRAY_SIZE(sample_rates); i++) {
        /* Closest match */
        cur_val = abs(gpwm8904->fs - sample_rates[i].rate);
        if (cur_val < best_val) {
            best = i;
            best_val = cur_val;
        }
    }
        WM8904_CODEC_LOG_DEBUG("Selected SAMPLE_RATE of %dHz", sample_rates[best].rate);
        clock1 |= (sample_rates[best].sample_rate
           << WM8904_SAMPLE_RATE_SHIFT);

    /* Enable sloping stopband filter for low sample rates */
    if (gpwm8904->fs <= 24000) {
        dac_digital1 |= WM8904_DAC_SB_FILT;
    }

    /* BCLK_DIV */
    best = 0;
    best_val = INT_MAX;
    for (i = 0; i < ARRAY_SIZE(bclk_divs); i++) {
        cur_val = ((gpwm8904->sysclk_rate * 10) / bclk_divs[i].div)
            - gpwm8904->bclk;
        if (cur_val < 0) { /* Table is sorted */
            break;
        }
        if (cur_val < best_val) {
            best = i;
            best_val = cur_val;
        }
    }
    gpwm8904->bclk = (gpwm8904->sysclk_rate * 10) / bclk_divs[best].div;

        WM8904_CODEC_LOG_DEBUG("Selected BCLK_DIV of %d for %dHz BCLK\n", bclk_divs[best].div, gpwm8904->bclk);

    aif2 |= bclk_divs[best].bclk_div;

    /* LRCLK is a simple fraction of BCLK */
    WM8904_CODEC_LOG_DEBUG("LRCLK_RATE is %d\n", gpwm8904->bclk / gpwm8904->fs);

    aif3 |= gpwm8904->bclk / gpwm8904->fs;

    /* Apply the settings */
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_DAC_DIGITAL_1, WM8904_DAC_SB_FILT, dac_digital1, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_AUDIO_INTERFACE_1, WM8904_AIF_WL_MASK, aif1, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_AUDIO_INTERFACE_2, WM8904_BCLK_DIV_MASK, aif2, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_AUDIO_INTERFACE_3, WM8904_LRCLK_RATE_MASK, aif3, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_1,
                        WM8904_SAMPLE_RATE_MASK | WM8904_CLK_SYS_RATE_MASK, clock1, 2);

    /* Update filters for the new settings */
    WM8904_Set_Retune_Mobile();
    WM8904_Set_Deemph();

    // WM8904I2cAllRegDump(g_wm8904_i2c_handle);
    g_wm8904HwPara = 1;
    return 0;
}

int32_t Wm8904DaiTrigger(const struct AudioCard *audioCard, int cmd, const struct DaiDevice *dai)
{
    WM8904_CODEC_LOG_ERR("cmd = %d ", cmd);
    int32_t val = 0;

    switch (cmd)
    {
        case AUDIO_DRV_PCM_IOCTL_RENDER_START:
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_STANDBY);
        msleep(10);
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_PREPARE);
        msleep(120);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_2, 0x7, 0x7, 2);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CHARGE_PUMP_0, 0x1, 0x1, 2);
        msleep(120);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CLASS_W_0, 0xc, 0xc, 2);
        out_pga(90, 1);
        WM8904RegRead(g_wm8904_i2c_handle, 90, &val, 2);
        msleep(50);
        out_pga(94, 1);
        msleep(50);
        out_pga(90, 2);
        msleep(10);
        out_pga(94, 2);
        msleep(10);
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_ON);
        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_POWER_MANAGEMENT_6, 0xc, 2);
        msleep(10);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_DAC_DIGITAL_1, WM8904_DAC_MUTE, 0x0, 2);
        break;
        case AUDIO_DRV_PCMIOCTL_CAPUTRE_START:
        // record
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_STANDBY);
        msleep(10);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_MIC_BIAS_CONTROL_0, 0x1, 0x1, 2);
        msleep(50);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_2, 0x6, 0x6, 2);
        msleep(50);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_POWER_MANAGEMENT_0, 0x3, 0x3, 2);
        msleep(50);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_POWER_MANAGEMENT_6, 0x3, 0x3, 2);
        msleep(50);
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_STANDBY);
        msleep(50);
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_PREPARE);
        msleep(50);
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_ON);
        msleep(50);
        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_ANALOGUE_LEFT_INPUT_1, 0x45, 2);
        msleep(50);
        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_ANALOGUE_RIGHT_INPUT_1, 0x45, 2);
    break;
    case AUDIO_DRV_PCM_IOCTL_RENDER_PAUSE:
    case AUDIO_DRV_PCM_IOCTL_RENDER_STOP:
        /* code */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_DAC_DIGITAL_1, WM8904_DAC_MUTE, 8, 2);
        msleep(50);
        out_pga(90, 0x4);
        msleep(50);
        out_pga(94, 0x4);
        msleep(50);
        out_pga(90, 0x8);
        msleep(50);
        out_pga(94, 0x8);
        msleep(50);

        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_POWER_MANAGEMENT_6, 0, 2);
        msleep(50);
        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_2, 0, 2);
        msleep(50);
        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_FLL_CONTROL_1, 0, 2);
        msleep(50);
        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_CHARGE_PUMP_0, 0, 2);
        msleep(50);

        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_PREPARE);
        msleep(50);
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_STANDBY);
        msleep(50);
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_OFF);
        break;

    case AUDIO_DRV_PCM_IOCTL_CAPTURE_PAUSE:
    case AUDIO_DRV_PCM_IOCTL_CAPTURE_STOP:
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_PREPARE);
        msleep(50);

        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_POWER_MANAGEMENT_6, 0, 2);
        msleep(50);
        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_POWER_MANAGEMENT_0, 0, 2);
        msleep(50);
        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_2, 0, 2);
        msleep(50);
        WM8904RegWrite(g_wm8904_i2c_handle, WM8904_FLL_CONTROL_1, 0, 2);
        msleep(50);

        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_STANDBY);
        msleep(50);
        WM8904_SET_BIAS_LEVEL(SND_SOC_BIAS_OFF);
        break;
     default:
     break;
     }
    return HDF_SUCCESS;
}

static int WM8904_Set_Sysclk(int clk_id, unsigned int freq)
{
    struct wm8904_priv *priv = gpwm8904;

    switch (clk_id) {
    case WM8904_CLK_MCLK:
        priv->sysclk_src = clk_id;
        priv->mclk_rate = freq;
        break;

    case WM8904_CLK_FLL:
        priv->sysclk_src = clk_id;
        break;

    default:
        return -EINVAL;
    }

    WM8904_CODEC_LOG_DEBUG("Clock source is %d at %uHz\n", clk_id, freq);

    WM8904_Configure_Clocking();

    return 0;
}

static int WM8904_Set_Fmt(unsigned int fmt)
{
    unsigned int aif1 = 0;
    unsigned int aif3 = 0;

    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
    case SND_SOC_DAIFMT_CBS_CFS:
        break;
    case SND_SOC_DAIFMT_CBS_CFM:
        aif3 |= WM8904_LRCLK_DIR;
        break;
    case SND_SOC_DAIFMT_CBM_CFS:
        aif1 |= WM8904_BCLK_DIR;
        break;
    case SND_SOC_DAIFMT_CBM_CFM:
        aif1 |= WM8904_BCLK_DIR;
        aif3 |= WM8904_LRCLK_DIR;
        break;
    default:
        return -EINVAL;
    }

    switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
    case SND_SOC_DAIFMT_DSP_B:
        aif1 |= 0x3 | WM8904_AIF_LRCLK_INV;
        /* fall through */
    case SND_SOC_DAIFMT_DSP_A:
        aif1 |= 0x3;
        break;
    case SND_SOC_DAIFMT_I2S:
        aif1 |= 0x2;
        break;
    case SND_SOC_DAIFMT_RIGHT_J:
        break;
    case SND_SOC_DAIFMT_LEFT_J:
        aif1 |= 0x1;
        break;
    default:
        return -EINVAL;
    }

    switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
    case SND_SOC_DAIFMT_DSP_A:
    case SND_SOC_DAIFMT_DSP_B:
        /* frame inversion not valid for DSP modes */
        switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
        case SND_SOC_DAIFMT_NB_NF:
            break;
        case SND_SOC_DAIFMT_IB_NF:
            aif1 |= WM8904_AIF_BCLK_INV;
            break;
        default:
            return -EINVAL;
        }
        break;

    case SND_SOC_DAIFMT_I2S:
    case SND_SOC_DAIFMT_RIGHT_J:
    case SND_SOC_DAIFMT_LEFT_J:
        switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
        case SND_SOC_DAIFMT_NB_NF:
            break;
        case SND_SOC_DAIFMT_IB_IF:
            aif1 |= WM8904_AIF_BCLK_INV | WM8904_AIF_LRCLK_INV;
            break;
        case SND_SOC_DAIFMT_IB_NF:
            aif1 |= WM8904_AIF_BCLK_INV;
            break;
        case SND_SOC_DAIFMT_NB_IF:
            aif1 |= WM8904_AIF_LRCLK_INV;
            break;
        default:
            return -EINVAL;
        }
        break;
    default:
        return -EINVAL;
    }

    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_AUDIO_INTERFACE_1,
                WM8904_AIF_BCLK_INV | WM8904_AIF_LRCLK_INV |
                WM8904_AIF_FMT_MASK | WM8904_BCLK_DIR, aif1, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_AUDIO_INTERFACE_3,
                WM8904_LRCLK_DIR, aif3, 2);

    return 0;
}

struct _fll_div {
    u16 fll_fratio;
    u16 fll_outdiv;
    u16 fll_clk_ref_div;
    u16 n;
    u16 k;
};

/* The size in bits of the FLL divide multiplied by 10
 * to allow rounding later */
#define FIXED_FLL_SIZE ((1 << 16) * 10)

int g_cur_bias_level = SND_SOC_BIAS_OFF;

#define WM8904_RATES SNDRV_PCM_RATE_8000_96000

#define WM8904_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE |\
            SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE)

int WM8904_SET_BIAS_LEVEL(enum snd_soc_bias_level level);
int find_i2c_client(struct device *dev, void *data);

/* HdfDriverEntry implementations */
static int32_t CodecDriverBind(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        WM8904_CODEC_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    struct CodecHost *codecHost = (struct CodecHost *)OsalMemCalloc(sizeof(*codecHost));
    if (codecHost == NULL) {
        WM8904_CODEC_LOG_ERR("malloc codecHost fail!");
        return HDF_FAILURE;
    }
    codecHost->device = device;
    device->service = &codecHost->service;
    return HDF_SUCCESS;
}

struct CodecData g_g8904_codec_data = {
    .Init = Wm8904DevInit,
    .Read = CodecDeviceRegI2cRead,
    .Write = CodecDeviceRegI2cWrite,
};

struct AudioDaiOps g_g8904_codec_dai_device_ops = {
    .Startup = Wm8904DaiStart,
    .HwParams = Wm8904DaiHwParamsSet,
    .Trigger = Wm8904DaiTrigger,
};

struct DaiData g_g8904_codec_dai_data = {
    .drvDaiName = "wm8904_codec_dai",
    .DaiInit = Wm8904DaiDevInit,
    .ops = &g_g8904_codec_dai_device_ops,
};

#define FINDFLAG 1
int find_i2c_client(struct device *dev, void *data)
{
    struct i2c_client *client = to_i2c_client(dev);
    if (client != NULL) {
        if (!strncmp(client->name, "wm8904", 6)) {
            *((struct i2c_client **) data) = client;
            return FINDFLAG;
        } else {
            return !FINDFLAG;
        }
    } else {
        return !FINDFLAG;
    }
}

int WM8904_SET_BIAS_LEVEL(enum snd_soc_bias_level level)
{
    int ret = 0;

    switch (level) {
    case SND_SOC_BIAS_ON:
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_ANALOGUE_LEFT_INPUT_1,
                    WM8904_L_MODE_MASK, 0x01, 2);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_ANALOGUE_RIGHT_INPUT_1,
                    WM8904_R_MODE_MASK, 0x01, 2);
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_MIC_BIAS_CONTROL_0,
                    WM8904_MIC_DET_EINT, 0x01, 2);
        g_cur_bias_level = SND_SOC_BIAS_STANDBY;
        break;
    case SND_SOC_BIAS_PREPARE:
        /* VMID resistance 2*50k */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_VMID_CONTROL_0,
                    WM8904_VMID_RES_MASK,
                    0x1 << WM8904_VMID_RES_SHIFT, 2);

        /* Normal bias current */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_BIAS_CONTROL_0,
                    WM8904_ISEL_MASK, 2 << WM8904_ISEL_SHIFT, 2);
                g_cur_bias_level = SND_SOC_BIAS_PREPARE;
        break;

    case SND_SOC_BIAS_STANDBY:
        if (g_cur_bias_level == SND_SOC_BIAS_OFF) {
            ret = regulator_bulk_enable(ARRAY_SIZE(gpwm8904->supplies),
                            gpwm8904->supplies);
            if (ret != 0) {
                WM8904_CODEC_LOG_ERR("Failed to enable supplies: %d", ret);
                return ret;
            }


            /* Enable bias */
            WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_BIAS_CONTROL_0,
                        WM8904_BIAS_ENA, WM8904_BIAS_ENA, 2);
            /* Enable VMID, VMID buffering, 2*5k resistance */
            WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_VMID_CONTROL_0,
                        WM8904_VMID_ENA |
                        WM8904_VMID_RES_MASK,
                        WM8904_VMID_ENA |
                        0x3 << WM8904_VMID_RES_SHIFT, 2);

            /* Let VMID ramp */
            msleep(1);
        }

        /* Maintain VMID with 2*250k */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_VMID_CONTROL_0,
                    WM8904_VMID_RES_MASK,
                    0x2 << WM8904_VMID_RES_SHIFT, 2);

        /* Bias current *0.5 */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_BIAS_CONTROL_0,
                    WM8904_ISEL_MASK, 0, 2);
                g_cur_bias_level = SND_SOC_BIAS_STANDBY;

        break;

    case SND_SOC_BIAS_OFF:
        /* Turn off VMID */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_VMID_CONTROL_0,
                    WM8904_VMID_RES_MASK | WM8904_VMID_ENA, 0, 2);

        /* Stop bias generation */
        WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_BIAS_CONTROL_0,
                    WM8904_BIAS_ENA, 0, 2);

        regulator_bulk_disable(ARRAY_SIZE(gpwm8904->supplies),
                       gpwm8904->supplies);
        clk_disable_unprepare(gpwm8904->mclk);
                g_cur_bias_level = SND_SOC_BIAS_OFF;
        break;
    }
    return 0;
}

static int32_t Wm8904DriverInit(struct HdfDeviceObject *device)
{
    struct i2c_client **data;
    data = (struct i2c_client **)kzalloc(sizeof(struct i2c_client *) , GFP_KERNEL);

    bus_for_each_dev(&i2c_bus_type, NULL, (void*)(data), find_i2c_client);

    struct i2c_client *i2c = *data;
    if (i2c != NULL) {
        WM8904_CODEC_LOG_DEBUG("find kernel %s device", i2c->name);
    } else {
        WM8904_CODEC_LOG_ERR("wm8904 i2c dev not exist");
        return HDF_ERR_INVALID_OBJECT;
    }

    unsigned int val = 0;
    int ret = 0, i = 0;

    gpwm8904 = devm_kzalloc(&i2c->dev, sizeof(struct wm8904_priv),
                              GFP_KERNEL);
    if (gpwm8904 == NULL) {
        return -ENOMEM;
    }

    g_wm8904_i2c_handle = WM8904I2cOpen();
    ret = WM8904I2cAllRegDefautInit(g_wm8904_i2c_handle);

    gpwm8904->mclk = devm_clk_get(&i2c->dev, "mclk");
    if (IS_ERR(gpwm8904->mclk)) {
        ret = PTR_ERR(gpwm8904->mclk);
        WM8904_CODEC_LOG_ERR("Failed to get MCLK\n");
        return ret;
    }

    i2c_set_clientdata(i2c, gpwm8904);
    gpwm8904->pdata = i2c->dev.platform_data;

    for (i = 0; i < ARRAY_SIZE(gpwm8904->supplies); i++) {
        gpwm8904->supplies[i].supply = wm8904_supply_names[i];
    }

    ret = devm_regulator_bulk_get(&i2c->dev, ARRAY_SIZE(gpwm8904->supplies),
                                  gpwm8904->supplies);
    if (ret != 0) {
            WM8904_CODEC_LOG_ERR("Failed to request supplies: %d", ret);
            return ret;
    }
    ret = regulator_bulk_enable(ARRAY_SIZE(gpwm8904->supplies),
                                gpwm8904->supplies);
    if (ret != 0) {
            WM8904_CODEC_LOG_ERR("Failed to enable supplies: %d", ret);
            return ret;
    }

    ret = WM8904RegRead(g_wm8904_i2c_handle, WM8904_SW_RESET_AND_ID, &val, 2);
    WM8904_CODEC_LOG_DEBUG("WM8904_SW_RESET_AND_ID val=0x%x", val);

    if (ret < 0) {
            WM8904_CODEC_LOG_ERR("Failed to read ID register: %d", ret);
            regulator_bulk_disable(ARRAY_SIZE(gpwm8904->supplies), gpwm8904->supplies);
            return 0;
    }

    if (val != 0x8904) {
            WM8904_CODEC_LOG_ERR("Device is not a WM8904, ID is %x", val);
            ret = -EINVAL;
            regulator_bulk_disable(ARRAY_SIZE(gpwm8904->supplies), gpwm8904->supplies);
            return 0;
    }

    ret = WM8904RegRead(g_wm8904_i2c_handle, WM8904_REVISION, &val, 2);
    if (ret < 0) {
            WM8904_CODEC_LOG_ERR("Failed to read device revision: %d",
                    ret);
            regulator_bulk_disable(ARRAY_SIZE(gpwm8904->supplies), gpwm8904->supplies);
            return 0;
    }
    WM8904_CODEC_LOG_DEBUG("revision %c", val + 'A');
    WM8904_CODEC_LOG_DEBUG("WM8904_REVISION1 val=0x%x", val);

    ret = WM8904RegWrite(g_wm8904_i2c_handle, WM8904_SW_RESET_AND_ID, 0, 2);
    if (ret < 0) {
            WM8904_CODEC_LOG_DEBUG("Failed to issue reset: %d", ret);
            regulator_bulk_disable(ARRAY_SIZE(gpwm8904->supplies), gpwm8904->supplies);
            return 0;
    }

    /* Change some default settings - latch VU and enable ZC */
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_ADC_DIGITAL_VOLUME_LEFT,
                        WM8904_ADC_VU, WM8904_ADC_VU, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_ADC_DIGITAL_VOLUME_RIGHT,
                        WM8904_ADC_VU, WM8904_ADC_VU, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_DAC_DIGITAL_VOLUME_LEFT,
                        WM8904_DAC_VU, WM8904_DAC_VU, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_DAC_DIGITAL_VOLUME_RIGHT,
                        WM8904_DAC_VU, WM8904_DAC_VU, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_ANALOGUE_OUT1_LEFT,
                        WM8904_HPOUT_VU | WM8904_HPOUTLZC,
                        WM8904_HPOUT_VU | WM8904_HPOUTLZC, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_ANALOGUE_OUT1_RIGHT,
                        WM8904_HPOUT_VU | WM8904_HPOUTRZC,
                        WM8904_HPOUT_VU | WM8904_HPOUTRZC, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_ANALOGUE_OUT2_LEFT,
                        WM8904_LINEOUT_VU | WM8904_LINEOUTLZC,
                        WM8904_LINEOUT_VU | WM8904_LINEOUTLZC, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_ANALOGUE_OUT2_RIGHT,
                        WM8904_LINEOUT_VU | WM8904_LINEOUTRZC,
                        WM8904_LINEOUT_VU | WM8904_LINEOUTRZC, 2);
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CLOCK_RATES_0,
                        WM8904_SR_MODE, 0, 2);

    /* Apply configuration from the platform data. */
    if (gpwm8904->pdata) {
        for (i = 0; i < WM8904_GPIO_REGS; i++) {
            if (!gpwm8904->pdata->gpio_cfg[i]) {
                continue;
            }

            WM8904RegUpdateBits(g_wm8904_i2c_handle,
                               WM8904_GPIO_CONTROL_1 + i,
                               0xffff,
                               gpwm8904->pdata->gpio_cfg[i], 2);
        }

        /* Zero is the default value for these anyway */
        for (i = 0; i < WM8904_MIC_REGS; i++) {
            WM8904RegUpdateBits(g_wm8904_i2c_handle,
                               WM8904_MIC_BIAS_CONTROL_0 + i,
                               0xffff,
                               gpwm8904->pdata->mic_cfg[i], 2);
        }
    }

    /* Set Class W by default - this will be managed by the Class
     * G widget at runtime where bypass paths are available.
     */
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_CLASS_W_0,
                        WM8904_CP_DYN_PWR, WM8904_CP_DYN_PWR, 2);

    /* Use normal bias source */
    WM8904RegUpdateBits(g_wm8904_i2c_handle, WM8904_BIAS_CONTROL_0,
                        WM8904_POBCTRL, 0, 2);

    regulator_bulk_disable(ARRAY_SIZE(gpwm8904->supplies), gpwm8904->supplies);

    WM8904StateInit(g_wm8904_i2c_handle);
    WM8904_CODEC_LOG_INFO("Success.");

    return HDF_SUCCESS;
}

static int32_t CodecDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret = 0;

    WM8904_CODEC_LOG_DEBUG("Success.");
    if (device == NULL) {
        WM8904_CODEC_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }
    ret = CodecGetConfigInfo(device, &g_g8904_codec_data);
    if (ret !=  HDF_SUCCESS) {
        WM8904_CODEC_LOG_ERR("get config info failed.");
        return ret;
    }

    if (CodecSetConfigInfo(&g_g8904_codec_data, &g_g8904_codec_dai_data) != HDF_SUCCESS) {
        WM8904_CODEC_LOG_ERR("set config info failed.");
        return HDF_FAILURE;
    }
    ret = CodecGetServiceName(device, &g_g8904_codec_data.drvCodecName);
    if (ret != HDF_SUCCESS) {
        WM8904_CODEC_LOG_ERR("get service name fail");
        return ret;
    }

    ret = AudioRegisterCodec(device, &g_g8904_codec_data, &g_g8904_codec_dai_data);
    if (ret != HDF_SUCCESS) {
        WM8904_CODEC_LOG_ERR("register dai fail.");
        return ret;
    }

    ret = Wm8904DriverInit(device);
    if (ret != HDF_SUCCESS) {
        WM8904_CODEC_LOG_ERR("accessory init fail");
        return ret;
    }

    WM8904_CODEC_LOG_ERR("success!");
    return HDF_SUCCESS;
}

static void CodecDriverRelease(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        WM8904_CODEC_LOG_ERR("device is NULL");
        return;
    }
}

struct HdfDriverEntry g_g8904_codec_driver_entry = {
    .moduleVersion = 1,
    .moduleName = "CODEC_WM8904",
    .Bind = CodecDriverBind,
    .Init = CodecDriverInit,
    .Release = CodecDriverRelease,
};
HDF_INIT(g_g8904_codec_driver_entry);

