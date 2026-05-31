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

#pragma once

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <memory>

// =============================================================================
// Deleters and owning handle aliases for SDL3_mixer audio resources
// =============================================================================

/// Deleter for MIX_Audio objects.  Calls MIX_DestroyAudio directly so it is
/// safe to invoke even when no AudioEngine instance is alive.
struct MixAudioDeleter {
    void operator()(MIX_Audio* audio) const noexcept {
        if (audio)
            MIX_DestroyAudio(audio);
    }
};

/// Deleter for MIX_Track objects.  Calls MIX_DestroyTrack directly so it is
/// safe to invoke even when no AudioEngine instance is alive.
struct MixTrackDeleter {
    void operator()(MIX_Track* track) const noexcept {
        if (track)
            MIX_DestroyTrack(track);
    }
};

/// Canonical owning handle for a MIX_Audio resource.
using mix_audio_ptr = std::unique_ptr<MIX_Audio, MixAudioDeleter>;

/// Canonical owning handle for a MIX_Track resource.
using mix_track_ptr = std::unique_ptr<MIX_Track, MixTrackDeleter>;

// =============================================================================
// AudioEngine interface
// =============================================================================

/**
 * Abstract interface for the runtime audio backend.
 *
 * Downstream subsystems (SFXManager, SoundPlayer, music players) depend on
 * this seam instead of calling SDL3_mixer or the Mix_* compat stubs directly.
 * The only concrete implementation is SDLAudioEngine (see src/Audio/).
 *
 * Factory methods (createTrack, loadAudioIO, loadRawAudio) return owning
 * handles (mix_track_ptr / mix_audio_ptr).  All other methods that accept
 * MIX_Track* or MIX_Audio* parameters treat them as non-owning borrows.
 */
class AudioEngine {
public:
    /// The SDL3_mixer tag applied to all background-music tracks.
    static constexpr const char* kMusicTag = "music";

    AudioEngine()                              = default;
    virtual ~AudioEngine()                     = default;
    AudioEngine(const AudioEngine&)            = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;
    AudioEngine(AudioEngine&&)                 = delete;
    AudioEngine& operator=(AudioEngine&&)      = delete;

    // -----------------------------------------------------------------------
    // Device state

    /// Returns true when the audio device was successfully opened.
    [[nodiscard]] virtual bool isOpen() const noexcept = 0;

    /// Fills \p spec with the mixer's operating format.  Returns false when
    /// the device is not open.
    [[nodiscard]] virtual bool getFormat(SDL_AudioSpec& spec) const noexcept = 0;

    // -----------------------------------------------------------------------
    // Master gain  [0.0f = silence, 1.0f = unity, >1.0f amplifies]

    virtual bool setMasterGain(float gain) noexcept            = 0;
    [[nodiscard]] virtual float getMasterGain() const noexcept = 0;

    // -----------------------------------------------------------------------
    // Track management

    /// Allocate a new track on the mixer.  Returns an empty handle on failure.
    [[nodiscard]] virtual mix_track_ptr createTrack() = 0;

    /// Associate a track with a named group (e.g., kMusicTag).
    virtual bool tagTrack(MIX_Track* track, const char* tag) noexcept = 0;

    /// Remove a tag association from a track.
    virtual void untagTrack(MIX_Track* track, const char* tag) noexcept = 0;

    // -----------------------------------------------------------------------
    // Audio data management

    /// Load audio from an SDL_IOStream.  When \p closeio is true the stream
    /// is closed (and freed) by SDL_mixer regardless of success.
    /// Returns an empty handle on failure.
    [[nodiscard]] virtual mix_audio_ptr loadAudioIO(SDL_IOStream* io, bool predecode, bool closeio) = 0;

    /// Load raw PCM audio from an in-memory buffer (data is copied).
    /// Returns an empty handle on failure.
    [[nodiscard]] virtual mix_audio_ptr loadRawAudio(const void* data, size_t datalen, const SDL_AudioSpec& spec) = 0;

    // -----------------------------------------------------------------------
    // Track input sources

    /// Assign a pre-loaded MIX_Audio as the track's audio source.
    virtual bool setTrackAudio(MIX_Track* track, MIX_Audio* audio) noexcept = 0;

    /// Assign an SDL_AudioStream as the track's streaming PCM source.
    /// Used by the ADL OPL synthesiser; the stream must remain valid until
    /// the track's source is changed or the track is destroyed.
    virtual bool setTrackAudioStream(MIX_Track* track, SDL_AudioStream* stream) noexcept = 0;

    /// Assign an SDL_IOStream as the track's audio source.  When \p closeio
    /// is true the stream is closed when the track is stopped or destroyed.
    virtual bool setTrackIOStream(MIX_Track* track, SDL_IOStream* io, bool closeio) noexcept = 0;

    // -----------------------------------------------------------------------
    // Playback control

    /// Start playing a track.  Pass 0 for \p options to use all defaults;
    /// use MIX_PROP_PLAY_LOOPS_NUMBER etc. to customise behaviour.
    virtual bool playTrack(MIX_Track* track, SDL_PropertiesID options = 0) noexcept = 0;

    /// Stop a track, optionally fading out over \p fade_out_frames sample frames.
    virtual bool stopTrack(MIX_Track* track, Sint64 fade_out_frames = 0) noexcept = 0;

    /// Returns true when the track is actively contributing to the mix.
    [[nodiscard]] virtual bool isTrackPlaying(MIX_Track* track) const noexcept = 0;

    // -----------------------------------------------------------------------
    // Per-track gain  [0.0f = silence, 1.0f = unity]

    virtual bool setTrackGain(MIX_Track* track, float gain) noexcept          = 0;
    [[nodiscard]] virtual float getTrackGain(MIX_Track* track) const noexcept = 0;

    // -----------------------------------------------------------------------
    // Tag-based bulk operations  (used for sound groups and music)

    /// Set gain on every track that carries \p tag.
    virtual bool setTagGain(const char* tag, float gain) noexcept = 0;

    /// Stop all tracks that carry \p tag, with an optional fade-out in
    /// milliseconds.  0 = immediate stop.
    virtual bool stopTag(const char* tag, Sint64 fade_out_ms = 0) noexcept = 0;

    // -----------------------------------------------------------------------
    // Music convenience helpers  (operate on tracks tagged kMusicTag)

    /// Returns true when at least one kMusicTag track is actively playing.
    [[nodiscard]] virtual bool isMusicPlaying() const noexcept = 0;

    /// Stop all kMusicTag tracks with an optional fade-out in milliseconds.
    virtual bool stopMusic(Sint64 fade_out_ms = 0) noexcept = 0;

    /// Set gain for all kMusicTag tracks.
    virtual bool setMusicGain(float gain) noexcept = 0;

    // -----------------------------------------------------------------------
    // Fire-and-forget playback  (no track management required)

    /// Play \p audio once with no track lifecycle management.
    virtual bool playAudio(MIX_Audio* audio) noexcept = 0;

    // -----------------------------------------------------------------------
    // Factory

    /// Create the SDL3_mixer-backed implementation wired to the default
    /// playback device.  Returns nullptr (and logs) on failure; never throws.
    [[nodiscard]] static std::unique_ptr<AudioEngine> createSDL3Backend();
};
