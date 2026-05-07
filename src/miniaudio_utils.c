#include "miniaudio_utils.h"

ma_result init_context(ma_context *pContext)
{
  return ma_context_init(NULL, 0, NULL, pContext);
}

ma_result can_get_devices(ma_context context, ma_device_info **pPlaybackInfos, ma_uint32 *playbackCount, ma_device_info **pCaptureInfos, ma_uint32 *captureCount)
{
  return ma_context_get_devices(&context, pPlaybackInfos, playbackCount, pCaptureInfos, captureCount);
}
