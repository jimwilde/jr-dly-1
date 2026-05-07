#include <string.h>
#include "miniaudio_utils.h"
#include "device.h"

void data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
  AudioSettings *pSettings = pDevice->pUserData;
  if (pSettings == NULL)
    return;

  // 1. Start with the DRY signal on the output
  ma_copy_pcm_frames(pOutput, pInput, frameCount, pDevice->playback.format, pDevice->playback.channels);

  if (pSettings->bypass)
    return;

  // 2. ACQUIRE both Read and Write pointers
  // We need the "Read" to get the old echo, and the "Write" to store the new mix
  ma_uint32 framesToProcess = frameCount;
  void *pReadBuffer;
  void *pWriteBuffer;

  if (ma_pcm_rb_acquire_read(&pSettings->delayBuffer, &framesToProcess, &pReadBuffer) == MA_SUCCESS)
  {
    if (ma_pcm_rb_acquire_write(&pSettings->delayBuffer, &framesToProcess, &pWriteBuffer) == MA_SUCCESS)
    {
      float *pOut = (float *)pOutput;
      float *pIn = (float *)pInput;
      float *pDelayLine = (float *)pReadBuffer;
      float *pFeedbackLoop = (float *)pWriteBuffer;

      ma_uint32 totalSamples = framesToProcess * pDevice->playback.channels;

      for (ma_uint32 i = 0; i < totalSamples; ++i)
      {
        // Current echo we are hearing
        float oldEcho = pDelayLine[i];

        // Add echo to output
        pOut[i] += (oldEcho * pSettings->volume);

        // FEEDBACK: Write the mix back into the buffer for the NEXT cycle
        // New Memory = Mic Input + (Old Echo reduced by feedback)
        pFeedbackLoop[i] = pIn[i] + (oldEcho * pSettings->feedback);
      }

      ma_pcm_rb_commit_write(&pSettings->delayBuffer, framesToProcess);
    }
    ma_pcm_rb_commit_read(&pSettings->delayBuffer, framesToProcess);
  }
}
