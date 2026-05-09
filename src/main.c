#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdatomic.h>
#include "link_bridge.h"
#include "delay_effect.h"

static _Atomic(bool) g_shutdown = false;

/**
 * @brief Signal handler for graceful shutdown.
 */
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
  printf("Initializing Link session...\n");

  /* Initialize Link with 120 BPM and 48kHz sample rate */
  void *linkHandle = link_init(120.0, 48000);
  if (!linkHandle)
  {
    fprintf(stderr, "Failed to initialize Link.\n");
    return 1;
  }

  printf("✓ Link initialized successfully.\n\n");

  /* Set up signal handler for Ctrl+C */
  signal(SIGINT, signal_handler);

  /* Interactive CLI loop */
  printf("Commands:\n");
  printf("  b              - Toggle bypass\n");
  printf("  v <float>      - Set volume (0.0-1.0)\n");
  printf("  f <float>      - Set feedback (0.0-0.99)\n");
  printf("  s              - Show status (BPM, peers)\n");
  printf("  q              - Quit\n");
  printf("\n");

  while (!atomic_load(&g_shutdown))
  {
    printf("> ");
    fflush(stdout);

    char line[256];
    if (!fgets(line, sizeof(line), stdin))
    {
      break;
    }

    /* Remove trailing newline */
    line[strcspn(line, "\n")] = '\0';

    if (strlen(line) == 0)
    {
      continue;
    }

    char cmd = line[0];

    switch (cmd)
    {
    case 'b':
    case 'B':
    {
      printf("Bypass not yet implemented.\n");
      break;
    }

    case 'v':
    case 'V':
    {
      float volume;
      if (sscanf(&line[1], "%f", &volume) == 1)
      {
        if (volume < 0.0f)
          volume = 0.0f;
        if (volume > 1.0f)
          volume = 1.0f;
        printf("Volume set to %.2f\n", volume);
      }
      else
      {
        printf("Usage: v <float>\n");
      }
      break;
    }

    case 'f':
    case 'F':
    {
      float feedback;
      if (sscanf(&line[1], "%f", &feedback) == 1)
      {
        if (feedback < 0.0f)
          feedback = 0.0f;
        if (feedback >= 0.99f)
          feedback = 0.99f;
        printf("Feedback set to %.2f\n", feedback);
      }
      else
      {
        printf("Usage: f <float>\n");
      }
      break;
    }

    case 's':
    case 'S':
    {
      double bpm;
      uint32_t numPeers;

      link_get_bpm(linkHandle, &bpm);
      numPeers = link_get_num_peers(linkHandle);

      printf("Status:\n");
      printf("  BPM: %.1f\n", bpm);
      printf("  Peers: %u\n", numPeers);
      break;
    }

    case 'q':
    case 'Q':
    {
      printf("Shutting down...\n");
      atomic_store(&g_shutdown, true);
      break;
    }

    default:
      printf("Unknown command: %c\n", cmd);
      break;
    }
  }

  /* Cleanup */
  printf("Cleaning up Link resources...\n");
  link_cleanup(linkHandle);
  printf("✓ Shutdown complete.\n");

  return 0;
}