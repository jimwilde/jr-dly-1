#include <stdio.h>
#include <stdatomic.h>
#include "delay_effect.h"
#include "assert_utils.h"

#define BUF_SIZE 32

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static void set_params(AudioSettings *s,
                       float volume, float feedback, float mix,
                       size_t delaySamples)
{
  atomic_store(&s->volume, volume);
  atomic_store(&s->feedback, feedback);
  atomic_store(&s->mix, mix);
  atomic_store(&s->delaySamples, delaySamples);
  atomic_store(&s->bypass, false);
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static int test_init_defaults(void)
{
  int fails = 0;
  AudioSettings s;
  delay_effect_init(&s, BUF_SIZE);

  fails += ASSERT_FLOAT_EQ(atomic_load(&s.volume),   0.5f, 0.001f, "init: volume  = 0.5");
  fails += ASSERT_FLOAT_EQ(atomic_load(&s.feedback), 0.8f, 0.001f, "init: feedback = 0.8");
  fails += ASSERT_FLOAT_EQ(atomic_load(&s.mix),      1.0f, 0.001f, "init: mix      = 1.0");
  fails += ASSERT_TRUE(atomic_load(&s.bypass) == false,            "init: bypass   = false");
  fails += ASSERT_TRUE(atomic_load(&s.delaySamples) == BUF_SIZE / 2,
                                                                   "init: delaySamples = bufSize/2");

  delay_effect_cleanup(&s);
  return fails;
}

static int test_bypass_passes_input_through(void)
{
  int fails = 0;
  AudioSettings s;
  delay_effect_init(&s, BUF_SIZE);
  atomic_store(&s.bypass, true);

  float input[]  = {0.1f, 0.5f, -0.3f, 0.9f};
  float output[] = {0.0f, 0.0f,  0.0f, 0.0f};
  apply_delay_effect(&s, input, output, 4, 1);

  fails += ASSERT_FLOAT_EQ(output[0], 0.1f,  0.0001f, "bypass: output[0] == input[0]");
  fails += ASSERT_FLOAT_EQ(output[1], 0.5f,  0.0001f, "bypass: output[1] == input[1]");
  fails += ASSERT_FLOAT_EQ(output[2], -0.3f, 0.0001f, "bypass: output[2] == input[2]");
  fails += ASSERT_FLOAT_EQ(output[3], 0.9f,  0.0001f, "bypass: output[3] == input[3]");

  delay_effect_cleanup(&s);
  return fails;
}

static int test_dry_mix_passes_input_through(void)
{
  int fails = 0;
  AudioSettings s;
  delay_effect_init(&s, BUF_SIZE);
  set_params(&s, 1.0f, 0.5f, 0.0f, 4);  /* mix=0 → dry only */

  float input[]  = {0.2f, 0.7f, -0.4f, 0.6f};
  float output[] = {0.0f, 0.0f,  0.0f, 0.0f};
  apply_delay_effect(&s, input, output, 4, 1);

  fails += ASSERT_FLOAT_EQ(output[0], 0.2f,  0.0001f, "dry mix=0: output[0] == input[0]");
  fails += ASSERT_FLOAT_EQ(output[1], 0.7f,  0.0001f, "dry mix=0: output[1] == input[1]");
  fails += ASSERT_FLOAT_EQ(output[2], -0.4f, 0.0001f, "dry mix=0: output[2] == input[2]");
  fails += ASSERT_FLOAT_EQ(output[3], 0.6f,  0.0001f, "dry mix=0: output[3] == input[3]");

  delay_effect_cleanup(&s);
  return fails;
}

/* An impulse at sample 0 should appear in the output exactly D samples later.
   With feedback=0 and mix=1, the system is a pure delay line. */
static int test_impulse_delay_timing(void)
{
  int fails = 0;
  AudioSettings s;
  delay_effect_init(&s, BUF_SIZE);

  const size_t D = 4;
  set_params(&s, 1.0f, 0.0f, 1.0f, D);  /* vol=1, feedback=0, mix=1 */

  /* Impulse at [0], silence for the rest */
  float input[BUF_SIZE]  = {0};
  float output[BUF_SIZE] = {0};
  input[0] = 1.0f;

  apply_delay_effect(&s, input, output, BUF_SIZE, 1);

  /* Samples before the delay should be silent */
  for (size_t i = 0; i < D; ++i)
    fails += ASSERT_FLOAT_EQ(output[i], 0.0f, 0.0001f,
                             "impulse delay: pre-delay output is 0");

  /* The impulse arrives at exactly sample D */
  fails += ASSERT_FLOAT_EQ(output[D], 1.0f, 0.0001f,
                           "impulse delay: output[D] == 1.0 (impulse arrived)");

  /* Silence after the single echo (feedback=0) */
  for (size_t i = D + 1; i < BUF_SIZE; ++i)
    fails += ASSERT_FLOAT_EQ(output[i], 0.0f, 0.0001f,
                             "impulse delay: post-echo silence (feedback=0)");

  delay_effect_cleanup(&s);
  return fails;
}

/* Volume should scale only the wet (delayed) component. */
static int test_volume_scales_wet_signal(void)
{
  int fails = 0;
  AudioSettings s;
  delay_effect_init(&s, BUF_SIZE);

  const size_t D = 4;
  set_params(&s, 0.5f, 0.0f, 1.0f, D);  /* vol=0.5, feedback=0, mix=1 */

  float input[BUF_SIZE]  = {0};
  float output[BUF_SIZE] = {0};
  input[0] = 1.0f;

  apply_delay_effect(&s, input, output, BUF_SIZE, 1);

  fails += ASSERT_FLOAT_EQ(output[D], 0.5f, 0.0001f,
                           "volume 0.5: delayed output is 0.5 * impulse");

  delay_effect_cleanup(&s);
  return fails;
}

/* Feedback: each echo should be (previous echo) * feedback.
   At 120 BPM with D=4 samples, echoes appear at D, 2D, 3D.
   Amplitudes: 1, feedback, feedback^2. */
static int test_feedback_echoes_decay(void)
{
  int fails = 0;
  AudioSettings s;
  delay_effect_init(&s, BUF_SIZE);

  const size_t D = 4;
  const float  FB = 0.5f;
  set_params(&s, 1.0f, FB, 1.0f, D);  /* vol=1, feedback=0.5, mix=1 */

  float input[BUF_SIZE]  = {0};
  float output[BUF_SIZE] = {0};
  input[0] = 1.0f;

  apply_delay_effect(&s, input, output, BUF_SIZE, 1);

  fails += ASSERT_FLOAT_EQ(output[D],     1.0f,        0.001f,
                           "feedback: first echo  amplitude = 1.0");
  fails += ASSERT_FLOAT_EQ(output[2 * D], FB,          0.001f,
                           "feedback: second echo amplitude = feedback^1");
  fails += ASSERT_FLOAT_EQ(output[3 * D], FB * FB,     0.001f,
                           "feedback: third echo  amplitude = feedback^2");

  delay_effect_cleanup(&s);
  return fails;
}

/* Dry/wet blend: mix=0.5 should produce 50% input + 50% delayed. */
static int test_drywet_blend(void)
{
  int fails = 0;
  AudioSettings s;
  delay_effect_init(&s, BUF_SIZE);

  const size_t D = 4;
  set_params(&s, 1.0f, 0.0f, 0.5f, D);  /* vol=1, feedback=0, mix=0.5 */

  float input[BUF_SIZE]  = {0};
  float output[BUF_SIZE] = {0};
  input[0] = 1.0f;

  apply_delay_effect(&s, input, output, BUF_SIZE, 1);

  /* Before the echo arrives: output = input*(1-0.5) + 0*0.5 */
  fails += ASSERT_FLOAT_EQ(output[0], 0.5f, 0.0001f,
                           "drywet 0.5: output[0] = 0.5*input (dry path)");

  /* At the echo: output = 0*(0.5) + 1*1*0.5 */
  fails += ASSERT_FLOAT_EQ(output[D], 0.5f, 0.0001f,
                           "drywet 0.5: output[D] = 0.5*delayed (wet path)");

  delay_effect_cleanup(&s);
  return fails;
}

/* ------------------------------------------------------------------ */
/* Runner                                                              */
/* ------------------------------------------------------------------ */

int main(void)
{
  int fails = 0;

  printf("\n--- delay_effect tests ---\n");

  printf("\n[test_init_defaults]\n");
  fails += test_init_defaults();

  printf("\n[test_bypass_passes_input_through]\n");
  fails += test_bypass_passes_input_through();

  printf("\n[test_dry_mix_passes_input_through]\n");
  fails += test_dry_mix_passes_input_through();

  printf("\n[test_impulse_delay_timing]\n");
  fails += test_impulse_delay_timing();

  printf("\n[test_volume_scales_wet_signal]\n");
  fails += test_volume_scales_wet_signal();

  printf("\n[test_feedback_echoes_decay]\n");
  fails += test_feedback_echoes_decay();

  printf("\n[test_drywet_blend]\n");
  fails += test_drywet_blend();

  printf("\n%s (%d failure%s)\n",
         fails == 0 ? "ALL PASS" : "FAILURES",
         fails, fails == 1 ? "" : "s");

  return fails > 0 ? 1 : 0;
}
