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
#include <linux/clk/clk-conf.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/dmaengine.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/mfd/syscon/imx6q-iomuxc-gpr.h>
#include <linux/mfd/syscon.h>
#include <linux/interrupt.h>
#include <linux/pinctrl/consumer.h>
#include <linux/busfreq-imx.h>

#include "audio_driver_log.h"
#include "imx8mm_platform.h"
#include "imx8mm_common.h"
#include "osal_mem.h"
#include "sai_driver.h"

#include "osal_io.h"
#include "osal_irq.h"
#include "osal_sem.h"
#include "osal_time.h"

#define HDF_LOG_TAG imx8mm_sai_driver

#define FSL_SAI_FLAGS (FSL_SAI_CSR_SEIE | FSL_SAI_CSR_FEIE)

#define FSL_SAI_VERID_0301 (0x0301)
struct fsl_sai *g_sai;

static struct fsl_sai *GetDrvSai(const struct PlatformData *pd)
{
    struct PrivPlatformData *ppd = NULL;
    if (pd == NULL) {
        AUDIO_DRIVER_LOG_ERR("get drv sai: the input platform data is null");
        return NULL;
    }
    ppd = (struct PrivPlatformData *)pd->dmaPrv;
    if (ppd != NULL) {
        return &(ppd->sai);
    }

    return NULL;
}

void SaiPrintTCRReg(void)
{
    struct fsl_sai *sai = g_sai;
    u32 read_val = 0;

    regmap_read(sai->regmap, FSL_SAI_TCSR(8), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TCSR %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TCR1(8), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TCR1 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TCR2(8), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TCR2 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TCR3(8), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TCR3 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TCR4(8), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TCR4 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TCR5(8), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TCR5 %08x", read_val);
}

void SaiPrintTDRReg(void)
{
    struct fsl_sai *sai = g_sai;
    u32 read_val = 0;

    regmap_read(sai->regmap, FSL_SAI_TDR0, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TDR0 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TDR1, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TDR1 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TDR2, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TDR2 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TDR3, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TDR3 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TDR4, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TDR4 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TDR5, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TDR5 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TDR6, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TDR6 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TDR7, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TDR7 %08x", read_val);
}

void SaiPrintTFRReg(void)
{
    struct fsl_sai *sai = g_sai;
    u32 read_val = 0;
    regmap_read(sai->regmap, FSL_SAI_TFR0, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TFR0 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TFR1, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TFR1 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TFR2, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TFR2 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TFR3, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TFR3 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TFR4, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TFR4 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TFR5, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TFR5 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TFR6, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TFR6 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TFR7, &read_val);
}

void SaiPrintOthersReg(void)
{
    struct fsl_sai *sai = g_sai;
    u32 read_val = 0;
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TFR7 %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TMR, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TMR %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TTCTL, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TTCTL %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TTCTN, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TTCTN %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_TTCAP, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_TTCAP %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_MCTL, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_MCTL %08x", read_val);
    regmap_read(sai->regmap, FSL_SAI_MDIV, &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_MDIV %08x", read_val);
}

int32_t SaiPrintAllRegister(void)
{
    SaiPrintTCRReg();
    SaiPrintTDRReg();
    SaiPrintTFRReg();
    SaiPrintOthersReg();
    return 0;
}

static irqreturn_t fsl_sai_isr(int irq, void *devid)
{
    struct PrivPlatformData *ppd = (struct PrivPlatformData *)devid;
    struct fsl_sai *sai = (struct fsl_sai *)&(ppd->sai);
    unsigned char offset = sai->reg_offset;
    u32 flags = 0, xcsr = 0, mask = 0;
    bool irq_none = true;

    /*
     * Both IRQ status bits and IRQ mask bits are in the xCSR but
     * different shifts. And we here create a mask only for those
     * IRQs that we activated.
     */
    mask = (FSL_SAI_FLAGS >> FSL_SAI_CSR_xIE_SHIFT) << FSL_SAI_CSR_xF_SHIFT;

    /* Tx IRQ */
    regmap_read(sai->regmap, FSL_SAI_TCSR(offset), &xcsr);
    flags = xcsr & mask;

    if (flags) {
        irq_none = false;
    } else {
        if (flags) {
            irq_none = false;
        } else {
            if (irq_none) {
                return IRQ_NONE;
            } else {
                return IRQ_HANDLED;
            }
        }

        if (flags & FSL_SAI_CSR_WSF) {
            AUDIO_DRIVER_LOG_ERR("isr: Start of Rx word detected\n");
        }

        if (flags & FSL_SAI_CSR_SEF) {
            AUDIO_DRIVER_LOG_ERR("isr: Rx Frame sync error detected\n");
        }

        if (flags & FSL_SAI_CSR_FEF) {
            AUDIO_DRIVER_LOG_ERR("isr: Receive overflow detected\n");
            /* FIFO reset for safety */
            xcsr |= FSL_SAI_CSR_FR;
        }

        if (flags & FSL_SAI_CSR_FWF) {
            AUDIO_DRIVER_LOG_ERR("isr: Enabled receive FIFO is full\n");
        }

        if (flags & FSL_SAI_CSR_FRF) {
            AUDIO_DRIVER_LOG_ERR("isr: Receive FIFO watermark has been reached\n");
        }

        flags &= FSL_SAI_CSR_xF_W_MASK;
        xcsr &= ~FSL_SAI_CSR_xF_MASK;

        if (flags) {
            regmap_write(sai->regmap, FSL_SAI_RCSR(offset), flags | xcsr);
        }

        if (irq_none) {
            return IRQ_NONE;
        } else {
            return IRQ_HANDLED;
        }
    }

    if (flags & FSL_SAI_CSR_WSF) {
        AUDIO_DRIVER_LOG_ERR("isr: Start of Tx word detected\n");
    }

    if (flags & FSL_SAI_CSR_SEF) {
        AUDIO_DRIVER_LOG_ERR("isr: Tx Frame sync error detected\n");
    }

    if (flags & FSL_SAI_CSR_FEF) {
        AUDIO_DRIVER_LOG_ERR("isr: Transmit underrun detected\n");
        /* FIFO reset for safety */
        xcsr |= FSL_SAI_CSR_FR;
    }

    if (flags & FSL_SAI_CSR_FWF) {
        AUDIO_DRIVER_LOG_ERR("isr: Enabled transmit FIFO is empty\n");
    }

    if (flags & FSL_SAI_CSR_FRF) {
        AUDIO_DRIVER_LOG_ERR("isr: Transmit FIFO watermark has been reached\n");
    }

    flags &= FSL_SAI_CSR_xF_W_MASK;
    xcsr &= ~FSL_SAI_CSR_xF_MASK;

    if (flags) {
        regmap_write(sai->regmap, FSL_SAI_TCSR(offset), flags | xcsr);
    }

    /* Rx IRQ */
    regmap_read(sai->regmap, FSL_SAI_RCSR(offset), &xcsr);
    flags = xcsr & mask;

    if (flags) {
        irq_none = false;
    } else {
        if (irq_none) {
            return IRQ_NONE;
        } else {
            return IRQ_HANDLED;
        }
    }

    if (flags & FSL_SAI_CSR_WSF) {
        AUDIO_DRIVER_LOG_ERR("isr: Start of Rx word detected\n");
    }

    if (flags & FSL_SAI_CSR_SEF) {
        AUDIO_DRIVER_LOG_ERR("isr: Rx Frame sync error detected\n");
    }

    if (flags & FSL_SAI_CSR_FEF) {
        AUDIO_DRIVER_LOG_ERR("isr: Receive overflow detected\n");
        /* FIFO reset for safety */
        xcsr |= FSL_SAI_CSR_FR;
    }

    if (flags & FSL_SAI_CSR_FWF) {
        AUDIO_DRIVER_LOG_ERR("isr: Enabled receive FIFO is full\n");
    }

    if (flags & FSL_SAI_CSR_FRF) {
        AUDIO_DRIVER_LOG_ERR("isr: Receive FIFO watermark has been reached\n");
    }

    flags &= FSL_SAI_CSR_xF_W_MASK;
    xcsr &= ~FSL_SAI_CSR_xF_MASK;

    if (flags) {
        regmap_write(sai->regmap, FSL_SAI_RCSR(offset), flags | xcsr);
    }

    if (irq_none) {
        return IRQ_NONE;
    } else {
        return IRQ_HANDLED;
    }
}

static struct reg_default fsl_sai_v3_reg_defaults[] = {
    {FSL_SAI_TCR1(8), 0},
    {FSL_SAI_TCR2(8), 0},
    {FSL_SAI_TCR3(8), 0},
    {FSL_SAI_TCR4(8), 0},
    {FSL_SAI_TCR5(8), 0},
    {FSL_SAI_TDR0, 0},
    {FSL_SAI_TDR1, 0},
    {FSL_SAI_TDR2, 0},
    {FSL_SAI_TDR3, 0},
    {FSL_SAI_TDR4, 0},
    {FSL_SAI_TDR5, 0},
    {FSL_SAI_TDR6, 0},
    {FSL_SAI_TDR7, 0},
    {FSL_SAI_TMR,  0},
    {FSL_SAI_RCR1(8), 0},
    {FSL_SAI_RCR2(8), 0},
    {FSL_SAI_RCR3(8), 0},
    {FSL_SAI_RCR4(8), 0},
    {FSL_SAI_RCR5(8), 0},
    {FSL_SAI_RMR,  0},
    {FSL_SAI_MCTL, 0},
    {FSL_SAI_MDIV, 0},
};

static bool fsl_sai_readable_reg(struct device *dev, unsigned int reg)
{
    struct fsl_sai *sai = dev_get_drvdata(dev);
    unsigned char offset = sai->reg_offset;

    if (reg >= FSL_SAI_TCSR(offset) && reg <= FSL_SAI_TCR5(offset)) {
        return true;
    }

    if (reg >= FSL_SAI_RCSR(offset) && reg <= FSL_SAI_RCR5(offset)) {
        return true;
    }

    switch (reg) {
        case FSL_SAI_TFR0:
        case FSL_SAI_TFR1:
        case FSL_SAI_TFR2:
        case FSL_SAI_TFR3:
        case FSL_SAI_TFR4:
        case FSL_SAI_TFR5:
        case FSL_SAI_TFR6:
        case FSL_SAI_TFR7:
        case FSL_SAI_TMR:
        case FSL_SAI_RDR0:
        case FSL_SAI_RDR1:
        case FSL_SAI_RDR2:
        case FSL_SAI_RDR3:
        case FSL_SAI_RDR4:
        case FSL_SAI_RDR5:
        case FSL_SAI_RDR6:
        case FSL_SAI_RDR7:
        case FSL_SAI_RFR0:
        case FSL_SAI_RFR1:
        case FSL_SAI_RFR2:
        case FSL_SAI_RFR3:
        case FSL_SAI_RFR4:
        case FSL_SAI_RFR5:
        case FSL_SAI_RFR6:
        case FSL_SAI_RFR7:
        case FSL_SAI_RMR:
        case FSL_SAI_MCTL:
        case FSL_SAI_MDIV:
        case FSL_SAI_VERID:
        case FSL_SAI_PARAM:
        case FSL_SAI_TTCTN:
        case FSL_SAI_RTCTN:
        case FSL_SAI_TTCTL:
        case FSL_SAI_TBCTN:
        case FSL_SAI_TTCAP:
        case FSL_SAI_RTCTL:
        case FSL_SAI_RBCTN:
        case FSL_SAI_RTCAP:
            return true;
        default:
            return false;
    }
}

static bool fsl_sai_volatile_reg(struct device *dev, unsigned int reg)
{
    struct fsl_sai *sai = dev_get_drvdata(dev);
    unsigned char offset = sai->reg_offset;

    if (reg == FSL_SAI_TCSR(offset) || reg == FSL_SAI_RCSR(offset)) {
        return true;
    }

    if (sai->reg_offset == 8 && (reg == FSL_SAI_VERID ||
                                 reg == FSL_SAI_PARAM)) {
        return true;
    }

    switch (reg) {
        case FSL_SAI_TFR0:
        case FSL_SAI_TFR1:
        case FSL_SAI_TFR2:
        case FSL_SAI_TFR3:
        case FSL_SAI_TFR4:
        case FSL_SAI_TFR5:
        case FSL_SAI_TFR6:
        case FSL_SAI_TFR7:
        case FSL_SAI_RFR0:
        case FSL_SAI_RFR1:
        case FSL_SAI_RFR2:
        case FSL_SAI_RFR3:
        case FSL_SAI_RFR4:
        case FSL_SAI_RFR5:
        case FSL_SAI_RFR6:
        case FSL_SAI_RFR7:
        case FSL_SAI_RDR0:
        case FSL_SAI_RDR1:
        case FSL_SAI_RDR2:
        case FSL_SAI_RDR3:
        case FSL_SAI_RDR4:
        case FSL_SAI_RDR5:
        case FSL_SAI_RDR6:
        case FSL_SAI_RDR7:
            return true;
        default:
            return false;
    }
}

static bool fsl_sai_writeable_reg(struct device *dev, unsigned int reg)
{
    struct fsl_sai *sai = dev_get_drvdata(dev);
    unsigned char offset = sai->reg_offset;

    if (reg >= FSL_SAI_TCSR(offset) && reg <= FSL_SAI_TCR5(offset)) {
        return true;
    }

    if (reg >= FSL_SAI_RCSR(offset) && reg <= FSL_SAI_RCR5(offset)) {
        return true;
    }

    switch (reg) {
        case FSL_SAI_TDR0:
        case FSL_SAI_TDR1:
        case FSL_SAI_TDR2:
        case FSL_SAI_TDR3:
        case FSL_SAI_TDR4:
        case FSL_SAI_TDR5:
        case FSL_SAI_TDR6:
        case FSL_SAI_TDR7:
        case FSL_SAI_TMR:
        case FSL_SAI_RMR:
        case FSL_SAI_MCTL:
        case FSL_SAI_MDIV:
        case FSL_SAI_TTCTL:
        case FSL_SAI_RTCTL:
            return true;
        default:
            return false;
    }
}

static const struct regmap_config fsl_sai_regmap_config = {
    .reg_bits = 32,
    .reg_stride = 4,
    .val_bits = 32,

    .max_register = FSL_SAI_MDIV,
    .reg_defaults = fsl_sai_v3_reg_defaults,
    .num_reg_defaults = ARRAY_SIZE(fsl_sai_v3_reg_defaults),
    .readable_reg = fsl_sai_readable_reg,
    .volatile_reg = fsl_sai_volatile_reg,
    .writeable_reg = fsl_sai_writeable_reg,
    .cache_type = REGCACHE_FLAT,
};

#define OFFSET_7 (7)
#define OFFSET_8 (8)
static unsigned int fsl_sai_calc_dl_off(unsigned long dl_mask)
{
    int fbidx = 0, nbidx = 0, offset = 0;

    fbidx = find_first_bit(&dl_mask, OFFSET_8);
    nbidx = find_next_bit(&dl_mask, OFFSET_8, fbidx + 1);
    offset = nbidx - fbidx - 1;

    return (offset < 0 || offset >= OFFSET_7 ? 0 : offset);
}

#define ELEMS_3  (3)
static int fsl_sai_read_dlcfg(struct platform_device *pdev, char *pn,
                              struct fsl_sai_dl_cfg **rcfg, unsigned int soc_dl)
{
    int ret = 0, elems = 0, i = 0, index = 0, num_cfg = 0;
    struct device_node *np = pdev->dev.of_node;
    struct fsl_sai_dl_cfg *cfg = NULL;
    u32 rx = 0, tx = 0, pins = 0;

    *rcfg = NULL;

    elems = of_property_count_u32_elems(np, pn);
    /* consider default value "0 0x1 0x1" if property is missing */
    if (elems <= 0) {
        elems = ELEMS_3;
    }

    if (elems % ELEMS_3) {
        AUDIO_DRIVER_LOG_ERR(
            "Number of elements in %s must be divisible to 3.\n", pn);
        return -EINVAL;
    }

    num_cfg = elems / ELEMS_3;
    cfg = devm_kzalloc(&pdev->dev, num_cfg * sizeof(*cfg), GFP_KERNEL);
    if (cfg == NULL) {
        AUDIO_DRIVER_LOG_ERR("Cannot allocate memory for %s.\n", pn);
        return -ENOMEM;
    }

    for (i = 0, index = 0; i < num_cfg; i++) {
        ret = of_property_read_u32_index(np, pn, index++, &pins);
        if (ret) {
            pins = 0;
        }

        ret = of_property_read_u32_index(np, pn, index++, &rx);
        if (ret) {
            rx = 1;
        }

        ret = of_property_read_u32_index(np, pn, index++, &tx);
        if (ret) {
            tx = 1;
        }

        if ((rx & ~soc_dl) || (tx & ~soc_dl)) {
            AUDIO_DRIVER_LOG_ERR(
                "%s: dataline cfg[%d] setting error, mask is 0x%x\n",
                pn, i, soc_dl);
            return -EINVAL;
        }

        cfg[i].pins = pins;
        cfg[i].mask[0] = rx;
        cfg[i].offset[0] = fsl_sai_calc_dl_off(rx);
        cfg[i].mask[1] = tx;
        cfg[i].offset[1] = fsl_sai_calc_dl_off(tx);
    }

    *rcfg = cfg;
    return num_cfg;
}

static int fsl_sai_check_ver(struct fsl_sai *sai, struct device *dev)
{
    unsigned char offset = sai->reg_offset;
    unsigned int val = 0;
    int ret = 0;

    if (FSL_SAI_TCSR(offset) == FSL_SAI_VERID) {
        return 0;
    }

    if (sai->verid.loaded) {
        return 0;
    }

    ret = regmap_read(sai->regmap, FSL_SAI_VERID, &val);
    if (ret < 0) {
        return ret;
    }

    AUDIO_DRIVER_LOG_ERR("VERID: 0x%016X\n", val);

    sai->verid.id = (val & FSL_SAI_VER_ID_MASK) >> FSL_SAI_VER_ID_SHIFT;
    sai->verid.extfifo_en = (val & FSL_SAI_VER_EFIFO_EN);
    sai->verid.timestamp_en = (val & FSL_SAI_VER_TSTMP_EN);

    ret = regmap_read(sai->regmap, FSL_SAI_PARAM, &val);
    if (ret < 0) {
        return ret;
    }

    AUDIO_DRIVER_LOG_ERR("PARAM: 0x%016X\n", val);

    /* max slots per frame, power of 2 */
    sai->param.spf = 1 <<
        ((val & FSL_SAI_PAR_SPF_MASK) >> FSL_SAI_PAR_SPF_SHIFT);

    /* words per fifo, power of 2 */
    sai->param.wpf = 1 <<
        ((val & FSL_SAI_PAR_WPF_MASK) >> FSL_SAI_PAR_WPF_SHIFT);

    /* number of datalines implemented */
    sai->param.dln = val & FSL_SAI_PAR_DLN_MASK;

    AUDIO_DRIVER_LOG_ERR(
        "Version: 0x%08X, SPF: %u, WPF: %u, DLN: %u\n",
        sai->verid.id, sai->param.spf, sai->param.wpf, sai->param.dln);

    sai->verid.loaded = true;

    return HDF_SUCCESS;
}

#define RATIO_1  (1)
#define RATIO_2  (2)
#define RATIO_512  (512)
#define RATE_1000  (1000)
static int fsl_sai_set_bclk(struct fsl_sai *sai, bool tx, u32 freq)
{
    unsigned char offset = sai->reg_offset;
    unsigned long clk_rate = 0;
    unsigned int reg = 0;
    u32 ratio = 0, savesub = freq, saveratio = 0, savediv = 0;
    u32 id = 0;
    int ret = 0;

    /* Don't apply to slave mode */
    if (sai->slave_mode[tx]) {
        return 0;
    }

    for (id = 0; id < FSL_SAI_MCLK_MAX; id++) {
        clk_rate = clk_get_rate(sai->mclk_clk[id]);
        if (!clk_rate) {
            continue;
        }

        if (freq != 0) {
            ratio = clk_rate / freq;
        }

        ret = clk_rate - ratio * freq;

        AUDIO_DRIVER_LOG_ERR("ratio %d for freq %dHz based on clock %ldHz\n",
                             ratio, freq, clk_rate);
        /*
         * Drop the source that can not be
         * divided into the required rate.
         */
        if (ret != 0 && clk_rate / ret < RATE_1000) {
            continue;
        }

        AUDIO_DRIVER_LOG_ERR("ratio %d for freq %dHz based on clock %ldHz\n",
                             ratio, freq, clk_rate);

        if ((ratio % RATIO_2 == 0 && ratio >= RATIO_2 && ratio <= RATIO_512) ||
            (ratio == RATIO_1 && sai->verid.id >= FSL_SAI_VERID_0301)) {
            if (ret < savesub) {
                saveratio = ratio;
                sai->mclk_id[tx] = id;
                savesub = ret;
            }

            if (ret == 0) {
                break;
            }
        }
    }

    if (saveratio == 0) {
        AUDIO_DRIVER_LOG_ERR("failed to derive required %cx rate: %d\n",
                             tx ? 'T' : 'R', freq);
        return HDF_FAILURE;
    }

    /*
     * 1) For Asynchronous mode, we must set RCR2 register for capture, and
     *    set TCR2 register for playback.
     * 2) For Tx sync with Rx clock, we must set RCR2 register for playback
     *    and capture.
     * 3) For Rx sync with Tx clock, we must set TCR2 register for playback
     *    and capture.
     * 4) For Tx and Rx are both Synchronous with another SAI, we just
     *    ignore it.
     */
    if ((!tx || sai->synchronous[TX]) && !sai->synchronous[RX]) {
        reg = FSL_SAI_RCR2(offset);
    } else if ((tx || sai->synchronous[RX]) && !sai->synchronous[TX]) {
        reg = FSL_SAI_TCR2(offset);
    }

    if (reg) {
        regmap_update_bits(sai->regmap, reg, FSL_SAI_CR2_MSEL_MASK,
            FSL_SAI_CR2_MSEL(sai->mclk_id[tx]));

        savediv = (saveratio == 1 ? 0 : (saveratio >> 1) - 1);
        regmap_update_bits(sai->regmap, reg, FSL_SAI_CR2_DIV_MASK, savediv);

        if (sai->verid.id >= FSL_SAI_VERID_0301) {
            regmap_update_bits(sai->regmap, reg, FSL_SAI_CR2_BYP,
                (saveratio == 1 ? FSL_SAI_CR2_BYP : 0));
        }
    }

    if (sai->verid.id >= FSL_SAI_VERID_0301) {
        /* SAI is in master mode at this point, so enable MCLK */
        regmap_update_bits(sai->regmap, FSL_SAI_MCTL,
            FSL_SAI_MCTL_MCLK_EN, FSL_SAI_MCTL_MCLK_EN);
    }

    AUDIO_DRIVER_LOG_ERR("best fit: clock id=%d, ratio=%d, deviation=%d\n",
                         sai->mclk_id[tx], saveratio, savesub);

    return 0;
}

int32_t SaiSetHwParams(const struct PlatformData *pd, const enum AudioStreamType streamType)
{
    struct fsl_sai *sai = GetDrvSai(pd);
    unsigned char offset = sai->reg_offset;
    u32 channels = 0;
    u32 rate = 0;
    u32 word_width = 0;
    bool tx = streamType == AUDIO_RENDER_STREAM;

    if (streamType == AUDIO_RENDER_STREAM) {
        channels = pd->renderPcmInfo.channels;
        rate = pd->renderPcmInfo.rate;
        word_width = pd->renderPcmInfo.bitWidth;
    } else if (streamType == AUDIO_CAPTURE_STREAM) {
        channels = pd->capturePcmInfo.channels;
        rate = pd->capturePcmInfo.rate;
        word_width = pd->capturePcmInfo.bitWidth;
    } else {
        AUDIO_DRIVER_LOG_ERR("streamType is invaild ");
        return HDF_FAILURE;
    }

    u32 val_cr4 = 0, val_cr5 = 0;
    u32 slots = (channels == 1) ? 2 : channels;
    u32 slot_width = word_width;
    u32 pins = 0, bclk = 0;
    int32_t ret = 0, i = 0, trce_mask = 0, dl_cfg_cnt, dl_cfg_idx = 0;
    struct fsl_sai_dl_cfg *dl_cfg = NULL;

    if (sai->slots) {
        slots = sai->slots;
    }

    pins = DIV_ROUND_UP(channels, slots);
    sai->is_dsd = false;
    dl_cfg = sai->pcm_dl_cfg;
    dl_cfg_cnt = sai->pcm_dl_cfg_cnt;
    for (i = 0; i < dl_cfg_cnt; i++) {
        if (dl_cfg[i].pins == pins) {
            dl_cfg_idx = i;
            break;
        }
    }

    if (dl_cfg_idx >= dl_cfg_cnt) {
        AUDIO_DRIVER_LOG_ERR("fsl,dataline%s invalid or not provided.",
                             sai->is_dsd ? ",dsd" : "");
        return HDF_FAILURE;
    }

    if (sai->slot_width) {
        slot_width = sai->slot_width;
    }

    AUDIO_DRIVER_LOG_ERR("rate = %d, slots = %d, slot_width = %d", rate, slots, slot_width);

    bclk = rate*(sai->bitclk_ratio ? sai->bitclk_ratio : slots * slot_width);

    if (!IS_ERR_OR_NULL(sai->pinctrl)) {
        AUDIO_DRIVER_LOG_ERR("pin cntrl");
        sai->pins_state = pinctrl_lookup_state(sai->pinctrl, "default");

        if (!IS_ERR_OR_NULL(sai->pins_state)) {
            AUDIO_DRIVER_LOG_ERR("pin cntrl");
            ret = pinctrl_select_state(sai->pinctrl, sai->pins_state);
            if (ret) {
                AUDIO_DRIVER_LOG_ERR("failed to set proper pins state");
                return ret;
            }
        }
    }

    if (!sai->slave_mode[tx]) {
        ret = fsl_sai_set_bclk(sai, tx, bclk);
        if (ret) {
            return ret;
        }

        /* Do not enable the clock if it is already enabled */
        if (!(sai->mclk_streams & BIT(streamType))) {
            AUDIO_DRIVER_LOG_ERR("mclk enable %d", sai->mclk_id[tx]);
            ret = clk_prepare_enable(sai->mclk_clk[sai->mclk_id[tx]]);
            if (ret) {
                return ret;
            }

            sai->mclk_streams |= BIT(streamType);
        }
    }

    if (!sai->is_dsp_mode) {
        val_cr4 |= FSL_SAI_CR4_SYWD(slot_width);
    }

    val_cr5 |= FSL_SAI_CR5_WNW(slot_width);
    val_cr5 |= FSL_SAI_CR5_W0W(slot_width);

    if (sai->is_lsb_first) {
        val_cr5 |= FSL_SAI_CR5_FBT(0);
    } else {
        val_cr5 |= FSL_SAI_CR5_FBT(word_width - 1);
    }

    val_cr4 |= FSL_SAI_CR4_FRSZ(slots);

    /* Output Mode - data pins transmit 0 when slots are masked
     * or channels are disabled
     */
    val_cr4 |= FSL_SAI_CR4_CHMOD;

    /*
     * For SAI master mode, when Tx(Rx) sync with Rx(Tx) clock, Rx(Tx) will
     * generate bclk and frame clock for Tx(Rx), we should set RCR4(TCR4),
     * RCR5(TCR5) and RMR(TMR) for playback(capture), or there will be sync
     * error.
     */

    if (!sai->slave_mode[tx]) {
        if (!sai->synchronous[TX] && sai->synchronous[RX] && !tx) {
            regmap_update_bits(sai->regmap, FSL_SAI_TCR4(offset),
                FSL_SAI_CR4_SYWD_MASK | FSL_SAI_CR4_FRSZ_MASK |
                FSL_SAI_CR4_CHMOD_MASK,
                val_cr4);
            regmap_update_bits(sai->regmap, FSL_SAI_TCR5(offset),
                FSL_SAI_CR5_WNW_MASK | FSL_SAI_CR5_W0W_MASK |
                FSL_SAI_CR5_FBT_MASK, val_cr5);
        } else if (!sai->synchronous[RX] && sai->synchronous[TX] && tx) {
            regmap_update_bits(sai->regmap, FSL_SAI_RCR4(offset),
                FSL_SAI_CR4_SYWD_MASK | FSL_SAI_CR4_FRSZ_MASK |
                FSL_SAI_CR4_CHMOD_MASK,
                val_cr4);
            regmap_update_bits(sai->regmap, FSL_SAI_RCR5(offset),
                FSL_SAI_CR5_WNW_MASK | FSL_SAI_CR5_W0W_MASK |
                FSL_SAI_CR5_FBT_MASK, val_cr5);
        }
    }

    if (sai->dataline != 0x1) {
        if (dl_cfg[dl_cfg_idx].mask[tx] <= 1 || sai->is_multi_lane) {
            regmap_update_bits(sai->regmap, FSL_SAI_xCR4(tx, offset),
                FSL_SAI_CR4_FCOMB_MASK, 0);
        } else {
            regmap_update_bits(sai->regmap, FSL_SAI_xCR4(tx, offset),
                FSL_SAI_CR4_FCOMB_MASK, FSL_SAI_CR4_FCOMB_SOFT);
        }
    }

    if (__sw_hweight8(dl_cfg[dl_cfg_idx].mask[tx] & 0xFF) < pins) {
        AUDIO_DRIVER_LOG_ERR("channel not supported");
        return -EINVAL;
    }

    /* find a proper tcre setting */
    for (i = 0; i < 8; i++) {
        trce_mask = (1 << (i + 1)) - 1;
        if (__sw_hweight8(dl_cfg[dl_cfg_idx].mask[tx] & trce_mask) == pins) {
            break;
        }
    }

    regmap_update_bits(sai->regmap, FSL_SAI_xCR3(tx, offset),
        FSL_SAI_CR3_TRCE_MASK,
        FSL_SAI_CR3_TRCE((dl_cfg[dl_cfg_idx].mask[tx] & trce_mask)));

    regmap_update_bits(sai->regmap, FSL_SAI_xCR4(tx, offset),
        FSL_SAI_CR4_SYWD_MASK | FSL_SAI_CR4_FRSZ_MASK |
        FSL_SAI_CR4_CHMOD_MASK,
        val_cr4);

    regmap_update_bits(sai->regmap, FSL_SAI_xCR5(tx, offset),
        FSL_SAI_CR5_WNW_MASK | FSL_SAI_CR5_W0W_MASK |
        FSL_SAI_CR5_FBT_MASK, val_cr5);

    regmap_write(sai->regmap, FSL_SAI_xMR(tx),
        ~0UL - ((1 << min(channels, slots)) - 1));

    return HDF_SUCCESS;
}

int32_t SaiSetDaiTdmSlot(const struct PlatformData *pd, int32_t tx_mask,
                         int32_t rx_mask, int32_t slots, int32_t slot_width)
{
    struct fsl_sai *sai = GetDrvSai(pd);

    if (sai != NULL) {
        sai->slots = slots;
        sai->slot_width = slot_width;
    } else {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t SaiSetSysclkTr(struct fsl_sai *sai, int clk_id, unsigned int freq, int fsl_dir)
{
    unsigned char offset = 0;
    bool tx = fsl_dir == FSL_FMT_TRANSMITTER;
    uint32_t val_cr2 = 0;

    offset = sai->reg_offset;
    switch (clk_id) {
        case FSL_SAI_CLK_BUS:
            val_cr2 |= FSL_SAI_CR2_MSEL_BUS;
            break;
        case FSL_SAI_CLK_MAST1:
            val_cr2 |= FSL_SAI_CR2_MSEL_MCLK1;
            break;
        case FSL_SAI_CLK_MAST2:
            val_cr2 |= FSL_SAI_CR2_MSEL_MCLK2;
            break;
        case FSL_SAI_CLK_MAST3:
            val_cr2 |= FSL_SAI_CR2_MSEL_MCLK3;
            break;
        default:
            return -EINVAL;
    }

    regmap_update_bits(sai->regmap, FSL_SAI_xCR2(tx, offset),
                       FSL_SAI_CR2_MSEL_MASK, val_cr2);

    return HDF_SUCCESS;
}

static int32_t fsl_sai_set_mclk_rate(struct fsl_sai *sai, int clk_id, unsigned int freq)
{
    struct clk *p = sai->mclk_clk[clk_id], *pll = 0, *npll = 0;
    u64 ratio = freq;
    int ret = 0;
    AUDIO_DRIVER_LOG_ERR("mclk rate in");
    while (p && sai->pll8k_clk && sai->pll11k_clk) {
        struct clk *pp = clk_get_parent(p);

        if (clk_is_match(pp, sai->pll8k_clk) ||
            clk_is_match(pp, sai->pll11k_clk)) {
            pll = pp;
            break;
        }
        p = pp;
    }

    if (pll) {
        npll = (do_div(ratio, 8000) ? sai->pll11k_clk : sai->pll8k_clk);
        if (!clk_is_match(pll, npll)) {
            ret = clk_set_parent(p, npll);
            if (ret < 0) {
                AUDIO_DRIVER_LOG_ERR("failed to set parent %s: %d\n",
                                     __clk_get_name(npll), ret);
            }
        }
    }

    ret = clk_set_rate(sai->mclk_clk[clk_id], freq);
    AUDIO_DRIVER_LOG_ERR("mclk_clk id %d freq %d", clk_id, freq);
    if (ret < 0) {
        AUDIO_DRIVER_LOG_ERR("failed to set clock rate (%u): %d\n",
                             freq, ret);
    }

    return ret;
}

int32_t SaiSetSysclk(const struct PlatformData *pd, int clk_id, unsigned int freq, int dir)
{
    struct fsl_sai *sai = GetDrvSai(pd);
    int32_t ret = 0;

    if (sai == NULL) {
        AUDIO_DRIVER_LOG_ERR("sai null");
        return HDF_FAILURE;
    }

    if (dir == SOC_CLOCK_IN) {
        return HDF_SUCCESS;
    }

    if (freq > 0 && clk_id != FSL_SAI_CLK_BUS) {
        if (clk_id < 0 || clk_id >= FSL_SAI_MCLK_MAX) {
            AUDIO_DRIVER_LOG_ERR("Unknown clock id: %d", clk_id);
            return -EINVAL;
        }

        if (IS_ERR_OR_NULL(sai->mclk_clk[clk_id])) {
            AUDIO_DRIVER_LOG_ERR("Unassigned clock: %d", clk_id);
            return -EINVAL;
        }
        AUDIO_DRIVER_LOG_ERR("clock: %d", clk_id);
        if (sai->mclk_streams == 0) {
            AUDIO_DRIVER_LOG_ERR("mclk rate = %d", freq);
            ret = fsl_sai_set_mclk_rate(sai, clk_id, freq);
            if (ret < 0) {
                return ret;
            }
        }
    }

    ret = SaiSetSysclkTr(sai, clk_id, freq, FSL_FMT_TRANSMITTER);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("cannot set tx sysclk");
        return HDF_FAILURE;
    }

    ret = SaiSetSysclkTr(sai, clk_id, freq, FSL_FMT_RECEIVER);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("cannot set rx sysclk");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t SaiSetDaiFmtTr(struct PrivPlatformData *ppd, unsigned int fmt, int fsl_dir)
{
    struct fsl_sai *sai = NULL;
    unsigned char offset = 0;
    int32_t val_cr2 = 0;
    int32_t val_cr4 = 0;
    bool tx = fsl_dir == FSL_FMT_TRANSMITTER;
    uint32_t read_val;

    if (ppd == NULL) {
        AUDIO_DRIVER_LOG_ERR("pdd is null");
        return HDF_FAILURE;
    }

    sai = &ppd->sai;
    offset = sai->reg_offset;

    if (!sai->is_lsb_first) {
        val_cr4 |= FSL_SAI_CR4_MF;
    }

    sai->is_dsp_mode = false;

    /* DAI mode */
    switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
        case SND_SOC_DAIFMT_I2S:
            /*
            * Frame low, 1clk before data, one word length for frame sync,
            * frame sync starts one serial clock cycle earlier,
            * that is, together with the last bit of the previous
            * data word.
            */
            val_cr2 |= FSL_SAI_CR2_BCP;
            val_cr4 |= FSL_SAI_CR4_FSE | FSL_SAI_CR4_FSP;
            break;
        case SND_SOC_DAIFMT_LEFT_J:
            /*
            * Frame high, one word length for frame sync,
            * frame sync asserts with the first bit of the frame.
            */
            val_cr2 |= FSL_SAI_CR2_BCP;
            break;
        case SND_SOC_DAIFMT_DSP_A:
            /*
            * Frame high, 1clk before data, one bit for frame sync,
            * frame sync starts one serial clock cycle earlier,
            * that is, together with the last bit of the previous
            * data word.
            */
            val_cr2 |= FSL_SAI_CR2_BCP;
            val_cr4 |= FSL_SAI_CR4_FSE;
            sai->is_dsp_mode = true;
            break;
        case SND_SOC_DAIFMT_DSP_B:
            /*
            * Frame high, one bit for frame sync,
            * frame sync asserts with the first bit of the frame.
            */
            val_cr2 |= FSL_SAI_CR2_BCP;
            sai->is_dsp_mode = true;
            break;
        case SND_SOC_DAIFMT_PDM:
            val_cr2 |= FSL_SAI_CR2_BCP;
            val_cr4 &= ~FSL_SAI_CR4_MF;
            sai->is_dsp_mode = true;
            break;
        case SND_SOC_DAIFMT_RIGHT_J:
            /* To be done */
        default:
            return -EINVAL;
    }

    /* DAI clock inversion */
    switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
        case SND_SOC_DAIFMT_IB_IF:
            /* Invert both clocks */
            val_cr2 ^= FSL_SAI_CR2_BCP;
            val_cr4 ^= FSL_SAI_CR4_FSP;
            break;
        case SND_SOC_DAIFMT_IB_NF:
            /* Invert bit clock */
            val_cr2 ^= FSL_SAI_CR2_BCP;
            break;
        case SND_SOC_DAIFMT_NB_IF:
            /* Invert frame clock */
            val_cr4 ^= FSL_SAI_CR4_FSP;
            break;
        case SND_SOC_DAIFMT_NB_NF:
            /* Nothing to do for both normal cases */
            break;
        default:
            return -EINVAL;
    }

    sai->slave_mode[tx] = false;

    /* DAI clock master masks */
    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
        case SND_SOC_DAIFMT_CBS_CFS:
            val_cr2 |= FSL_SAI_CR2_BCD_MSTR;
            val_cr4 |= FSL_SAI_CR4_FSD_MSTR;
            break;
        case SND_SOC_DAIFMT_CBM_CFM:
            sai->slave_mode[tx] = true;
            break;
        case SND_SOC_DAIFMT_CBS_CFM:
            val_cr2 |= FSL_SAI_CR2_BCD_MSTR;
            break;
        case SND_SOC_DAIFMT_CBM_CFS:
            val_cr4 |= FSL_SAI_CR4_FSD_MSTR;
            sai->slave_mode[tx] = true;
            break;
        default:
            return -EINVAL;
    }

    regmap_read(sai->regmap, FSL_SAI_xCR2(tx, offset), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_xCR2 %08x", read_val);

    regmap_read(sai->regmap, FSL_SAI_xCR4(tx, offset), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_xCR4 %08x", read_val);

    regmap_update_bits(sai->regmap, FSL_SAI_xCR2(tx, offset),
                       FSL_SAI_CR2_BCP | FSL_SAI_CR2_BCD_MSTR, val_cr2);
    regmap_update_bits(sai->regmap, FSL_SAI_xCR4(tx, offset),
                       FSL_SAI_CR4_MF | FSL_SAI_CR4_FSE |
                       FSL_SAI_CR4_FSP | FSL_SAI_CR4_FSD_MSTR, val_cr4);

    regmap_read(sai->regmap, FSL_SAI_xCR2(tx, offset), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_xCR2 %08x", read_val);

    regmap_read(sai->regmap, FSL_SAI_xCR4(tx, offset), &read_val);
    AUDIO_DRIVER_LOG_ERR("FSL_SAI_xCR4 %08x", read_val);

    return HDF_SUCCESS;
}

int32_t SaiSetDaiFmt(const struct PlatformData *pd, unsigned int fmt)
{
    struct PrivPlatformData *ppd = (struct PrivPlatformData *)pd->dmaPrv;
    int32_t ret;

    ret = SaiSetDaiFmtTr(ppd, fmt, FSL_FMT_TRANSMITTER);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("set dai fmt tx failed");
        return ret;
    }

    ret = SaiSetDaiFmtTr(ppd, fmt, FSL_FMT_RECEIVER);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("set dai fmt tx failed");
    }

    return ret;
}

int32_t SaiHwFree(const struct DaiData *pd, int isTx)
{
    struct fsl_sai *sai = g_sai;
    unsigned char offset = sai->reg_offset;
    bool tx = isTx == TRIGGER_TX;
    regmap_update_bits(sai->regmap, FSL_SAI_xCR3(tx, offset),
                       FSL_SAI_CR3_TRCE_MASK, 0);

    if (sai->mclk_streams != 0) {
        clk_disable_unprepare(sai->mclk_clk[sai->mclk_id[tx]]);
        if (sai->mclk_streams & BIT(AUDIO_RENDER_STREAM)) {
            sai->mclk_streams &= ~BIT(AUDIO_RENDER_STREAM);
        } else if (sai->mclk_streams & BIT(AUDIO_CAPTURE_STREAM)) {
            sai->mclk_streams &= ~BIT(AUDIO_CAPTURE_STREAM);
        }
    }

    return HDF_SUCCESS;
}

int32_t SaiDriverInit(struct PlatformData *pd)
{
    struct PrivPlatformData *ppd = (struct PrivPlatformData *)pd->dmaPrv;
    struct device_node *np = NULL;
    struct resource *res = NULL;
    struct fsl_sai *sai = NULL;
    void __iomem *base = NULL;
    char tmp[8];
    int irq = 0, ret = 0, i = 0;
    unsigned long clk_rate = 0;
    int ret = 0;

    sai = &ppd->sai;
    g_sai = sai;

    np = ppd->pdev->dev.of_node;

    ret = of_clk_set_defaults(np, false);
    if (ret < 0) {
        AUDIO_DRIVER_LOG_ERR("sai driver init clk set default failed");
        return HDF_FAILURE;
    }

    sai->is_lsb_first = of_property_read_bool(np, "lsb-first");

    res = platform_get_resource(ppd->pdev, IORESOURCE_MEM, 0);
    base = devm_ioremap_resource(&ppd->pdev->dev, res);
    if (IS_ERR(base)) {
        return PTR_ERR(base);
    }

    sai->reg_offset = FSL_REG_OFFSET;

    sai->regmap = devm_regmap_init_mmio_clk(&ppd->pdev->dev, "bus", base,
                                            &fsl_sai_regmap_config);

    if (IS_ERR(sai->regmap) && PTR_ERR(sai->regmap) != -EPROBE_DEFER) {
        sai->regmap = devm_regmap_init_mmio_clk(&ppd->pdev->dev,
            "sai", base, &fsl_sai_regmap_config);
    }
    if (IS_ERR(sai->regmap)) {
        AUDIO_DRIVER_LOG_ERR("regmap init failed\n");
        return PTR_ERR(sai->regmap);
    }

    sai->bus_clk = devm_clk_get(&ppd->pdev->dev, "bus");

    if (IS_ERR(sai->bus_clk)) {
        AUDIO_DRIVER_LOG_ERR("failed to get bus clk %d", PTR_ERR(sai->bus_clk));
        sai->bus_clk = NULL;
    }

    clk_rate = clk_get_rate(sai->bus_clk);
    AUDIO_DRIVER_LOG_ERR("bus rate %d", clk_rate);

    for (i = 0; i < FSL_SAI_MCLK_MAX; i++) {
        ret = snprintf_s(tmp, sizeof(tmp)-1, sizeof(tmp)-1, "mclk%d", i);
        if (ret < 0) {
            AUDIO_DRIVER_LOG_ERR("failed to write mclk");
        }
        sai->mclk_clk[i] = devm_clk_get(&ppd->pdev->dev, tmp);
        if (IS_ERR(sai->mclk_clk[i])) {
            AUDIO_DRIVER_LOG_ERR("failed to get mclk%d clock: %ld\n",
                i, PTR_ERR(sai->mclk_clk[i]));
            sai->mclk_clk[i] = NULL;
        }
        clk_rate = clk_get_rate(sai->mclk_clk[i]);
        AUDIO_DRIVER_LOG_ERR("mclk %d rate %d", i, clk_rate);
    }

    sai->pll8k_clk = devm_clk_get(&ppd->pdev->dev, "pll8k");
    if (IS_ERR(sai->pll8k_clk)) {
        sai->pll8k_clk = NULL;
    }

    sai->pll11k_clk = devm_clk_get(&ppd->pdev->dev, "pll11k");
    if (IS_ERR(sai->pll11k_clk)) {
        sai->pll11k_clk = NULL;
    }

    sai->reg_offset = FSL_REG_OFFSET;
    sai->dataline = 0xFF;
    sai->fifo_depth = 128;

    ret = fsl_sai_read_dlcfg(ppd->pdev, "fsl,dataline", &sai->pcm_dl_cfg,
                             sai->dataline);
    if (ret < 0) {
        return ret;
    }

    sai->pcm_dl_cfg_cnt = ret;

    ret = fsl_sai_read_dlcfg(ppd->pdev, "fsl,dataline,dsd", &sai->dsd_dl_cfg,
                             sai->dataline);
    if (ret < 0) {
        return ret;
    }

    sai->dsd_dl_cfg_cnt = ret;

    irq = platform_get_irq(ppd->pdev, 0);
    if (irq < 0) {
        AUDIO_DRIVER_LOG_ERR("failed to get irq");
        return irq;
    }

    ret = devm_request_irq(&ppd->pdev->dev, irq, fsl_sai_isr, 0,
                           np->name, ppd);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("failed to claim irq");
        return ret;
    }

    sai->synchronous[RX] = true;
    sai->synchronous[TX] = false;

    if (of_find_property(np, "fsl,sai-synchronous-rx", NULL) &&
        of_find_property(np, "fsl,sai-asynchronous", NULL)) {
        /* error out if both synchronous and asynchronous are present */
        AUDIO_DRIVER_LOG_ERR("invalid binding for synchronous mode\n");
        return -EINVAL;
    }

    if (of_find_property(np, "fsl,sai-synchronous-rx", NULL)) {
        /* Sync Rx with Tx */
        sai->synchronous[RX] = false;
        sai->synchronous[TX] = true;
    } else if (of_find_property(np, "fsl,sai-asynchronous", NULL)) {
        /* Discard all settings for asynchronous mode */
        sai->synchronous[RX] = false;
        sai->synchronous[TX] = false;
    }

    platform_set_drvdata(ppd->pdev, sai);
    ret = fsl_sai_check_ver(sai, &ppd->pdev->dev);
    if (ret < 0) {
        AUDIO_DRIVER_LOG_ERR("Error reading SAI version: %d", ret);
    }

    if (of_find_property(np, "fsl,sai-mclk-direction-output", NULL) &&
        sai->verid.id >= FSL_SAI_VERID_0301) {
        regmap_update_bits(sai->regmap, FSL_SAI_MCTL,
            FSL_SAI_MCTL_MCLK_EN, FSL_SAI_MCTL_MCLK_EN);
    }

    ppd->dma_addr_src = res->start + FSL_SAI_RDR0;
    ppd->dma_addr_dst = res->start + FSL_SAI_TDR0;
    ppd->dma_maxburst_rx = FSL_SAI_MAXBURST_RX;
    ppd->dma_maxburst_tx = FSL_SAI_MAXBURST_TX;

    sai->pinctrl = devm_pinctrl_get(&ppd->pdev->dev);
    regcache_cache_only(sai->regmap, true);

    pm_runtime_enable(&ppd->pdev->dev);

    AUDIO_DRIVER_LOG_ERR("success probe in sai driver");

    return HDF_SUCCESS;
}

int32_t SaiTrigger(const struct DaiData *pd, int cmd, int isTx)
{
    struct fsl_sai *sai = g_sai;
    unsigned char offset = sai->reg_offset;
    u8 channels = 0;
    bool tx = isTx == TRIGGER_TX;
    if (isTx == TRIGGER_TX) {
        channels = pd->pcmInfo.channels;
    } else if (isTx == TRIGGER_RX) {
        channels = pd->pcmInfo.channels;
    }

    u32 slots = (channels == 1) ? 2 : channels;
    u32 xcsr = 0, count = 100;
    u32 pins = 0;
    int i = 0, j = 0, k = 0, dl_cfg_cnt = 0, dl_cfg_idx = 0;
    struct fsl_sai_dl_cfg *dl_cfg;

    if (sai->slots) {
        slots = sai->slots;
    }

    pins = DIV_ROUND_UP(channels, slots);
    sai->is_dsd = false;

    dl_cfg = sai->pcm_dl_cfg;
    dl_cfg_cnt = sai->pcm_dl_cfg_cnt;

    for (i = 0; i < dl_cfg_cnt; i++) {
        if (dl_cfg[i].pins == pins) {
            dl_cfg_idx = i;
            break;
        }
    }

    i = 0;
    if (cmd == TRIGGER_START) {
        /*
         * Asynchronous mode: Clear SYNC for both Tx and Rx.
         * Rx sync with Tx clocks: Clear SYNC for Tx, set it for Rx.
         * Tx sync with Rx clocks: Clear SYNC for Rx, set it for Tx.
         */
        regmap_update_bits(sai->regmap, FSL_SAI_TCR2(offset), FSL_SAI_CR2_SYNC,
                           sai->synchronous[TX] ? FSL_SAI_CR2_SYNC : 0);
        regmap_update_bits(sai->regmap, FSL_SAI_RCR2(offset), FSL_SAI_CR2_SYNC,
                           sai->synchronous[RX] ? FSL_SAI_CR2_SYNC : 0);

        while (tx && i < channels) {
            if (dl_cfg[dl_cfg_idx].mask[tx] & (1 << j)) {
                regmap_write(sai->regmap, FSL_SAI_TDR0 + j * 0x4, 0x0);
                i++;
                k++;
            }
            j++;

            if ((pins != 0) && (k % pins) == 0) {
                j = 0;
            }
        }

        regmap_update_bits(sai->regmap, FSL_SAI_xCSR(tx, offset),
                           FSL_SAI_CSR_FRDE, FSL_SAI_CSR_FRDE);

        regmap_update_bits(sai->regmap, FSL_SAI_xCSR(tx, offset),
                           FSL_SAI_CSR_TERE, FSL_SAI_CSR_TERE);
        regmap_update_bits(sai->regmap, FSL_SAI_xCSR(tx, offset),
                           FSL_SAI_CSR_SE, FSL_SAI_CSR_SE);
        if (!sai->synchronous[TX] && sai->synchronous[RX] && !tx) {
            regmap_update_bits(sai->regmap, FSL_SAI_xCSR((!tx), offset),
                               FSL_SAI_CSR_TERE, FSL_SAI_CSR_TERE);
        } else if (!sai->synchronous[RX] && sai->synchronous[TX] && tx) {
            regmap_update_bits(sai->regmap, FSL_SAI_xCSR((!tx), offset),
                               FSL_SAI_CSR_TERE, FSL_SAI_CSR_TERE);
        }

        regmap_update_bits(sai->regmap, FSL_SAI_xCSR(tx, offset),
                           FSL_SAI_CSR_xIE_MASK, FSL_SAI_FLAGS);
    } else {
        regmap_update_bits(sai->regmap, FSL_SAI_xCSR(tx, offset),
            FSL_SAI_CSR_FRDE, 0);
        regmap_update_bits(sai->regmap, FSL_SAI_xCSR(tx, offset),
            FSL_SAI_CSR_xIE_MASK, 0);

        /* Check if the opposite FRDE is also disabled */
        regmap_read(sai->regmap, FSL_SAI_xCSR(!tx, offset), &xcsr);
        if (!(xcsr & FSL_SAI_CSR_FRDE)) {
            /* Disable both directions and reset their FIFOs */
            regmap_update_bits(sai->regmap, FSL_SAI_TCSR(offset),
                FSL_SAI_CSR_TERE, 0);
            regmap_update_bits(sai->regmap, FSL_SAI_RCSR(offset),
                FSL_SAI_CSR_TERE, 0);

            /* TERE will remain set till the end of current frame */
            do {
                udelay(10);
                regmap_read(sai->regmap, FSL_SAI_xCSR(tx, offset), &xcsr);
            } while (((--count) && (xcsr)) & (FSL_SAI_CSR_TERE));

            regmap_update_bits(sai->regmap, FSL_SAI_TCSR(offset),
                FSL_SAI_CSR_FR, FSL_SAI_CSR_FR);
            regmap_update_bits(sai->regmap, FSL_SAI_RCSR(offset),
                FSL_SAI_CSR_FR, FSL_SAI_CSR_FR);

            /*
             * For sai master mode, after several open/close sai,
             * there will be no frame clock, and can't recover
             * anymore. Add software reset to fix this issue.
             * This is a hardware bug, and will be fix in the
             * next sai version.
             */
            if (!sai->slave_mode[tx]) {
                /* Software Reset for both Tx and Rx */
                regmap_write(sai->regmap,
                    FSL_SAI_TCSR(offset), FSL_SAI_CSR_SR);
                regmap_write(sai->regmap,
                    FSL_SAI_RCSR(offset), FSL_SAI_CSR_SR);
                /* Clear SR bit to finish the reset */
                regmap_write(sai->regmap, FSL_SAI_TCSR(offset), 0);
                regmap_write(sai->regmap, FSL_SAI_RCSR(offset), 0);
            }
        }
    }

    SaiPrintAllRegister();

    return HDF_SUCCESS;
}

int32_t SaiRuntimeResume(const struct PlatformData *platformData)
{
    struct fsl_sai *sai = GetDrvSai(platformData);
    uint8_t offset = sai->reg_offset;
    int32_t ret;

    AUDIO_DRIVER_LOG_ERR("bus_clk enable");
    ret = clk_prepare_enable(sai->bus_clk);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("failed to enable bus clock: %d\n", ret);
        return ret;
    }

    if (sai->mclk_streams & BIT(AUDIO_RENDER_STREAM)) {
        AUDIO_DRIVER_LOG_ERR("mclk enable %d", sai->mclk_id[TX]);
        ret = clk_prepare_enable(sai->mclk_clk[sai->mclk_id[TX]]);
        if (ret) {
            clk_disable_unprepare(sai->bus_clk);
            return ret;
        }
    }

    if (sai->mclk_streams & BIT(AUDIO_CAPTURE_STREAM)) {
        AUDIO_DRIVER_LOG_ERR("mclk enable %d", sai->mclk_id[RX]);
        ret = clk_prepare_enable(sai->mclk_clk[sai->mclk_id[RX]]);
        if (ret) {
            if (sai->mclk_streams & BIT(AUDIO_RENDER_STREAM)) {
                clk_disable_unprepare(sai->mclk_clk[sai->mclk_id[1]]);
            }
            clk_disable_unprepare(sai->bus_clk);

            return ret;
        }
    }

    request_bus_freq(BUS_FREQ_AUDIO);

    regcache_cache_only(sai->regmap, false);
    regcache_mark_dirty(sai->regmap);

    regmap_write(sai->regmap, FSL_SAI_TCSR(offset), FSL_SAI_CSR_SR);
    regmap_write(sai->regmap, FSL_SAI_RCSR(offset), FSL_SAI_CSR_SR);
    usleep_range(1000, 2000);
    regmap_write(sai->regmap, FSL_SAI_TCSR(offset), 0);
    regmap_write(sai->regmap, FSL_SAI_RCSR(offset), 0);

    ret = regcache_sync(sai->regmap);
    if (ret) {
        if (sai->mclk_streams & BIT(AUDIO_CAPTURE_STREAM)) {
            clk_disable_unprepare(sai->mclk_clk[sai->mclk_id[0]]);
        }
        if (sai->mclk_streams & BIT(AUDIO_RENDER_STREAM)) {
            clk_disable_unprepare(sai->mclk_clk[sai->mclk_id[1]]);
        }
        clk_disable_unprepare(sai->bus_clk);

        return ret;
    }

    return HDF_SUCCESS;
}

int32_t SaiRuntimeSuspend(const struct DaiData *platformData)
{
    struct fsl_sai *sai = g_sai;

    regcache_cache_only(sai->regmap, true);

    release_bus_freq(BUS_FREQ_AUDIO);

    if (sai->mclk_streams & BIT(AUDIO_CAPTURE_STREAM)) {
        clk_disable_unprepare(sai->mclk_clk[sai->mclk_id[0]]);
    }

    if (sai->mclk_streams & BIT(AUDIO_RENDER_STREAM)) {
        clk_disable_unprepare(sai->mclk_clk[sai->mclk_id[1]]);
    }

    clk_disable_unprepare(sai->bus_clk);

    regcache_cache_only(sai->regmap, true);
    regcache_mark_dirty(sai->regmap);

    return HDF_SUCCESS;
}

int32_t SaiDaiProbe(const struct PlatformData *platformData)
{
    struct fsl_sai *sai = GetDrvSai(platformData);
    uint8_t offset = sai->reg_offset;
    int32_t ret = 0;

    AUDIO_DRIVER_LOG_ERR("sai dai probe");
    regmap_update_bits(sai->regmap, FSL_SAI_TCR1(offset),
                       sai->fifo_depth - 1,
                       sai->fifo_depth - FSL_SAI_MAXBURST_TX);
    regmap_update_bits(sai->regmap, FSL_SAI_RCR1(offset),
                       sai->fifo_depth - 1,
                       FSL_SAI_MAXBURST_RX - 1);

    if (!IS_ERR_OR_NULL(sai->pinctrl)) {
        sai->pins_state = pinctrl_lookup_state(sai->pinctrl, "default");
        if (!IS_ERR_OR_NULL(sai->pins_state)) {
            ret = pinctrl_select_state(sai->pinctrl, sai->pins_state);
            if (ret) {
                AUDIO_DRIVER_LOG_ERR("failed to set properpins state");
                return HDF_FAILURE;
            }
        }
    }

    return HDF_SUCCESS;
}

int32_t SaiWrite(const struct PlatformData *platformData, int i)
{
    struct fsl_sai *sai = GetDrvSai(platformData);

    regmap_update_bits(sai->regmap, FSL_SAI_TDR0, 0xFFFF, (i*0xFF+i));

    return 0;
}
