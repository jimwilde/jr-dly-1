#include <stdio.h>
#include "miniaudio_utils.h"
#include "assert_utils.h"
#include "encoder.h"
#include "device.h"

int test_configure_encoder()
{

  ma_encoder_config encoderConfig;
  ma_encoder encoder;
  ma_device fakeDevice = {0};
  fakeDevice.sampleRate = 44100;
  fakeDevice.capture.channels = 2;
  char *filename = "test_output.wav";

  cfg_encoder_args args = {
      .filename = filename,
      .device = &fakeDevice, // Pass the fake device
      .encoderConfig = &encoderConfig,
      .encoder = &encoder};

  ma_result result = configure_encoder(&args);

  ASSERT_SUCCESS(result, "Configuring encoder should return MA_SUCCESS");

  // Clean up
  ma_encoder_uninit(&encoder);
  remove(filename);
  return 0;
}

int main()
{
  printf("--- Running encoder tests ---\n");

  int failures = 0;
  failures += test_configure_encoder();

  if (failures == 0)
  {
    printf("All tests passed!\n");
  }
  else
  {
    printf("%d tests failed.\n", failures);
  }

  return failures;
}
