#include <string.h>
#include "miniaudio_utils.h"

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
  ma_encoder *pEncoder = (ma_encoder *)pDevice->pUserData;

  // 1. Save to wav file
  if (pInput != NULL && pEncoder != NULL)
  {
    // Write all channels exactly as they come from the hardware
    ma_encoder_write_pcm_frames(pEncoder, pInput, frameCount, NULL);
  }

  // 2. Pass-through logic: Copy Mic Input to Speaker Output
  if (pOutput != NULL && pInput != NULL)
  {
    ma_uint32 bytesPerFrame = ma_get_bytes_per_frame(pDevice->playback.format, pDevice->playback.channels);
    memcpy(pOutput, pInput, frameCount * bytesPerFrame);
  }

  (void)pOutput;
}