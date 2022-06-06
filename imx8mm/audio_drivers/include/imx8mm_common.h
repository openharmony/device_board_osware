#ifndef IMX8MM_COMMON_H
#define IMX8MM_COMMON_H

#include "audio_platform_if.h"

typedef enum {
    AUDIO_BIT_WIDTH_8   = 0,   /* 8bit width */
    AUDIO_BIT_WIDTH_16  = 1,   /* 16bit width */
    AUDIO_BIT_WIDTH_24  = 2,   /* 24bit width */
    AUDIO_BIT_WIDTH_BUTT,
} AudioBitWidth;

enum {
    AUDIODRV_CTL_ELEM_IFACE_DAC = 0,      /* virtual dac device */
    AUDIODRV_CTL_ELEM_IFACE_ADC = 1,      /* virtual adc device */
    AUDIODRV_CTL_ELEM_IFACE_GAIN = 2,     /* virtual adc device */
    AUDIODRV_CTL_ELEM_IFACE_MIXER = 3,    /* virtual mixer device */
    AUDIODRV_CTL_ELEM_IFACE_ACODEC = 4,   /* Acodec device */
    AUDIODRV_CTL_ELEM_IFACE_PGA = 5,      /* PGA device */
    AUDIODRV_CTL_ELEM_IFACE_AIAO = 6,     /* AIAO device */
};

#endif
