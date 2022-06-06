#ifndef DMA_DRIVER_H
#define DMA_DRIVER_H

int32_t DMAInitTxBuff(struct PlatformData *platformData);
int32_t DMAInitRxBuff(struct PlatformData *platformData);
int32_t DMAEnableTx(const struct PlatformData *platformData);
int32_t DMAEnableRx(const struct PlatformData *platformData);
int32_t DMADisableTx(struct PlatformData *platformData);
int32_t DMADisableRx(struct PlatformData *platformData);
int32_t DMAPauseTx(struct PlatformData *platformData);
int32_t DMAPauseRx(struct PlatformData *platformData);
int32_t DmaInit(struct PlatformData *platformData);
int32_t DMADeinitTxBuff(struct PlatformData *platformData);
int32_t DMADeinitRxBuff(struct PlatformData *platformData);

#endif
