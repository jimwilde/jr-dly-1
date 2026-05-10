#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include "link_bridge.h"
#include "menu.h"

#define MAX_CHANNELS 64

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

void run_menu(void *linkHandle, _Atomic(bool) *shutdown)
{
  print_menu();

  while (!atomic_load(shutdown))
  {
    printf("> ");
    fflush(stdout);

    char line[256];
    if (!fgets(line, sizeof(line), stdin))
      break;

    line[strcspn(line, "\n")] = '\0';
    if (strlen(line) == 0)
      continue;

    char cmd = line[0];

    switch (cmd)
    {
    case 'm':
    case 'M':
    {
      print_menu();
      break;
    }

    case 'b':
    case 'B':
    {
      bool bypass = !link_get_bypass(linkHandle);
      link_set_bypass(linkHandle, bypass);
      printf("Bypass %s\n", bypass ? "on" : "off");
      break;
    }

    case 'v':
    case 'V':
    {
      float volume;
      if (sscanf(&line[1], "%f", &volume) == 1)
      {
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;
        link_set_volume(linkHandle, volume);
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
        if (feedback < 0.0f) feedback = 0.0f;
        if (feedback >= 0.99f) feedback = 0.99f;
        link_set_feedback(linkHandle, feedback);
        printf("Feedback set to %.2f\n", feedback);
      }
      else
      {
        printf("Usage: f <float>\n");
      }
      break;
    }

    case 'w':
    case 'W':
    {
      float mix;
      if (sscanf(&line[1], "%f", &mix) == 1)
      {
        if (mix < 0.0f) mix = 0.0f;
        if (mix > 1.0f) mix = 1.0f;
        link_set_mix(linkHandle, mix);
        printf("Dry/wet mix set to %.2f\n", mix);
      }
      else
      {
        printf("Usage: w <float>  (0.0=dry, 1.0=wet)\n");
      }
      break;
    }

    case 't':
    case 'T':
    {
      float num, den;
      float beats;
      if (sscanf(&line[1], "%f/%f", &num, &den) == 2)
        beats = num / den;
      else if (sscanf(&line[1], "%f", &beats) != 1)
      {
        printf("Usage: t <beats>  e.g. t 1  t 2  t 1/2  t 1/4\n");
        break;
      }
      if (beats <= 0.0f)
      {
        printf("Beats must be greater than 0.\n");
        break;
      }
      link_set_delay_beats(linkHandle, beats);
      printf("Delay set to %.4g beat(s)\n", beats);
      break;
    }

    case 'z':
    case 'Z':
    {
      float ms;
      if (sscanf(&line[1], "%f", &ms) == 1)
      {
        link_set_manual_latency_ms(linkHandle, ms);
        printf("Manual latency offset set to %.1f ms\n", ms);
      }
      else
      {
        printf("Usage: z <ms>  (e.g. z 10  z -5  z 0)\n");
      }
      break;
    }

    case 'l':
    case 'L':
    {
      LinkAudioChannel channels[MAX_CHANNELS];
      uint32_t count = link_list_channels(linkHandle, channels, MAX_CHANNELS);
      if (count == 0)
      {
        printf("No Link Audio channels found. Make sure Ableton Live has Link\n");
        printf("enabled and Link Audio is active.\n");
      }
      else
      {
        printf("Available channels (%u):\n", count);
        for (uint32_t i = 0; i < count; ++i)
          printf("  %u: %s / %s\n", i, channels[i].peer_name, channels[i].name);
      }
      break;
    }

    case 'c':
    case 'C':
    {
      unsigned int idx;
      if (sscanf(&line[1], "%u", &idx) != 1)
      {
        printf("Usage: c <N>\n");
        break;
      }

      LinkAudioChannel channels[MAX_CHANNELS];
      uint32_t count = link_list_channels(linkHandle, channels, MAX_CHANNELS);
      if (count == 0)
      {
        printf("No channels available. Use 'l' to list.\n");
        break;
      }
      if (idx >= count)
      {
        printf("Index out of range (0-%u).\n", count - 1);
        break;
      }

      link_unsubscribe_channel(linkHandle);
      link_subscribe_channel(linkHandle, channels[idx].id);
      printf("Connected to: %s / %s\n",
             channels[idx].peer_name, channels[idx].name);
      printf("Delay output announced as 'Delay Output' on the Link session.\n");
      break;
    }

    case 'd':
    case 'D':
    {
      link_unsubscribe_channel(linkHandle);
      printf("Disconnected.\n");
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
      printf("  BPM:          %.1f\n", bpm);
      printf("  Peers:        %u\n", numPeers);
      printf("  Audio:        %s\n",
             link_audio_is_enabled(linkHandle) ? "enabled" : "disabled");
      printf("  Source:       %s\n",
             link_has_source(linkHandle) ? "connected" : "none");
      printf("  Delay:        %.4g beat(s)\n",
             link_get_delay_beats(linkHandle));
      printf("  Latency:      buffer %.1f ms  offset %+.1f ms\n",
             link_get_buffer_latency_ms(linkHandle),
             link_get_manual_latency_ms(linkHandle));
      break;
    }

    case 'q':
    case 'Q':
    {
      printf("Shutting down...\n");
      atomic_store(shutdown, true);
      break;
    }

    default:
      printf("Unknown command: %c\n", cmd);
      break;
    }
  }
}
