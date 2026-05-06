#include <stdio.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
  // In playback mode copy data to pOutput. In capture mode read data from pInput. In full-duplex mode, both
  // pOutput and pInput will be valid and you can move data from pInput into pOutput. Never process more than
  // frameCount frames.
}

// CONTEXT SETUP
ma_result init_context(ma_context *pContext)
{
  return ma_context_init(NULL, 0, NULL, pContext);
}

ma_result can_get_devices(ma_context context, ma_device_info **pPlaybackInfos, ma_uint32 *playbackCount, ma_device_info **pCaptureInfos, ma_uint32 *captureCount)
{
  return ma_context_get_devices(&context, pPlaybackInfos, playbackCount, pCaptureInfos, captureCount);
}

// CONFIG SETUP
ma_device_config init_config()
{
  return ma_device_config_init(ma_device_type_duplex);
}

void setup_config(ma_device_config config, ma_device_info *pPlaybackInfos, ma_uint32 playbackCount, ma_device_info *pCaptureInfos, ma_uint32 captureCount)
{
  // Loop over each device info and do something with it. Here we just print the name with their index. You may want
  // to give the user the opportunity to choose which device they'd prefer.
  for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice += 1)
  {
    printf("%d - %s\n", iDevice, pPlaybackInfos[iDevice].name);
  }
  int playbackIndex = 0;
  printf("Select playback device: ");
  scanf("%d", &playbackIndex);
  int captureIndex = 0;
  printf("Select capture device: ");
  scanf("%d", &captureIndex);

  config.playback.pDeviceID = &pPlaybackInfos[playbackIndex].id;
  config.capture.pDeviceID = &pPlaybackInfos[captureIndex].id;
  config.playback.format = ma_format_f32; // Set to ma_format_unknown to use the device's native format.
  config.capture.format = ma_format_f32;  // Set to ma_format_unknown to use the device's native format.
  config.sampleRate = 0;                  // Set to 0 to use the device's native sample rate.
  config.dataCallback = data_callback;    // This function will be called when miniaudio needs more data.
  // config.pUserData = pMyCustomData;    // Can be accessed from the device object (device.pUserData).
}

int main()
{
  ma_context context;
  if (init_context(&context) != MA_SUCCESS)
    return -1;

  ma_device_info *pPlaybackInfos;
  ma_uint32 playbackCount = 0;
  ma_device_info *pCaptureInfos;
  ma_uint32 captureCount = 0;

  if (can_get_devices(context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS)
    return -1;

  ma_device_config config = init_config();
  setup_config(config, pPlaybackInfos, playbackCount, pCaptureInfos, captureCount);

  ma_device device;
  if (ma_device_init(&context, &config, &device) != MA_SUCCESS)
  {
    return -1; // Failed to initialize the device.
  }

  ma_device_start(&device); // The device is sleeping by default so you'll need to start it manually.

  // Do something here. Probably your program's main loop.

  ma_device_uninit(&device);
  return 0;
}