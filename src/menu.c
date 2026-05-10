#include <stdio.h>
#include "menu.h"

void print_menu(void)
{
  printf("Commands:\n");
  printf("  b              - Toggle bypass\n");
  printf("  v <float>      - Set volume (0.0-1.0)\n");
  printf("  f <float>      - Set feedback (0.0-0.99)\n");
  printf("  w <float>      - Set dry/wet mix (0.0=dry, 1.0=wet)\n");
  printf("  t <beats>      - Set delay time in beats (e.g. 1, 2, 1/2, 1/4)\n");
  printf("  z <ms>         - Manual latency offset in ms (+ = compensate more)\n");
  printf("  l              - List available Link Audio channels\n");
  printf("  c <N>          - Connect to channel N from list\n");
  printf("  d              - Disconnect current source\n");
  printf("  s              - Show status\n");
  printf("  m              - Show this menu\n");
  printf("  q              - Quit\n\n");
}

void on_channels_changed(void *context)
{
  (void)context;
  printf("\n[Link Audio channels changed — use 'l' to refresh]\n> ");
  fflush(stdout);
}
