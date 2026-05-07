#include "miniaudio.h"

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);

ma_result init_context(ma_context *pContext);

ma_result can_get_devices(ma_context context, ma_device_info **pPlaybackInfos, ma_uint32 *playbackCount, ma_device_info **pCaptureInfos, ma_uint32 *captureCount);

ma_result configure_encoder(char *filename, ma_device *device, ma_encoder_config *encoderConfig, ma_encoder *encoder);