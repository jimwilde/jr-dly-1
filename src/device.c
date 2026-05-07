#include <stdio.h>
#include "miniaudio.h"
#include "device.h"
#include "miniaudio_utils.h"

void select_device(ma_uint32 playbackCount, ma_device_info *pPlaybackInfos, device_indexes *indexes)
{
  // Show user device options
  for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice += 1)
  {
    printf("%d - %s\n", iDevice, pPlaybackInfos[iDevice].name);
  }

  printf("Select capture device: ");
  scanf("%d%*c", &indexes->capture);

  printf("Select playback device: ");
  scanf("%d%*c", &indexes->playback);
}

ma_result configure_device(
    ma_context *ctx,
    ma_device_config *cfg,
    ma_device *device,
    ma_device_info *pPlaybackInfos,
    ma_uint32 playbackCount,
    ma_device_info *pCaptureInfos,
    device_indexes *indexes)
{
  *indexes = (device_indexes){.capture = 0, .playback = 0};
  // user prompted to choose c/p devices
  select_device(playbackCount, pPlaybackInfos, indexes);

  // 1. Initialize the Device FIRST with "0" (native) settings
  *cfg = ma_device_config_init(ma_device_type_duplex);
  cfg->capture.pDeviceID = &pCaptureInfos[indexes->capture].id;
  cfg->capture.format = ma_format_f32;
  cfg->capture.channels = 0;
  cfg->playback.pDeviceID = &pPlaybackInfos[indexes->playback].id;
  cfg->playback.format = ma_format_f32;
  cfg->playback.channels = 0;
  cfg->sampleRate = 0;
  cfg->dataCallback = data_callback;
  // deviceConfig.pUserData = &encoder; // Do NOT set this yet as encoder isn't ready

  return ma_device_init(ctx, cfg, device);
}