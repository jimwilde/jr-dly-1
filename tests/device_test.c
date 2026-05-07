#include <stdio.h>
#include <string.h>
#include "assert_utils.h"
#include "device.h"

void test_select_device()
{
  // 1. Create an array of 2 dummy devices
  ma_device_info dummyCaptureDevices[2];
  ma_device_info dummyPlaybackDevices[2];
  memset(dummyCaptureDevices, 0, sizeof(dummyCaptureDevices));   // Clear memory
  memset(dummyPlaybackDevices, 0, sizeof(dummyPlaybackDevices)); // Clear memory

  // 2. Set the names (use strncpy to be safe with fixed-length arrays)
  strncpy(dummyCaptureDevices[0].name, "Scarlett 18i20", MA_MAX_DEVICE_NAME_LENGTH);
  strncpy(dummyCaptureDevices[1].name, "MacBook Mic", MA_MAX_DEVICE_NAME_LENGTH);
  strncpy(dummyPlaybackDevices[0].name, "Scarlett 18i20", MA_MAX_DEVICE_NAME_LENGTH);
  strncpy(dummyPlaybackDevices[1].name, "MacBook Mic", MA_MAX_DEVICE_NAME_LENGTH);

  // 3. Set up the fake input (User selects index 0)
  const char *filename = "test_input.txt";
  FILE *f = fopen(filename, "w");
  fprintf(f, "0\n0\n"); // Select 0 for capture, 0 for playback
  fclose(f);
  freopen(filename, "r", stdin);

  // 4. Run the function
  device_indexes idxs = {-1, -1};
  printf("\n--- Expecting to see Scarlett and MacBook in output ---\n");
  select_device(2, dummyPlaybackDevices, 2, dummyCaptureDevices, &idxs);

  printf("\n");

  // 5. Verify
  assert(idxs.capture == 0);
  ASSERT_SUCCESS(MA_SUCCESS, "select_device: Correctly parsed redirected input");

  // Clean up
  freopen("/dev/tty", "r", stdin);
  remove(filename);
}

void test_configure_device()
{
  ma_context ctx;
  ma_device_config config;
  ma_device device;
  device_indexes idxs = {0, 0}; // Assuming device 0 is valid

  // We need real device info for the ID pointers to be valid
  ma_device_info *pPlay, *pCap;
  ma_uint32 playCount, capCount;
  can_get_devices(&ctx, &pPlay, &playCount, &pCap, &capCount);

  cfg_devices_args args = {
      .ctx = &ctx,
      .cfg = &config,
      .device = &device,
      .pPlaybackInfos = pPlay,
      .pCaptureInfos = pCap,
      .indexes = &idxs};

  ma_result result = configure_device(&args);
  ASSERT_SUCCESS(result, "configure_device should return MA_SUCCESS with valid hardware");

  ma_device_uninit(&device);
  ma_context_uninit(&ctx);
}

int main()
{
  test_select_device();
  return 0;
}