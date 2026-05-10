#ifndef DELAY_EFFECT_H
#define DELAY_EFFECT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Audio settings for delay effect.
   * Thread-safe: volume, feedback, and bypass are atomics.
   * Ring buffer is only accessed by the audio callback thread (no contention).
   */
  typedef struct
  {
    _Atomic(float) volume;        /* Gain of the delayed signal (0.0-1.0) */
    _Atomic(float) feedback;      /* Feedback amount (0.0-0.99) */
    _Atomic(float) mix;           /* Dry/wet blend: 0.0 = dry only, 1.0 = wet only */
    _Atomic(bool) bypass;         /* Whether to bypass the effect */
    _Atomic(size_t) delaySamples; /* Beat-synced delay in samples (updated from Link BPM) */

    float *delayBuffer;        /* Ring buffer for delay (allocated by user) */
    size_t bufferSizeInFrames; /* Total size of delay buffer in frames */
    size_t writeIndex;         /* Current write position in ring buffer */
  } AudioSettings;

  /**
   * @brief Initialize audio settings struct.
   * @param settings Pointer to AudioSettings struct to initialize.
   * @param bufferSizeInFrames Size of delay buffer to allocate (in frames).
   * @param sampleRate Sample rate in Hz (used to calculate delay duration).
   * @return true on success, false on allocation failure.
   */
  bool delay_effect_init(AudioSettings *settings, size_t bufferSizeInFrames);

  /**
   * @brief Apply delay effect to input audio.
   * @param settings Pointer to AudioSettings struct.
   * @param input Input audio samples (float, mono or interleaved).
   * @param output Output audio samples (float, same layout as input).
   * @param numFrames Number of frames to process.
   * @param numChannels Number of channels (1 for mono, 2 for stereo).
   */
  void apply_delay_effect(
      AudioSettings *settings,
      const float *input,
      float *output,
      size_t numFrames,
      int numChannels);

  /**
   * @brief Cleanup audio settings (free allocated memory).
   * @param settings Pointer to AudioSettings struct.
   */
  void delay_effect_cleanup(AudioSettings *settings);

#ifdef __cplusplus
}
#endif

#endif /* DELAY_EFFECT_H */
