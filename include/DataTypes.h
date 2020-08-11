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

#ifndef DATATYPES_H
#define DATATYPES_H

#include <engine/EngineDataTypes.h>
#include <Definitions.h>
#include <misc/SDL2pp.h>

// Libraries
#include <string>
#include <array>

struct DuneTexture;

typedef std::array<const DuneTexture*, NUM_ZOOMLEVEL> zoomable_texture;

#include <unordered_set>


class SettingsClass
{
public:
    class GeneralClass {
    public:
        bool            playIntro;          ///< Play the intro when starting the game?
        std::string     playerName;         ///< The name of the player used for multiplayer games
        std::string     language;           ///< Language code: "en" = English, "fr" = French, "de" = German
        int             scrollSpeed;        ///< Scroll speed in pixels
        bool            showTutorialHints;  ///< If true, tutorial hints are shown during the game
    } general;

    class VideoClass {
    public:
        bool        fullscreen;
        int         physicalWidth;
        int         physicalHeight;
        int         width;
        int         height;
        bool        frameLimit;
        int         preferredZoomLevel;
        std::string scaler;
        bool        rotateUnitGraphics;
        std::string renderer;
    } video;

    class AudioClass {
    public:
        bool        playSFX;
        int         sfxVolume;
        bool        playMusic;
        int         musicVolume;
        std::string musicType;
    } audio;

    class NetworkClass {
    public:
        int         serverPort;
        std::string metaServer;
        bool        debugNetwork;
    } network;

    class AIClass {
    public:
        std::string campaignAI;
    } ai;

    GameOptionsClass gameOptions;
};


namespace Dune {
    typedef Uint32 object_id_type;
    typedef std::unordered_set<object_id_type> selected_set_type;
}

#endif //DATATYPES_H
