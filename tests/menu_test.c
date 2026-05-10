#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include <unistd.h>
#include "menu.h"
#include "assert_utils.h"
#include "link_bridge_stub.h"

/* ------------------------------------------------------------------ */
/* I/O capture and injection helpers                                   */
/* ------------------------------------------------------------------ */

static int saved_stdout_fd;
static int out_pipefd[2];

static void begin_stdout_capture(void)
{
  pipe(out_pipefd);
  saved_stdout_fd = dup(STDOUT_FILENO);
  fflush(stdout);
  dup2(out_pipefd[1], STDOUT_FILENO);
  close(out_pipefd[1]);
}

static void end_stdout_capture(char *buf, size_t maxlen)
{
  fflush(stdout);
  dup2(saved_stdout_fd, STDOUT_FILENO);
  close(saved_stdout_fd);
  ssize_t n = read(out_pipefd[0], buf, (ssize_t)maxlen - 1);
  close(out_pipefd[0]);
  buf[n > 0 ? n : 0] = '\0';
}

/* Feed `input` to stdin, suppress stdout, then run run_menu and return. */
static void run_with_input(const char *input, _Atomic(bool) *shutdown)
{
  int in_pipefd[2];
  pipe(in_pipefd);
  int saved_stdin = dup(STDIN_FILENO);
  dup2(in_pipefd[0], STDIN_FILENO);
  close(in_pipefd[0]);
  write(in_pipefd[1], input, strlen(input));
  close(in_pipefd[1]);

  char devnull[4096];
  begin_stdout_capture();
  atomic_store(shutdown, false);
  run_menu(NULL, shutdown);
  end_stdout_capture(devnull, sizeof(devnull));

  dup2(saved_stdin, STDIN_FILENO);
  close(saved_stdin);
}

/* ------------------------------------------------------------------ */
/* print_menu tests                                                    */
/* ------------------------------------------------------------------ */

static int test_print_menu_nonempty(void)
{
  int fails = 0;
  char buf[4096];

  begin_stdout_capture();
  print_menu();
  end_stdout_capture(buf, sizeof(buf));

  fails += ASSERT_TRUE(strlen(buf) > 0, "menu: output is non-empty");
  return fails;
}

static int test_print_menu_lists_all_commands(void)
{
  int fails = 0;
  char buf[4096];

  begin_stdout_capture();
  print_menu();
  end_stdout_capture(buf, sizeof(buf));

  fails += ASSERT_TRUE(strstr(buf, "bypass")         != NULL, "menu: bypass listed");
  fails += ASSERT_TRUE(strstr(buf, "volume")         != NULL, "menu: volume listed");
  fails += ASSERT_TRUE(strstr(buf, "feedback")       != NULL, "menu: feedback listed");
  fails += ASSERT_TRUE(strstr(buf, "dry/wet")        != NULL, "menu: dry/wet listed");
  fails += ASSERT_TRUE(strstr(buf, "beats")          != NULL, "menu: beats listed");
  fails += ASSERT_TRUE(strstr(buf, "latency")        != NULL, "menu: latency listed");
  fails += ASSERT_TRUE(strstr(buf, "List available") != NULL, "menu: list channels listed");
  fails += ASSERT_TRUE(strstr(buf, "Connect")        != NULL, "menu: connect listed");
  fails += ASSERT_TRUE(strstr(buf, "Disconnect")     != NULL, "menu: disconnect listed");
  fails += ASSERT_TRUE(strstr(buf, "status")         != NULL, "menu: status listed");
  fails += ASSERT_TRUE(strstr(buf, "this menu")      != NULL, "menu: show menu listed");
  fails += ASSERT_TRUE(strstr(buf, "Quit")           != NULL, "menu: quit listed");

  return fails;
}

/* ------------------------------------------------------------------ */
/* on_channels_changed tests                                           */
/* ------------------------------------------------------------------ */

static int test_channels_changed_mentions_refresh(void)
{
  int fails = 0;
  char buf[512];

  begin_stdout_capture();
  on_channels_changed(NULL);
  end_stdout_capture(buf, sizeof(buf));

  fails += ASSERT_TRUE(strstr(buf, "channels") != NULL, "channels_changed: mentions 'channels'");
  fails += ASSERT_TRUE(strstr(buf, "refresh")  != NULL, "channels_changed: mentions 'refresh'");
  return fails;
}

static int test_channels_changed_ignores_context(void)
{
  int fails = 0;
  char buf_null[512];
  char buf_ptr[512];
  int dummy = 42;

  begin_stdout_capture();
  on_channels_changed(NULL);
  end_stdout_capture(buf_null, sizeof(buf_null));

  begin_stdout_capture();
  on_channels_changed(&dummy);
  end_stdout_capture(buf_ptr, sizeof(buf_ptr));

  fails += ASSERT_TRUE(strcmp(buf_null, buf_ptr) == 0,
                       "channels_changed: output identical regardless of context pointer");
  return fails;
}

/* ------------------------------------------------------------------ */
/* run_menu tests                                                      */
/* ------------------------------------------------------------------ */

static int test_run_menu_q_exits(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  run_with_input("q\n", &shutdown);

  fails += ASSERT_TRUE(atomic_load(&shutdown), "run_menu 'q': shutdown flag set");
  return fails;
}

static int test_run_menu_bypass_toggle(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  stub_reset();
  run_with_input("b\nq\n", &shutdown);

  fails += ASSERT_TRUE(stub_bypass == true, "run_menu 'b': bypass toggled on from false");
  return fails;
}

static int test_run_menu_volume(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  stub_reset();
  run_with_input("v 0.75\nq\n", &shutdown);

  fails += ASSERT_FLOAT_EQ(stub_volume, 0.75f, 0.001f, "run_menu 'v 0.75': volume set");
  return fails;
}

static int test_run_menu_feedback(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  stub_reset();
  run_with_input("f 0.4\nq\n", &shutdown);

  fails += ASSERT_FLOAT_EQ(stub_feedback, 0.4f, 0.001f, "run_menu 'f 0.4': feedback set");
  return fails;
}

static int test_run_menu_mix(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  stub_reset();
  run_with_input("w 0.6\nq\n", &shutdown);

  fails += ASSERT_FLOAT_EQ(stub_mix, 0.6f, 0.001f, "run_menu 'w 0.6': dry/wet mix set");
  return fails;
}

static int test_run_menu_delay_beats(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  stub_reset();
  run_with_input("t 2\nq\n", &shutdown);

  fails += ASSERT_FLOAT_EQ(stub_delay_beats, 2.0f, 0.001f, "run_menu 't 2': delay beats set");
  return fails;
}

static int test_run_menu_delay_beats_fraction(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  stub_reset();
  run_with_input("t 1/4\nq\n", &shutdown);

  fails += ASSERT_FLOAT_EQ(stub_delay_beats, 0.25f, 0.001f, "run_menu 't 1/4': fraction parsed correctly");
  return fails;
}

static int test_run_menu_latency(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  stub_reset();
  run_with_input("z 32\nq\n", &shutdown);

  fails += ASSERT_FLOAT_EQ(stub_manual_latency_ms, 32.0f, 0.1f, "run_menu 'z 32': latency offset set");
  return fails;
}

static int test_run_menu_volume_clamped(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  stub_reset();
  run_with_input("v 2.0\nq\n", &shutdown);

  fails += ASSERT_FLOAT_EQ(stub_volume, 1.0f, 0.001f, "run_menu 'v 2.0': volume clamped to 1.0");
  return fails;
}

static int test_run_menu_feedback_clamped(void)
{
  int fails = 0;
  _Atomic(bool) shutdown = false;

  stub_reset();
  run_with_input("f 1.5\nq\n", &shutdown);

  fails += ASSERT_TRUE(stub_feedback < 1.0f, "run_menu 'f 1.5': feedback clamped below 1.0");
  return fails;
}

/* ------------------------------------------------------------------ */
/* Runner                                                              */
/* ------------------------------------------------------------------ */

int main(void)
{
  int fails = 0;

  printf("\n--- menu tests ---\n");

  printf("\n[test_print_menu_nonempty]\n");
  fails += test_print_menu_nonempty();

  printf("\n[test_print_menu_lists_all_commands]\n");
  fails += test_print_menu_lists_all_commands();

  printf("\n[test_channels_changed_mentions_refresh]\n");
  fails += test_channels_changed_mentions_refresh();

  printf("\n[test_channels_changed_ignores_context]\n");
  fails += test_channels_changed_ignores_context();

  printf("\n[test_run_menu_q_exits]\n");
  fails += test_run_menu_q_exits();

  printf("\n[test_run_menu_bypass_toggle]\n");
  fails += test_run_menu_bypass_toggle();

  printf("\n[test_run_menu_volume]\n");
  fails += test_run_menu_volume();

  printf("\n[test_run_menu_feedback]\n");
  fails += test_run_menu_feedback();

  printf("\n[test_run_menu_mix]\n");
  fails += test_run_menu_mix();

  printf("\n[test_run_menu_delay_beats]\n");
  fails += test_run_menu_delay_beats();

  printf("\n[test_run_menu_delay_beats_fraction]\n");
  fails += test_run_menu_delay_beats_fraction();

  printf("\n[test_run_menu_latency]\n");
  fails += test_run_menu_latency();

  printf("\n[test_run_menu_volume_clamped]\n");
  fails += test_run_menu_volume_clamped();

  printf("\n[test_run_menu_feedback_clamped]\n");
  fails += test_run_menu_feedback_clamped();

  printf("\n%s (%d failure%s)\n",
         fails == 0 ? "ALL PASS" : "FAILURES",
         fails, fails == 1 ? "" : "s");

  return fails > 0 ? 1 : 0;
}
