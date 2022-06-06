/*
 *
 *
 */

#ifndef IMX8MM_PLATFORM_OPS_H
#define IMX8MM_PLATFORM_OPS_H

#include "audio_core.h"

int32_t Imx8mmDmaBufAlloc(struct PlatformData *data, const enum AudioStreamType streamType);
int32_t Imx8mmDmaBufFree(struct PlatformData *data, const enum AudioStreamType streamType);
int32_t Imx8mmDmaRequestChannel(const struct PlatformData *data, const enum AudioStreamType streamType);
int32_t Imx8mmDmaConfigChannel(const struct PlatformData *data, const enum AudioStreamType streamType);
int32_t Imx8mmDmaPrep(const struct PlatformData *data, const enum AudioStreamType streamType);
int32_t Imx8mmDmaSubmit(const struct PlatformData *data, const enum AudioStreamType streamType);
int32_t Imx8mmDmaPending(struct PlatformData *data, const enum AudioStreamType streamType);
int32_t Imx8mmDmaPause(struct PlatformData *data, const enum AudioStreamType streamType);
int32_t Imx8mmDmaResume(const struct PlatformData *data, const enum AudioStreamType streamType);
int32_t Imx8mmDmaPointer(struct PlatformData *data, const enum AudioStreamType streamType, uint32_t *pointer);

#endif
