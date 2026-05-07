#include "miniaudio.h"

typedef struct
{
  int capture;
  int playback;
} device_indexes;

void select_device(ma_uint32 playbackCount, ma_device_info *pPlaybackInfos, device_indexes *indexes);

ma_result configure_device(
    ma_context *ctx,
    ma_device_config *cfg,
    ma_device *device,
    ma_device_info *pPlaybackInfos,
    ma_uint32 playbackCount,
    ma_device_info *pCaptureInfos,
    device_indexes *indexes);