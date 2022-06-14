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

#ifndef SAI_DRIVER_H
#define SAI_DRIVER_H

#include "audio_platform_if.h"
#include "imx8mm_platform.h"

/*
 * DAI hardware audio formats.
 *
 * Describes the physical PCM data formating and clocking. Add new formats
 * to the end.
 */
#define SND_SOC_DAIFMT_I2S        (1) /* I2S mode */
#define SND_SOC_DAIFMT_RIGHT_J    (2) /* Right Justified mode */
#define SND_SOC_DAIFMT_LEFT_J     (3) /* Left Justified mode */
#define SND_SOC_DAIFMT_DSP_A      (4) /* L data MSB after FRM LRC */
#define SND_SOC_DAIFMT_DSP_B      (5) /* L data MSB during FRM LRC */
#define SND_SOC_DAIFMT_AC97       (6) /* AC97 */
#define SND_SOC_DAIFMT_PDM        (7) /* Pulse density modulation */

/* left and right justified also known as MSB and LSB respectively */
#define SND_SOC_DAIFMT_MSB        SND_SOC_DAIFMT_LEFT_J
#define SND_SOC_DAIFMT_LSB        SND_SOC_DAIFMT_RIGHT_J

/*
 * DAI hardware signal inversions.
 *
 * Specifies whether the DAI can also support inverted clocks for the specified
 * format.
 */
#define SND_SOC_DAIFMT_NB_NF    (1 << 8) /* normal bit clock + frame */
#define SND_SOC_DAIFMT_NB_IF    (2 << 8) /* normal BCLK + inv FRM */
#define SND_SOC_DAIFMT_IB_NF    (3 << 8) /* invert BCLK + nor FRM */
#define SND_SOC_DAIFMT_IB_IF    (4 << 8) /* invert BCLK + FRM */

/*
 * DAI hardware clock masters.
 *
 * This is wrt the codec, the inverse is true for the interface
 * i.e. if the codec is clk and FRM master then the interface is
 * clk and frame slave.
 */
#define SND_SOC_DAIFMT_CBM_CFM    (1 << 12) /* codec clk & FRM master */
#define SND_SOC_DAIFMT_CBS_CFM    (2 << 12) /* codec clk slave & FRM master */
#define SND_SOC_DAIFMT_CBM_CFS    (3 << 12) /* codec clk master & frame slave */
#define SND_SOC_DAIFMT_CBS_CFS    (4 << 12) /* codec clk & FRM slave */

#define SND_SOC_DAIFMT_FORMAT_MASK    (0x000f)
#define SND_SOC_DAIFMT_CLOCK_MASK     (0x00f0)
#define SND_SOC_DAIFMT_INV_MASK       (0x0f00)
#define SND_SOC_DAIFMT_MASTER_MASK    (0xf000)

#define FSL_SAI_FORMATS (SNDRV_PCM_FMTBIT_S16_LE |\
             SNDRV_PCM_FMTBIT_S24_LE |\
             SNDRV_PCM_FMTBIT_S32_LE |\
             SNDRV_PCM_FMTBIT_DSD_U8 |\
             SNDRV_PCM_FMTBIT_DSD_U16_LE |\
             SNDRV_PCM_FMTBIT_DSD_U32_LE)

/* SAI Register Map Register */
#define FSL_SAI_VERID        (0x00)          /* SAI Version ID Register */
#define FSL_SAI_PARAM        (0x04)          /* SAI Parameter Register */
#define FSL_SAI_TCSR(offset) (0x00 + offset) /* SAI Transmit Control */
#define FSL_SAI_TCR1(offset) (0x04 + offset) /* SAI Transmit Configuration 1 */
#define FSL_SAI_TCR2(offset) (0x08 + offset) /* SAI Transmit Configuration 2 */
#define FSL_SAI_TCR3(offset) (0x0c + offset) /* SAI Transmit Configuration 3 */
#define FSL_SAI_TCR4(offset) (0x10 + offset) /* SAI Transmit Configuration 4 */
#define FSL_SAI_TCR5(offset) (0x14 + offset) /* SAI Transmit Configuration 5 */
#define FSL_SAI_TDR0         (0x20)          /* SAI Transmit Data */
#define FSL_SAI_TDR1         (0x24)          /* SAI Transmit Data */
#define FSL_SAI_TDR2         (0x28)          /* SAI Transmit Data */
#define FSL_SAI_TDR3         (0x2C)          /* SAI Transmit Data */
#define FSL_SAI_TDR4         (0x30)          /* SAI Transmit Data */
#define FSL_SAI_TDR5         (0x34)          /* SAI Transmit Data */
#define FSL_SAI_TDR6         (0x38)          /* SAI Transmit Data */
#define FSL_SAI_TDR7         (0x3C)          /* SAI Transmit Data */
#define FSL_SAI_TFR0         (0x40)          /* SAI Transmit FIFO */
#define FSL_SAI_TFR1         (0x44)          /* SAI Transmit FIFO */
#define FSL_SAI_TFR2         (0x48)          /* SAI Transmit FIFO */
#define FSL_SAI_TFR3         (0x4C)          /* SAI Transmit FIFO */
#define FSL_SAI_TFR4         (0x50)          /* SAI Transmit FIFO */
#define FSL_SAI_TFR5         (0x54)          /* SAI Transmit FIFO */
#define FSL_SAI_TFR6         (0x58)          /* SAI Transmit FIFO */
#define FSL_SAI_TFR7         (0x5C)          /* SAI Transmit FIFO */
#define FSL_SAI_TMR          (0x60)          /* SAI Transmit Mask */
#define FSL_SAI_TTCTL        (0x70)          /* SAI Transmit Timestamp Control Register */
#define FSL_SAI_TTCTN        (0x74)          /* SAI Transmit Timestamp Counter Register */
#define FSL_SAI_TBCTN        (0x78)          /* SAI Transmit Bit Counter Register */
#define FSL_SAI_TTCAP        (0x7C)          /* SAI Transmit Timestamp Capture */

#define FSL_SAI_RCSR(offset) (0x80 + offset) /* SAI Receive Control */
#define FSL_SAI_RCR1(offset) (0x84 + offset) /* SAI Receive Configuration 1 */
#define FSL_SAI_RCR2(offset) (0x88 + offset) /* SAI Receive Configuration 2 */
#define FSL_SAI_RCR3(offset) (0x8c + offset) /* SAI Receive Configuration 3 */
#define FSL_SAI_RCR4(offset) (0x90 + offset) /* SAI Receive Configuration 4 */
#define FSL_SAI_RCR5(offset) (0x94 + offset) /* SAI Receive Configuration 5 */
#define FSL_SAI_RDR0         (0xa0)          /* SAI Receive Data */
#define FSL_SAI_RDR1         (0xa4)          /* SAI Receive Data */
#define FSL_SAI_RDR2         (0xa8)          /* SAI Receive Data */
#define FSL_SAI_RDR3         (0xac)          /* SAI Receive Data */
#define FSL_SAI_RDR4         (0xb0)          /* SAI Receive Data */
#define FSL_SAI_RDR5         (0xb4)          /* SAI Receive Data */
#define FSL_SAI_RDR6         (0xb8)          /* SAI Receive Data */
#define FSL_SAI_RDR7         (0xbc)          /* SAI Receive Data */
#define FSL_SAI_RFR0         (0xc0)          /* SAI Receive FIFO */
#define FSL_SAI_RFR1         (0xc4)          /* SAI Receive FIFO */
#define FSL_SAI_RFR2         (0xc8)          /* SAI Receive FIFO */
#define FSL_SAI_RFR3         (0xcc)          /* SAI Receive FIFO */
#define FSL_SAI_RFR4         (0xd0)          /* SAI Receive FIFO */
#define FSL_SAI_RFR5         (0xd4)          /* SAI Receive FIFO */
#define FSL_SAI_RFR6         (0xd8)          /* SAI Receive FIFO */
#define FSL_SAI_RFR7         (0xdc)          /* SAI Receive FIFO */
#define FSL_SAI_RMR          (0xe0)          /* SAI Receive Mask */
#define FSL_SAI_RTCTL        (0xf0)          /* SAI Receive Timestamp Control Register */
#define FSL_SAI_RTCTN        (0xf4)          /* SAI Receive Timestamp Counter Register */
#define FSL_SAI_RBCTN        (0xf8)          /* SAI Receive Bit Counter Register */
#define FSL_SAI_RTCAP        (0xfc)          /* SAI Receive Timestamp Capture */

#define FSL_SAI_MCTL         (0x100)         /* SAI MCLK Control Register */
#define FSL_SAI_MDIV         (0x104)         /* SAI MCLK Divide Register */

#define FSL_SAI_xCSR(tx, off)    (tx ? FSL_SAI_TCSR(off) : FSL_SAI_RCSR(off))
#define FSL_SAI_xCR1(tx, off)    (tx ? FSL_SAI_TCR1(off) : FSL_SAI_RCR1(off))
#define FSL_SAI_xCR2(tx, off)    (tx ? FSL_SAI_TCR2(off) : FSL_SAI_RCR2(off))
#define FSL_SAI_xCR3(tx, off)    (tx ? FSL_SAI_TCR3(off) : FSL_SAI_RCR3(off))
#define FSL_SAI_xCR4(tx, off)    (tx ? FSL_SAI_TCR4(off) : FSL_SAI_RCR4(off))
#define FSL_SAI_xCR5(tx, off)    (tx ? FSL_SAI_TCR5(off) : FSL_SAI_RCR5(off))
#define FSL_SAI_xMR(tx)          (tx ? FSL_SAI_TMR : FSL_SAI_RMR)

/* SAI Transmit/Receive Control Register */
#define FSL_SAI_CSR_TERE         BIT(31)
#define FSL_SAI_CSR_SE           BIT(30)
#define FSL_SAI_CSR_FR           BIT(25)
#define FSL_SAI_CSR_SR           BIT(24)
#define FSL_SAI_CSR_xF_SHIFT     (16)
#define FSL_SAI_CSR_xF_W_SHIFT   (18)
#define FSL_SAI_CSR_xF_MASK      (0x1f << FSL_SAI_CSR_xF_SHIFT)
#define FSL_SAI_CSR_xF_W_MASK    (0x7 << FSL_SAI_CSR_xF_W_SHIFT)
#define FSL_SAI_CSR_WSF          BIT(20)
#define FSL_SAI_CSR_SEF          BIT(19)
#define FSL_SAI_CSR_FEF          BIT(18)
#define FSL_SAI_CSR_FWF          BIT(17)
#define FSL_SAI_CSR_FRF          BIT(16)
#define FSL_SAI_CSR_xIE_SHIFT    (8)
#define FSL_SAI_CSR_xIE_MASK     (0x1f << FSL_SAI_CSR_xIE_SHIFT)
#define FSL_SAI_CSR_WSIE         BIT(12)
#define FSL_SAI_CSR_SEIE         BIT(11)
#define FSL_SAI_CSR_FEIE         BIT(10)
#define FSL_SAI_CSR_FWIE         BIT(9)
#define FSL_SAI_CSR_FRIE         BIT(8)
#define FSL_SAI_CSR_FRDE         BIT(0)

/* SAI Transmit and Receive Configuration 1 Register */
#define FSL_SAI_CR1_RFW_MASK     (0x1f)

/* SAI Transmit and Receive Configuration 2 Register */
#define FSL_SAI_CR2_SYNC         BIT(30)
#define FSL_SAI_CR2_MSEL_MASK    (0x3 << 26)
#define FSL_SAI_CR2_MSEL_BUS     (0)
#define FSL_SAI_CR2_MSEL_MCLK1   BIT(26)
#define FSL_SAI_CR2_MSEL_MCLK2   BIT(27)
#define FSL_SAI_CR2_MSEL_MCLK3   (BIT(26) | BIT(27))
#define FSL_SAI_CR2_MSEL(ID)     ((ID) << 26)
#define FSL_SAI_CR2_BCP          BIT(25)
#define FSL_SAI_CR2_BCD_MSTR     BIT(24)
#define FSL_SAI_CR2_BYP          BIT(23) /* BCLK bypass */
#define FSL_SAI_CR2_DIV_MASK     (0xff)

/* SAI Transmit and Receive Configuration 3 Register */
#define FSL_SAI_CR3_TRCE_MASK    (0xff << 16)
#define FSL_SAI_CR3_TRCE(x)      (x << 16)
#define FSL_SAI_CR3_WDFL(x)      (x)
#define FSL_SAI_CR3_WDFL_MASK    (0x1f)

/* SAI Transmit and Receive Configuration 4 Register */

#define FSL_SAI_CR4_FCONT       BIT(28)
#define FSL_SAI_CR4_FCOMB_SHIFT BIT(26)
#define FSL_SAI_CR4_FCOMB_SOFT  BIT(27)
#define FSL_SAI_CR4_FCOMB_MASK  (0x3 << 26)
#define FSL_SAI_CR4_FPACK_8     (0x2 << 24)
#define FSL_SAI_CR4_FPACK_16    (0x3 << 24)
#define FSL_SAI_CR4_FRSZ(x)     (((x) - 1) << 16)
#define FSL_SAI_CR4_FRSZ_MASK   (0x1f << 16)
#define FSL_SAI_CR4_SYWD(x)     (((x) - 1) << 8)
#define FSL_SAI_CR4_SYWD_MASK   (0x1f << 8)
#define FSL_SAI_CR4_CHMOD       (1 << 5)
#define FSL_SAI_CR4_CHMOD_MASK  (1 << 5)
#define FSL_SAI_CR4_MF          BIT(4)
#define FSL_SAI_CR4_FSE         BIT(3)
#define FSL_SAI_CR4_FSP         BIT(1)
#define FSL_SAI_CR4_FSD_MSTR    BIT(0)

/* SAI Transmit and Receive Configuration 5 Register */
#define FSL_SAI_CR5_WNW(x)      (((x) - 1) << 24)
#define FSL_SAI_CR5_WNW_MASK    (0x1f << 24)
#define FSL_SAI_CR5_W0W(x)      (((x) - 1) << 16)
#define FSL_SAI_CR5_W0W_MASK    (0x1f << 16)
#define FSL_SAI_CR5_FBT(x)      ((x) << 8)
#define FSL_SAI_CR5_FBT_MASK    (0x1f << 8)

/* SAI MCLK Control Register */
#define FSL_SAI_MCTL_MCLK_EN       BIT(30)    /* MCLK Enable */
#define FSL_SAI_MCTL_MSEL_MASK     (0x3 << 24)
#define FSL_SAI_MCTL_MSEL(ID)      ((ID) << 24)
#define FSL_SAI_MCTL_MSEL_BUS      (0)
#define FSL_SAI_MCTL_MSEL_MCLK1    BIT(24)
#define FSL_SAI_MCTL_MSEL_MCLK2    BIT(25)
#define FSL_SAI_MCTL_MSEL_MCLK3    (BIT(24) | BIT(25))
#define FSL_SAI_MCTL_DIV_EN        BIT(23)
#define FSL_SAI_MCTL_DIV_MASK      (0xFF)

/* SAI VERID Register */
#define FSL_SAI_VER_ID_SHIFT    (16)
#define FSL_SAI_VER_ID_MASK     (0xFFFF << FSL_SAI_VER_ID_SHIFT)
#define FSL_SAI_VER_EFIFO_EN    BIT(0)
#define FSL_SAI_VER_TSTMP_EN    BIT(1)

/* SAI PARAM Register */
#define FSL_SAI_PAR_SPF_SHIFT   (16)
#define FSL_SAI_PAR_SPF_MASK    (0x0F << FSL_SAI_PAR_SPF_SHIFT)
#define FSL_SAI_PAR_WPF_SHIFT   (8)
#define FSL_SAI_PAR_WPF_MASK    (0x0F << FSL_SAI_PAR_WPF_SHIFT)
#define FSL_SAI_PAR_DLN_MASK    (0x0F)

/* SAI MCLK Divide Register */
#define FSL_SAI_MDIV_MASK       (0xFFFFF)

/* SAI type */
#define FSL_SAI_DMA             BIT(0)
#define FSL_SAI_USE_AC97        BIT(1)
#define FSL_SAI_NET             BIT(2)
#define FSL_SAI_TRA_SYN         BIT(3)
#define FSL_SAI_REC_SYN         BIT(4)
#define FSL_SAI_USE_I2S_SLAVE   BIT(5)

#define FSL_FMT_TRANSMITTER     (0)
#define FSL_FMT_RECEIVER        (1)

/* SAI clock sources */
#define FSL_SAI_CLK_BUS         (0)
#define FSL_SAI_CLK_MAST1       (1)
#define FSL_SAI_CLK_MAST2       (2)
#define FSL_SAI_CLK_MAST3       (3)

#define FSL_SAI_MCLK_MAX        (4)

/* SAI data transfer numbers per DMA request */
#define FSL_SAI_MAXBURST_TX     (6)
#define FSL_SAI_MAXBURST_RX     (6)

#define SAI_FLAG_PMQOS          BIT(0)

/* SAI timestamp and bitcounter */
#define FSL_SAI_xTCTL_TSEN BIT(0)
#define FSL_SAI_xTCTL_TSINC BIT(1)
#define FSL_SAI_xTCTL_RTSC BIT(8)
#define FSL_SAI_xTCTL_RBC BIT(9)

#define FSL_REG_OFFSET          (8)

#define CLK_ENABLED             (1)
#define PLAYBACK                (2)

#define TX                      (1)
#define RX                      (0)

#define SOC_CLOCK_IN            (0)
#define SOC_CLOCK_OUT           (1)

struct fsl_sai_verid {
    u32 id;
    bool timestamp_en;
    bool extfifo_en;
    bool loaded;
};

struct fsl_sai_param {
    u32 spf; /* max slots per frame */
    u32 wpf; /* words in fifo */
    u32 dln; /* number of datalines implemented */
};

struct fsl_sai_dl_cfg {
    unsigned int pins;
    unsigned int mask[2];
    unsigned int offset[2];
};

struct fsl_sai {
    struct regmap *regmap;
    struct clk *bus_clk;
    struct clk *mclk_clk[FSL_SAI_MCLK_MAX];
    struct clk *pll8k_clk;
    struct clk *pll11k_clk;

    char *sai_name;

    uint8_t reg_offset;
    uint32_t dataline;
    uint32_t fifo_depth;

    bool slave_mode[2];
    bool is_lsb_first;
    bool is_dsp_mode;
    bool is_multi_lane;
    bool synchronous[2];
    bool is_stream_opened[2];
    bool is_dsd;

    int pcm_dl_cfg_cnt;
    int dsd_dl_cfg_cnt;
    struct fsl_sai_dl_cfg *pcm_dl_cfg;
    struct fsl_sai_dl_cfg *dsd_dl_cfg;

    unsigned int masterflag[2];

    unsigned int mclk_id[2];
    unsigned int mclk_streams;
    unsigned int slots;
    unsigned int slot_width;
    unsigned int bitclk_ratio;

    struct pinctrl *pinctrl;
    struct pinctrl_state *pins_state;

    struct fsl_sai_verid verid;
    struct fsl_sai_param param;
};

int32_t SaiSetDaiFmt(const struct PlatformData *pd, unsigned int fmt);
int32_t SaiSetDaiTdmSlot(const struct PlatformData *pd, int32_t tx_mask,
                         int32_t rx_mask, int32_t slots, int32_t slot_width);
int32_t SaiSetSysclk(const struct PlatformData *pd, int clk_id, unsigned int freq, int dir);
int32_t SaiSetHwParams(const struct PlatformData *pd, const enum AudioStreamType streamType);
int32_t SaiDriverInit(struct PlatformData *pd);
int32_t SaiTrigger(const struct DaiData *pd, int cmd, int isTx);
int32_t SaiRuntimeResume(const struct PlatformData *platformData);
int32_t SaiRuntimeSuspend(const struct DaiData *platformData);
int32_t SaiDaiProbe(const struct PlatformData *platformData);
int32_t SaiWrite(const struct PlatformData *platformData, int i);

#endif
