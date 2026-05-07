#ifndef MINIAUDIO_UTILS_H
#define MINIAUDIO_UTILS_H
#include "miniaudio.h" // Just the "menu" (declarations)

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);

ma_result init_context(ma_context *pContext);

ma_result can_get_devices(ma_context context, ma_device_info **pPlaybackInfos, ma_uint32 *playbackCount, ma_device_info **pCaptureInfos, ma_uint32 *captureCount);

#endif