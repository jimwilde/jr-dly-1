#ifndef LINK_BRIDGE_STUB_H
#define LINK_BRIDGE_STUB_H

#include <stdbool.h>

/* Stub state exposed for test assertions */
extern float stub_volume;
extern float stub_feedback;
extern float stub_mix;
extern bool  stub_bypass;
extern float stub_delay_beats;
extern float stub_manual_latency_ms;

void stub_reset(void);

#endif /* LINK_BRIDGE_STUB_H */
