#include <stdio.h>
#include <string.h>
#include "miniaudio_utils.h"
#include "device.h"
#include "encoder.h"

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("No output filename provided.\n");
    return -1;
  }

  ma_result result;

  // Set up context
  ma_context context;
  result = init_context(&context);
  if (result != MA_SUCCESS)
  {
    return -1;
  }

  ma_device_info *pPlaybackInfos;
  ma_uint32 playbackCount = 0;
  ma_device_info *pCaptureInfos;
  ma_uint32 captureCount = 0;

  result = can_get_devices(context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount);
  if (result != MA_SUCCESS)
  {
    return -1;
  }

  // Set up device
  ma_device_config deviceConfig;
  ma_device device;
  device_indexes dev_idxs;

  result = configure_device(&(cfg_devices_args){
      .ctx = &context,
      .cfg = &deviceConfig,
      .device = &device,
      .pPlaybackInfos = pPlaybackInfos,
      .playbackCount = playbackCount,
      .pCaptureInfos = pCaptureInfos,
      .indexes = &dev_idxs});
  if (result != MA_SUCCESS)
  {
    printf("Failed to initialize capture device.\n");
    return -2;
  }

  // Set up encoder
  ma_encoder_config encoderConfig;
  ma_encoder encoder;

  result = configure_encoder(&(cfg_encoder_args){
      .filename = argv[1],
      .device = &device,
      .encoderConfig = &encoderConfig,
      .encoder = &encoder});
  if (result != MA_SUCCESS)
  {
    printf("Failed to initialize output file.\n");
    return -1;
  }

  // SET the user data AFTER the encoder is ready but BEFORE starting
  AudioSettings audio_settings = {.bypass = false, .encoder = &encoder, .volume = 0.5};
  device.pUserData = &audio_settings;

  // START the hardware thread
  result = ma_device_start(&device);
  if (result != MA_SUCCESS)
  {
    ma_device_uninit(&device);
    printf("Failed to start device.\n");
    return -3;
  }

  printf("Press Enter to stop recording...\n");
  for (;;)
  {
    printf("> ");
    char line[16];
    fgets(line, sizeof(line), stdin);
    if (line[0] == '\n')
      break;
    if (line[0] == 'b')
    {
      audio_settings.bypass = !audio_settings.bypass;
      printf("Bypass is: %s\n", audio_settings.bypass ? "ON" : "OFF");
    }
  }

  // uninitialise miniaudio elements
  ma_device_uninit(&device);
  ma_encoder_uninit(&encoder);

  return 0;
}