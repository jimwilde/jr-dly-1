#include "miniaudio_utils.h"
typedef struct
{
  int capture;
  int playback;
} device_indexes;

void select_device(ma_uint32 playbackCount, ma_device_info *pPlaybackInfos, device_indexes *indexes);

typedef struct
{
  ma_context *ctx;
  ma_device_config *cfg;
  ma_device *device;
  ma_device_info *pPlaybackInfos;
  ma_uint32 playbackCount;
  ma_device_info *pCaptureInfos;
  device_indexes *indexes;
} cfg_devices_args;

ma_result configure_device(cfg_devices_args *args);