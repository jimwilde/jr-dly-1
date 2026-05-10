#include <stdio.h>
#include <signal.h>
#include <stdatomic.h>
#include "link_bridge.h"
#include "menu.h"

static _Atomic(bool) g_shutdown = false;

void signal_handler(int sig)
{
  (void)sig;
  atomic_store(&g_shutdown, true);
}

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  printf("=== Ableton Link Delay Effect ===\n");
  printf("Initializing Link Audio session...\n");

  void *linkHandle = link_init(120.0, 48000);
  if (!linkHandle)
  {
    fprintf(stderr, "Failed to initialize Link.\n");
    return 1;
  }

  printf("✓ Link Audio initialized. Announcing 'Delay Output' channel.\n\n");

  signal(SIGINT, signal_handler);
  link_set_channels_changed_callback(linkHandle, on_channels_changed, NULL);

  run_menu(linkHandle, &g_shutdown);

  printf("Cleaning up...\n");
  link_cleanup(linkHandle);
  printf("✓ Shutdown complete.\n");

  return 0;
}
