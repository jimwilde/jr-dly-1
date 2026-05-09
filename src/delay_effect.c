#include "delay_effect.h"
#include <stdlib.h>
#include <string.h>

bool delay_effect_init(AudioSettings *settings, size_t bufferSizeInFrames)
{
  settings->delayBuffer = (float *)malloc(bufferSizeInFrames * sizeof(float));
  if (!settings->delayBuffer)
    return false;

  settings->bufferSizeInFrames = bufferSizeInFrames;
  settings->writeIndex = 0;

  /* Initialize atomics */
  atomic_init(&settings->volume, 0.5f);
  atomic_init(&settings->feedback, 0.8f);
  atomic_init(&settings->bypass, false);

  /* Zero out delay buffer */
  memset(settings->delayBuffer, 0, bufferSizeInFrames * sizeof(float));

  return true;
}

void apply_delay_effect(
    AudioSettings *settings,
    const float *input,
    float *output,
    size_t numFrames,
    int numChannels)
{
  if (!settings || !input || !output)
    return;

  /* Check bypass flag */
  if (atomic_load(&settings->bypass))
  {
    /* If bypassed, copy input to output and return */
    memcpy(output, input, numFrames * numChannels * sizeof(float));
    return;
  }

  float volume = atomic_load(&settings->volume);
  float feedback = atomic_load(&settings->feedback);

  size_t bufferSize = settings->bufferSizeInFrames;
  float *delayLine = settings->delayBuffer;
  size_t writeIdx = settings->writeIndex;

  for (size_t i = 0; i < numFrames * numChannels; ++i)
  {
    /* Read delayed sample from ring buffer */
    float delayedSample = delayLine[writeIdx];

    /* Output = input + (delayed signal * volume) */
    output[i] = input[i] + (delayedSample * volume);

    /* Write back to delay buffer: input + (delayed * feedback) */
    delayLine[writeIdx] = input[i] + (delayedSample * feedback);

    /* Advance write pointer */
    writeIdx++;
    if (writeIdx >= bufferSize)
      writeIdx = 0;
  }

  /* Save updated write index */
  settings->writeIndex = writeIdx;
}

void delay_effect_cleanup(AudioSettings *settings)
{
  if (settings && settings->delayBuffer)
  {
    free(settings->delayBuffer);
    settings->delayBuffer = NULL;
  }
}
