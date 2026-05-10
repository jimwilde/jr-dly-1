#define LINK_PLATFORM_MACOSX 1

#include <ableton/LinkAudio.hpp>

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

#include "delay_effect.h"
#include "link_bridge.h"

using namespace ableton;

struct Link4Manager
{
  LinkAudio link;
  LinkAudioSink sink;
  std::unique_ptr<LinkAudioSource> source;
  AudioSettings *audioSettings;
  uint32_t sampleRate;
  double quantum;

  std::atomic<float> delayBeats{1.0f};
  std::atomic<float> measuredLatencyMs{0.0f}; /* EMA of round-trip, auto-measured */
  std::atomic<float> manualLatencyMs{0.0f};   /* user fine-tune offset */

  /* Pre-allocated float conversion buffers (avoids per-callback heap alloc) */
  std::vector<float> inputBuf;
  std::vector<float> outputBuf;

  Link4Manager(double bpm, uint32_t sampleRate_)
    : link(bpm, "jr-dly-1")
    , sink(link, "Delay Output", 4096)
    , audioSettings(nullptr)
    , sampleRate(sampleRate_)
    , quantum(4.0)
  {
    link.enable(true);
    link.enableLinkAudio(true);
  }

  void onSourceBuffer(const LinkAudioSource::BufferHandle bufHandle)
  {
    if (!audioSettings)
      return;

    const auto &info = bufHandle.info;
    const size_t totalSamples = info.numFrames * info.numChannels;

    /* Capture session state once for timing measurement and commit */
    auto sessionState = link.captureAudioSessionState();

    if (info.tempo > 0.0)
    {
      /* Snapshot one-way latency for display — not applied to compensation */
      if (auto endBeat = info.endBeats(sessionState, quantum))
      {
        auto now = link.clock().micros();
        double currentBeat = sessionState.beatAtTime(now, quantum);
        double oneWayBeats = std::max(0.0, currentBeat - *endBeat);
        double roundTripMs = oneWayBeats * 60000.0 / info.tempo * 2.0;
        measuredLatencyMs.store(static_cast<float>(roundTripMs));
      }

      /* Beat-synced delay minus manual compensation — rounded to nearest frame */
      double beatDelayFrames =
        info.sampleRate * 60.0 / info.tempo * static_cast<double>(delayBeats.load());
      double compFrames =
        info.sampleRate * static_cast<double>(manualLatencyMs.load()) / 1000.0;
      double adjustedFrames = std::max(0.0, beatDelayFrames - compFrames);

      const size_t delaySamples =
        static_cast<size_t>(std::llround(adjustedFrames)) * info.numChannels;
      const size_t bufSize = audioSettings->bufferSizeInFrames;
      atomic_store(&audioSettings->delaySamples,
                   delaySamples < bufSize ? delaySamples : bufSize - 1);
    }

    /* Grow conversion buffers only when needed */
    if (inputBuf.size() < totalSamples)
    {
      inputBuf.resize(totalSamples);
      outputBuf.resize(totalSamples);
    }

    /* int16 → float */
    for (size_t i = 0; i < totalSamples; ++i)
      inputBuf[i] = bufHandle.samples[i] / 32768.0f;

    /* Apply delay */
    apply_delay_effect(audioSettings, inputBuf.data(), outputBuf.data(),
                       info.numFrames, static_cast<int>(info.numChannels));

    /* Retain sink buffer */
    auto bufferHandle = LinkAudioSink::BufferHandle(sink);
    if (!bufferHandle)
      return;

    if (bufferHandle.maxNumSamples < totalSamples)
    {
      sink.requestMaxNumSamples(totalSamples);
      return;
    }

    /* float → int16 with clamp */
    for (size_t i = 0; i < totalSamples; ++i)
    {
      float s = outputBuf[i];
      if (s > 1.0f) s = 1.0f;
      if (s < -1.0f) s = -1.0f;
      bufferHandle.samples[i] = static_cast<int16_t>(s * 32767.0f);
    }

    /* Commit tagged to the same beat position as the source buffer */
    double beginBeats = 0.0;
    if (auto beats = info.beginBeats(sessionState, quantum))
      beginBeats = *beats;

    bufferHandle.commit(sessionState, beginBeats, quantum,
                        info.numFrames, info.numChannels, info.sampleRate);
  }
};

static Link4Manager *g_mgr = nullptr;

extern "C"
{
  void *link_init(double bpm, uint32_t sampleRate)
  {
    if (g_mgr)
      return static_cast<void *>(g_mgr);

    try
    {
      g_mgr = new Link4Manager(bpm, sampleRate);

      g_mgr->audioSettings =
        static_cast<AudioSettings *>(malloc(sizeof(AudioSettings)));
      if (!g_mgr->audioSettings)
      {
        delete g_mgr;
        g_mgr = nullptr;
        return nullptr;
      }

      /* Buffer: 4 seconds of stereo headroom for slow tempos */
      if (!delay_effect_init(g_mgr->audioSettings, sampleRate * 4))
      {
        free(g_mgr->audioSettings);
        delete g_mgr;
        g_mgr = nullptr;
        return nullptr;
      }

      return static_cast<void *>(g_mgr);
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
    auto *mgr = static_cast<Link4Manager *>(handle);
    try
    {
      *outBpm = mgr->link.captureAppSessionState().tempo();
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
    auto *mgr = static_cast<Link4Manager *>(handle);
    try
    {
      auto state = mgr->link.captureAppSessionState();
      auto hostTime = mgr->link.clock().micros();
      *outBeat = state.beatAtTime(hostTime, quantum);
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
    auto *mgr = static_cast<Link4Manager *>(handle);
    try
    {
      return static_cast<uint32_t>(mgr->link.numPeers());
    }
    catch (...)
    {
      return 0;
    }
  }

  bool link_audio_is_enabled(void *handle)
  {
    if (!handle)
      return false;
    return static_cast<Link4Manager *>(handle)->link.isLinkAudioEnabled();
  }

  void link_audio_enable(void *handle, bool enabled)
  {
    if (!handle)
      return;
    static_cast<Link4Manager *>(handle)->link.enableLinkAudio(enabled);
  }

  bool link_has_source(void *handle)
  {
    if (!handle)
      return false;
    return static_cast<Link4Manager *>(handle)->source != nullptr;
  }

  uint32_t link_list_channels(void *handle, LinkAudioChannel *out, uint32_t max)
  {
    if (!handle || !out || max == 0)
      return 0;
    auto *mgr = static_cast<Link4Manager *>(handle);
    try
    {
      auto channels = mgr->link.channels();
      uint32_t count = 0;
      for (const auto &ch : channels)
      {
        if (count >= max)
          break;
        strncpy(out[count].name, ch.name.c_str(), 255);
        out[count].name[255] = '\0';
        strncpy(out[count].peer_name, ch.peerName.c_str(), 255);
        out[count].peer_name[255] = '\0';
        const auto &id = ch.id;
        std::copy(id.begin(), id.end(), out[count].id);
        ++count;
      }
      return count;
    }
    catch (...)
    {
      return 0;
    }
  }

  void link_subscribe_channel(void *handle, const uint8_t id[8])
  {
    if (!handle)
      return;
    auto *mgr = static_cast<Link4Manager *>(handle);
    try
    {
      ChannelId channelId{};
      std::copy(id, id + 8, channelId.begin());
      mgr->source = std::make_unique<LinkAudioSource>(
        mgr->link,
        channelId,
        [mgr](LinkAudioSource::BufferHandle bh)
        { mgr->onSourceBuffer(bh); });
    }
    catch (...)
    {
    }
  }

  void link_unsubscribe_channel(void *handle)
  {
    if (!handle)
      return;
    static_cast<Link4Manager *>(handle)->source.reset();
  }

  void link_set_volume(void *handle, float volume)
  {
    if (!handle) return;
    auto *mgr = static_cast<Link4Manager *>(handle);
    if (mgr->audioSettings)
      atomic_store(&mgr->audioSettings->volume, volume);
  }

  void link_set_feedback(void *handle, float feedback)
  {
    if (!handle) return;
    auto *mgr = static_cast<Link4Manager *>(handle);
    if (mgr->audioSettings)
      atomic_store(&mgr->audioSettings->feedback, feedback);
  }

  void link_set_mix(void *handle, float mix)
  {
    if (!handle) return;
    auto *mgr = static_cast<Link4Manager *>(handle);
    if (mgr->audioSettings)
      atomic_store(&mgr->audioSettings->mix, mix);
  }

  void link_set_bypass(void *handle, bool bypass)
  {
    if (!handle) return;
    auto *mgr = static_cast<Link4Manager *>(handle);
    if (mgr->audioSettings)
      atomic_store(&mgr->audioSettings->bypass, bypass);
  }

  bool link_get_bypass(void *handle)
  {
    if (!handle) return false;
    auto *mgr = static_cast<Link4Manager *>(handle);
    if (!mgr->audioSettings) return false;
    return atomic_load(&mgr->audioSettings->bypass);
  }

  void link_set_delay_beats(void *handle, float beats)
  {
    if (!handle || beats <= 0.0f)
      return;
    static_cast<Link4Manager *>(handle)->delayBeats.store(beats);
  }

  float link_get_delay_beats(void *handle)
  {
    if (!handle)
      return 1.0f;
    return static_cast<Link4Manager *>(handle)->delayBeats.load();
  }

  float link_get_measured_latency_ms(void *handle)
  {
    if (!handle) return 0.0f;
    return static_cast<Link4Manager *>(handle)->measuredLatencyMs.load();
  }

  void link_set_manual_latency_ms(void *handle, float ms)
  {
    if (!handle) return;
    static_cast<Link4Manager *>(handle)->manualLatencyMs.store(ms);
  }

  float link_get_manual_latency_ms(void *handle)
  {
    if (!handle) return 0.0f;
    return static_cast<Link4Manager *>(handle)->manualLatencyMs.load();
  }

  void link_set_channels_changed_callback(void *handle,
                                          void (*callback)(void *context),
                                          void *context)
  {
    if (!handle)
      return;
    auto *mgr = static_cast<Link4Manager *>(handle);
    mgr->link.setChannelsChangedCallback(
      [callback, context]() { callback(context); });
  }

  void link_cleanup(void *handle)
  {
    if (!handle || handle != static_cast<void *>(g_mgr))
      return;
    auto *mgr = static_cast<Link4Manager *>(handle);
    mgr->source.reset();
    if (mgr->audioSettings)
    {
      delay_effect_cleanup(mgr->audioSettings);
      free(mgr->audioSettings);
    }
    delete mgr;
    g_mgr = nullptr;
  }
}
