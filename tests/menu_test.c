#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "menu.h"
#include "assert_utils.h"

/* ------------------------------------------------------------------ */
/* stdout capture helpers                                              */
/* ------------------------------------------------------------------ */

static int pipefd[2];
static int saved_stdout;

static void begin_capture(void)
{
  pipe(pipefd);
  saved_stdout = dup(STDOUT_FILENO);
  fflush(stdout);
  dup2(pipefd[1], STDOUT_FILENO);
}

static void end_capture(char *buf, size_t maxlen)
{
  fflush(stdout);
  dup2(saved_stdout, STDOUT_FILENO);
  close(saved_stdout);
  close(pipefd[1]);
  ssize_t n = read(pipefd[0], buf, (ssize_t)maxlen - 1);
  close(pipefd[0]);
  buf[n > 0 ? n : 0] = '\0';
}

/* ------------------------------------------------------------------ */
/* Tests                                                               */
/* ------------------------------------------------------------------ */

static int test_print_menu_lists_all_commands(void)
{
  int fails = 0;
  char buf[4096];

  begin_capture();
  print_menu();
  end_capture(buf, sizeof(buf));

  /* Check each command is described */
  fails += ASSERT_TRUE(strstr(buf, "bypass")          != NULL, "menu: bypass listed");
  fails += ASSERT_TRUE(strstr(buf, "volume")          != NULL, "menu: volume listed");
  fails += ASSERT_TRUE(strstr(buf, "feedback")        != NULL, "menu: feedback listed");
  fails += ASSERT_TRUE(strstr(buf, "dry/wet")         != NULL, "menu: dry/wet listed");
  fails += ASSERT_TRUE(strstr(buf, "beats")           != NULL, "menu: beats listed");
  fails += ASSERT_TRUE(strstr(buf, "latency")         != NULL, "menu: latency listed");
  fails += ASSERT_TRUE(strstr(buf, "List available")  != NULL, "menu: list channels listed");
  fails += ASSERT_TRUE(strstr(buf, "Connect")         != NULL, "menu: connect listed");
  fails += ASSERT_TRUE(strstr(buf, "Disconnect")      != NULL, "menu: disconnect listed");
  fails += ASSERT_TRUE(strstr(buf, "status")          != NULL, "menu: status listed");
  fails += ASSERT_TRUE(strstr(buf, "this menu")       != NULL, "menu: show menu listed");
  fails += ASSERT_TRUE(strstr(buf, "Quit")            != NULL, "menu: quit listed");

  return fails;
}

static int test_print_menu_nonempty(void)
{
  int fails = 0;
  char buf[4096];

  begin_capture();
  print_menu();
  end_capture(buf, sizeof(buf));

  fails += ASSERT_TRUE(strlen(buf) > 0, "menu: output is non-empty");
  return fails;
}

static int test_channels_changed_mentions_refresh(void)
{
  int fails = 0;
  char buf[512];

  begin_capture();
  on_channels_changed(NULL);
  end_capture(buf, sizeof(buf));

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

  begin_capture();
  on_channels_changed(NULL);
  end_capture(buf_null, sizeof(buf_null));

  begin_capture();
  on_channels_changed(&dummy);
  end_capture(buf_ptr, sizeof(buf_ptr));

  fails += ASSERT_TRUE(strcmp(buf_null, buf_ptr) == 0,
                       "channels_changed: output identical regardless of context pointer");
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

  printf("\n%s (%d failure%s)\n",
         fails == 0 ? "ALL PASS" : "FAILURES",
         fails, fails == 1 ? "" : "s");

  return fails > 0 ? 1 : 0;
}
