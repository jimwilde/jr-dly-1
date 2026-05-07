#include "miniaudio_utils.h"

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
  ma_encoder *pEncoder = (ma_encoder *)pDevice->pUserData;

  if (pInput != NULL && pEncoder != NULL)
  {
    // Write all channels exactly as they come from the hardware
    ma_encoder_write_pcm_frames(pEncoder, pInput, frameCount, NULL);
  }

  (void)pOutput;
}