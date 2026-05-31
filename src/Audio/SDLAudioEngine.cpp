/*
 *  This file is part of Dune Legacy.
 *
 *  Dune Legacy is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Dune Legacy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Dune Legacy.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Audio/AudioEngine.h>

#include <misc/dune_sdlpp.h>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

namespace {

class SDLAudioEngine final : public AudioEngine {
public:
    explicit SDLAudioEngine(MIX_Mixer* mixer) noexcept : mixer_(mixer) { }

    ~SDLAudioEngine() override {
        if (mixer_) {
            MIX_DestroyMixer(mixer_);
            mixer_ = nullptr;
        }
    }

    // -------------------------------------------------------------------
    // Device state

    [[nodiscard]] bool isOpen() const noexcept override { return mixer_ != nullptr; }

    [[nodiscard]] bool getFormat(SDL_AudioSpec& spec) const noexcept override {
        if (!mixer_)
            return false;
        return MIX_GetMixerFormat(mixer_, &spec);
    }

    // -------------------------------------------------------------------
    // Master gain

    bool setMasterGain(float gain) noexcept override {
        if (!mixer_)
            return false;
        return MIX_SetMixerGain(mixer_, gain);
    }

    [[nodiscard]] float getMasterGain() const noexcept override {
        if (!mixer_)
            return 1.0f;
        return MIX_GetMixerGain(mixer_);
    }

    // -------------------------------------------------------------------
    // Track management

    [[nodiscard]] mix_track_ptr createTrack() override {
        if (!mixer_)
            return nullptr;
        return mix_track_ptr{MIX_CreateTrack(mixer_)};
    }

    bool tagTrack(MIX_Track* track, const char* tag) noexcept override {
        if (!track || !tag)
            return false;
        return MIX_TagTrack(track, tag);
    }

    void untagTrack(MIX_Track* track, const char* tag) noexcept override {
        if (track)
            MIX_UntagTrack(track, tag);
    }

    // -------------------------------------------------------------------
    // Audio data management

    [[nodiscard]] mix_audio_ptr loadAudioIO(SDL_IOStream* io, bool predecode, bool closeio) override {
        return mix_audio_ptr{MIX_LoadAudio_IO(mixer_, io, predecode, closeio)};
    }

    [[nodiscard]] mix_audio_ptr loadRawAudio(const void* data, size_t datalen, const SDL_AudioSpec& spec) override {
        return mix_audio_ptr{MIX_LoadRawAudio(mixer_, data, datalen, &spec)};
    }

    // -------------------------------------------------------------------
    // Track input sources

    bool setTrackAudio(MIX_Track* track, MIX_Audio* audio) noexcept override {
        if (!track)
            return false;
        return MIX_SetTrackAudio(track, audio);
    }

    bool setTrackAudioStream(MIX_Track* track, SDL_AudioStream* stream) noexcept override {
        if (!track)
            return false;
        return MIX_SetTrackAudioStream(track, stream);
    }

    bool setTrackIOStream(MIX_Track* track, SDL_IOStream* io, bool closeio) noexcept override {
        if (!track)
            return false;
        return MIX_SetTrackIOStream(track, io, closeio);
    }

    // -------------------------------------------------------------------
    // Playback control

    bool playTrack(MIX_Track* track, SDL_PropertiesID options) noexcept override {
        if (!track)
            return false;
        return MIX_PlayTrack(track, options);
    }

    bool stopTrack(MIX_Track* track, Sint64 fade_out_frames) noexcept override {
        if (!track)
            return false;
        return MIX_StopTrack(track, fade_out_frames);
    }

    [[nodiscard]] bool isTrackPlaying(MIX_Track* track) const noexcept override {
        if (!track)
            return false;
        return MIX_TrackPlaying(track);
    }

    // -------------------------------------------------------------------
    // Per-track gain

    bool setTrackGain(MIX_Track* track, float gain) noexcept override {
        if (!track)
            return false;
        return MIX_SetTrackGain(track, gain);
    }

    [[nodiscard]] float getTrackGain(MIX_Track* track) const noexcept override {
        if (!track)
            return 1.0f;
        return MIX_GetTrackGain(track);
    }

    // -------------------------------------------------------------------
    // Tag-based bulk operations

    bool setTagGain(const char* tag, float gain) noexcept override {
        if (!mixer_ || !tag)
            return false;
        return MIX_SetTagGain(mixer_, tag, gain);
    }

    bool stopTag(const char* tag, Sint64 fade_out_ms) noexcept override {
        if (!mixer_ || !tag)
            return false;
        return MIX_StopTag(mixer_, tag, fade_out_ms);
    }

    // -------------------------------------------------------------------
    // Music convenience helpers

    [[nodiscard]] bool isMusicPlaying() const noexcept override {
        if (!mixer_)
            return false;
        int count                = 0;
        MIX_Track** const tracks = MIX_GetTaggedTracks(mixer_, kMusicTag, &count);
        if (!tracks || count == 0)
            return false;
        bool playing = false;
        for (int i = 0; i < count && !playing; ++i)
            playing = MIX_TrackPlaying(tracks[i]);
        SDL_free(tracks);
        return playing;
    }

    bool stopMusic(Sint64 fade_out_ms) noexcept override { return stopTag(kMusicTag, fade_out_ms); }

    bool setMusicGain(float gain) noexcept override { return setTagGain(kMusicTag, gain); }

    // -------------------------------------------------------------------
    // Fire-and-forget playback

    bool playAudio(MIX_Audio* audio) noexcept override {
        if (!mixer_ || !audio)
            return false;
        return MIX_PlayAudio(mixer_, audio);
    }

private:
    MIX_Mixer* mixer_ = nullptr;
};

} // namespace

std::unique_ptr<AudioEngine> AudioEngine::createSDL3Backend() {
    MIX_Mixer* mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!mixer) {
        sdl2::log_error("AudioEngine: failed to open audio device: {}", SDL_GetError());
        return nullptr;
    }
    sdl2::log_info("AudioEngine: SDL3 backend opened");
    return std::make_unique<SDLAudioEngine>(mixer);
}
