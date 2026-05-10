#include <stdint.h>
#include "link_bridge.h"
#include "link_bridge_stub.h"

float stub_volume            = 0.0f;
float stub_feedback          = 0.0f;
float stub_mix               = 0.0f;
bool  stub_bypass            = false;
float stub_delay_beats       = 1.0f;
float stub_manual_latency_ms = 0.0f;

void stub_reset(void)
{
  stub_volume            = 0.0f;
  stub_feedback          = 0.0f;
  stub_mix               = 0.0f;
  stub_bypass            = false;
  stub_delay_beats       = 1.0f;
  stub_manual_latency_ms = 0.0f;
}

void *link_init(double bpm, uint32_t sr)         { (void)bpm; (void)sr; return (void *)1; }
void  link_cleanup(void *h)                      { (void)h; }
void  link_get_bpm(void *h, double *out)         { (void)h; if (out) *out = 120.0; }
void  link_get_beat(void *h, double *out, double q) { (void)h; (void)q; if (out) *out = 0.0; }
uint32_t link_get_num_peers(void *h)             { (void)h; return 1; }
bool  link_audio_is_enabled(void *h)             { (void)h; return true; }
void  link_audio_enable(void *h, bool e)         { (void)h; (void)e; }
bool  link_has_source(void *h)                   { (void)h; return false; }

void  link_set_volume(void *h, float v)          { (void)h; stub_volume = v; }
void  link_set_feedback(void *h, float f)        { (void)h; stub_feedback = f; }
void  link_set_mix(void *h, float m)             { (void)h; stub_mix = m; }
void  link_set_bypass(void *h, bool b)           { (void)h; stub_bypass = b; }
bool  link_get_bypass(void *h)                   { (void)h; return stub_bypass; }
void  link_set_delay_beats(void *h, float b)     { (void)h; stub_delay_beats = b; }
float link_get_delay_beats(void *h)              { (void)h; return stub_delay_beats; }
float link_get_buffer_latency_ms(void *h)        { (void)h; return 10.0f; }
void  link_set_manual_latency_ms(void *h, float ms) { (void)h; stub_manual_latency_ms = ms; }
float link_get_manual_latency_ms(void *h)        { (void)h; return stub_manual_latency_ms; }

uint32_t link_list_channels(void *h, LinkAudioChannel *out, uint32_t max)
                                                 { (void)h; (void)out; (void)max; return 0; }
void link_subscribe_channel(void *h, const uint8_t id[8]) { (void)h; (void)id; }
void link_unsubscribe_channel(void *h)           { (void)h; }
void link_set_channels_changed_callback(void *h, void (*cb)(void *), void *ctx)
                                                 { (void)h; (void)cb; (void)ctx; }
