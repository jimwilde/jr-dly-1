#include <stdio.h>
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

ma_result configure_device(cfg_devices_args *args)
{
  *args->indexes = (device_indexes){.capture = 0, .playback = 0};
  // user prompted to choose c/p devices
  select_device(args->playbackCount, args->pPlaybackInfos, args->indexes);

  // 1. Initialize the Device FIRST with "0" (native) settings
  *args->cfg = ma_device_config_init(ma_device_type_duplex);
  args->cfg->capture.pDeviceID = &args->pCaptureInfos[args->indexes->capture].id;
  args->cfg->capture.format = ma_format_f32;
  args->cfg->capture.channels = 0;
  args->cfg->playback.pDeviceID = &args->pPlaybackInfos[args->indexes->playback].id;
  args->cfg->playback.format = ma_format_f32;
  args->cfg->playback.channels = 0;
  args->cfg->sampleRate = 0;
  args->cfg->dataCallback = data_callback;
  // args->cfg->pUserData = &encoder; // Do NOT set this yet as encoder isn't ready

  return ma_device_init(args->ctx, args->cfg, args->device);
}