#ifndef LINK_BRIDGE_H
#define LINK_BRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct
  {
    char name[256];
    char peer_name[256];
    uint8_t id[8];
  } LinkAudioChannel;

  void *link_init(double bpm, uint32_t sampleRate);
  void link_get_bpm(void *handle, double *outBpm);
  void link_get_beat(void *handle, double *outBeat, double quantum);
  uint32_t link_get_num_peers(void *handle);
  void link_cleanup(void *handle);

  /* Effect parameters */
  void link_set_volume(void *handle, float volume);
  void link_set_feedback(void *handle, float feedback);
  void link_set_mix(void *handle, float mix);
  void link_set_bypass(void *handle, bool bypass);
  bool link_get_bypass(void *handle);

  /* Delay timing */
  void link_set_delay_beats(void *handle, float beats);
  float link_get_delay_beats(void *handle);

  /* Audio control */
  bool link_audio_is_enabled(void *handle);
  void link_audio_enable(void *handle, bool enabled);
  bool link_has_source(void *handle);
  uint32_t link_list_channels(void *handle, LinkAudioChannel *out, uint32_t max);
  void link_subscribe_channel(void *handle, const uint8_t id[8]);
  void link_unsubscribe_channel(void *handle);
  /* Latency compensation */
  float link_get_buffer_latency_ms(void *handle);
  void link_set_manual_latency_ms(void *handle, float ms);
  float link_get_manual_latency_ms(void *handle);

  void link_set_channels_changed_callback(void *handle,
                                          void (*callback)(void *context),
                                          void *context);

#ifdef __cplusplus
}
#endif

#endif /* LINK_BRIDGE_H */
