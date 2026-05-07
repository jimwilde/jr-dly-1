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

  result = can_get_devices(&context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount);
  if (result != MA_SUCCESS)
  {
    return -1;
  }

  // Set up device
  ma_device_config deviceConfig;
  ma_device device;
  device_indexes dev_idxs = {.capture = 0, .playback = 0};

  // user prompted to choose c/p devices
  select_device(playbackCount, pPlaybackInfos, captureCount, pCaptureInfos, &dev_idxs);

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
  AudioSettings audio_settings = {
      .bypass = false,
      .encoder = &encoder,
      .volume = 0.5,
      .bufferSizeInFrames = device.sampleRate * 2,
      .writeIndex = 0,
      .feedback = 0.8f};

  result = ma_pcm_rb_init(
      device.capture.format,
      device.capture.channels,
      audio_settings.bufferSizeInFrames,
      NULL, // Let miniaudio allocate the memory
      NULL, // Use default allocation callbacks
      &audio_settings.delayBuffer);
  if (result != MA_SUCCESS)
  {
    printf("Failed to initialize ring buffer.\n");
    return -1;
  }

  device.pUserData = &audio_settings; // attach audio settings to device

  // Advance the write pointer by the initial delay amount.
  // This leaves an empty region in the ring buffer so playback/read can start
  // after the first chunk of captured audio has been recorded.
  ma_uint32 seekFrames = (ma_uint32)(audio_settings.feedback * device.sampleRate);
  ma_pcm_rb_seek_write(&audio_settings.delayBuffer, seekFrames);

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
    {
      break;
    }
    else if (line[0] == 'b')
    {
      audio_settings.bypass = !audio_settings.bypass;
      printf("Bypass is: %s\n", audio_settings.bypass ? "ON" : "OFF");
    }
    else if (line[0] == 'v')
    {
      float newVolume;
      if (sscanf(&line[2], "%f", &newVolume) == 1)
      {
        // Clamp volume between 0.0 and 1.0
        if (newVolume < 0.0f)
          newVolume = 0.0f;
        if (newVolume > 1.0f)
          newVolume = 1.0f;

        audio_settings.volume = newVolume;
        printf("Volume set to %.2f\n", audio_settings.volume);
      }
    }
    else if (line[0] == 'f')
    {
      float newFeedback;
      if (sscanf(&line[2], "%f", &newFeedback) == 1)
      {
        // CRITICAL: Clamp feedback strictly below 1.0
        if (newFeedback < 0.0f)
          newFeedback = 0.0f;
        if (newFeedback >= 0.99f)
          newFeedback = 0.99f;

        audio_settings.feedback = newFeedback;
        printf("Feedback set to %.2f\n", audio_settings.feedback);
      }
    }
  }

  // uninitialise miniaudio elements
  ma_context_uninit(&context);
  ma_device_uninit(&device);
  ma_encoder_uninit(&encoder);

  return 0;
}