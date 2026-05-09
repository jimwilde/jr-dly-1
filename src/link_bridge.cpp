/* Link bridge for C application.
 * Wraps Ableton Link v4 C++ API and exposes C interface.
 * Must be compiled with C++ compiler (g++, clang++).
 */

#define LINK_PLATFORM_MACOSX 1

#include <ableton/Link.hpp>
#include <memory>
#include <chrono>

#include "link_bridge.h"
#include "delay_effect.h"

using namespace ableton;

/**
 * @brief Internal state for Link manager.
 * Holds Link instance and shared audio settings.
 * Note: For MVP, we're using Link for timing synchronization only.
 * Full audio streaming (LinkAudioSource/Sink) can be added later.
 */
struct Link4Manager
{
  std::unique_ptr<Link> link;
  AudioSettings *audioSettings;
  uint32_t sampleRate;

  Link4Manager(double bpm, uint32_t sampleRate_)
      : sampleRate(sampleRate_)
  {
    link = std::make_unique<Link>(bpm);
    link->enable(true);
  }
};

static Link4Manager *g_mgr = nullptr;

extern "C"
{
  void *link_init(double bpm, uint32_t sampleRate)
  {
    if (g_mgr)
      return (void *)g_mgr;

    try
    {
      g_mgr = new Link4Manager(bpm, sampleRate);

      /* Allocate and initialize audio settings */
      g_mgr->audioSettings = (AudioSettings *)malloc(sizeof(AudioSettings));
      if (!g_mgr->audioSettings)
      {
        delete g_mgr;
        g_mgr = nullptr;
        return nullptr;
      }

      if (!delay_effect_init(g_mgr->audioSettings, sampleRate * 2))
      {
        free(g_mgr->audioSettings);
        delete g_mgr;
        g_mgr = nullptr;
        return nullptr;
      }

      return (void *)g_mgr;
    }
    catch (...)
    {
      return nullptr;
    }
  }

  void link_get_bpm(void *handle, double *outBpm)
  {
    if (!handle || !outBpm)
      return;

    Link4Manager *mgr = (Link4Manager *)handle;
    try
    {
      auto sessionState = mgr->link->captureAudioSessionState();
      *outBpm = sessionState.tempo();
    }
    catch (...)
    {
      *outBpm = 0.0;
    }
  }

  void link_get_beat(void *handle, double *outBeat, double quantum)
  {
    if (!handle || !outBeat)
      return;

    Link4Manager *mgr = (Link4Manager *)handle;
    try
    {
      auto sessionState = mgr->link->captureAudioSessionState();
      auto hostTime = mgr->link->clock().micros();
      *outBeat = sessionState.beatAtTime(hostTime, quantum);
    }
    catch (...)
    {
      *outBeat = 0.0;
    }
  }

  uint32_t link_get_num_peers(void *handle)
  {
    if (!handle)
      return 0;

    Link4Manager *mgr = (Link4Manager *)handle;
    try
    {
      return (uint32_t)mgr->link->numPeers();
    }
    catch (...)
    {
      return 0;
    }
  }

  void link_cleanup(void *handle)
  {
    if (!handle || handle != (void *)g_mgr)
      return;

    Link4Manager *mgr = (Link4Manager *)handle;
    if (mgr->audioSettings)
    {
      delay_effect_cleanup(mgr->audioSettings);
      free(mgr->audioSettings);
    }
    delete mgr;
    g_mgr = nullptr;
  }
}
