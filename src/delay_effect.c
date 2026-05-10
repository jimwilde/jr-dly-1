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
  atomic_init(&settings->mix, 1.0f);
  atomic_init(&settings->bypass, false);
  atomic_init(&settings->delaySamples, bufferSizeInFrames / 2);

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
  float mix = atomic_load(&settings->mix);

  size_t bufferSize = settings->bufferSizeInFrames;
  float *delayLine = settings->delayBuffer;
  size_t writeIdx = settings->writeIndex;

  size_t delay = atomic_load(&settings->delaySamples);
  if (delay == 0 || delay >= bufferSize)
    delay = bufferSize - 1;

  for (size_t i = 0; i < numFrames * numChannels; ++i)
  {
    /* Read from delay samples behind the write head */
    size_t readIdx = (writeIdx + bufferSize - delay) % bufferSize;
    float delayedSample = delayLine[readIdx];

    output[i] = input[i] * (1.0f - mix) + (delayedSample * volume * mix);

    /* Write back to delay buffer: input + (delayed * feedback) */
    delayLine[writeIdx] = input[i] + (delayedSample * feedback);

    /* Advance write pointer */
    writeIdx = (writeIdx + 1) % bufferSize;
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
