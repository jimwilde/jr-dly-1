#ifndef LINK_BRIDGE_H
#define LINK_BRIDGE_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Initialize Ableton Link session with audio support.
   * @param bpm Initial tempo in beats per minute.
   * @param sampleRate Sample rate in Hz (typically 48000).
   * @return Opaque handle to Link manager, NULL on failure.
   */
  void *link_init(double bpm, uint32_t sampleRate);

  /**
   * @brief Get current BPM from Link session (thread-safe).
   * @param handle Opaque Link manager handle from link_init().
   * @param outBpm Pointer to store BPM value.
   */
  void link_get_bpm(void *handle, double *outBpm);

  /**
   * @brief Get current beat position (thread-safe).
   * @param handle Opaque Link manager handle from link_init().
   * @param outBeat Pointer to store beat value.
   * @param quantum Quantum value for beat calculation (e.g., 4.0 for bars).
   */
  void link_get_beat(void *handle, double *outBeat, double quantum);

  /**
   * @brief Get number of connected peers (thread-safe).
   * @param handle Opaque Link manager handle from link_init().
   * @return Number of peers connected to Link session.
   */
  uint32_t link_get_num_peers(void *handle);

  /**
   * @brief Cleanup and release Link session resources.
   * @param handle Opaque Link manager handle from link_init().
   */
  void link_cleanup(void *handle);

#ifdef __cplusplus
}
#endif

#endif /* LINK_BRIDGE_H */
