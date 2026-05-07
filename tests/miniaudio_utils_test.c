#include <stdio.h>
#include "miniaudio_utils.h"
#include "assert_utils.h"

int test_init_context()
{
  ma_context context;
  ma_result result = init_context(&context);

  ASSERT_SUCCESS(result, "Context initialization should return MA_SUCCESS");

  ma_context_uninit(&context); // Clean up
  return 0;
}

int test_can_get_devices()
{
  ma_context context;
  ma_device_info *pPlaybackInfos;
  ma_uint32 playbackCount = 0;
  ma_device_info *pCaptureInfos;
  ma_uint32 captureCount = 0;

  init_context(&context);

  ma_result result = can_get_devices(&context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount);

  ASSERT_SUCCESS(result, "can_get_devices should return MA_SUCCESS");

  int found_any = (playbackCount + captureCount > 0);

  // We use a standard C assert or your custom handle_assertion for the logic check
  if (found_any)
  {
    printf("[PASS] Devices found: %u playback, %u capture\n", playbackCount, captureCount);
  }
  else
  {
    printf("[FAIL] No audio devices detected by the OS\n");
    return 1;
  }

  ma_context_uninit(&context); // Clean up
  return 0;
}

int main()
{
  printf("--- Running miniaudio_utils tests ---\n");

  int failures = 0;
  failures += test_init_context();
  failures += test_can_get_devices();

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
