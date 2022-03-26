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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <Colors.h>
#include <DataTypes.h>
#include <FileClasses/Palette.h>
#include <misc/RobustList.h>

#include <misc/SDL2pp.h>

#include <memory>

#define _(msgid) pTextManager->getLocalized(msgid)

// forward declarations
class SoundPlayer;
class MusicPlayer;

class FileManager;
class GFXManager;
class SFXManager;
class FontManager;
class TextManager;
class NetworkManager;

class Game;
class Map;
class ScreenBorder;
class House;
class HumanPlayer;
class UnitBase;
class StructureBase;
class Bullet;

// SDL stuff
extern SDL_Window* window;         ///< the window
extern SDL_Renderer* renderer;     ///< the renderer
extern SDL_Texture* screenTexture; ///< the texture
extern Palette palette;            ///< the palette for the screen
extern int drawnMouseX;            ///< the current mouse position (x coordinate)
extern int drawnMouseY;            ///< the current mouse position (y coordinate)
extern int cursorFrame;            ///< the current mouse cursor
extern int currentZoomlevel;       ///< 0 = the smallest zoom level, 1 = medium zoom level, 2 = maximum zoom level

// abstraction layers
extern std::unique_ptr<SoundPlayer> soundPlayer; ///< manager for playing sfx and voice
extern std::unique_ptr<MusicPlayer> musicPlayer; ///< manager for playing background music

extern std::unique_ptr<FileManager> pFileManager; ///< manager for loading files from PAKs
extern std::unique_ptr<GFXManager> pGFXManager;   ///< manager for loading and managing graphics
extern std::unique_ptr<SFXManager> pSFXManager;   ///< manager for loading and managing sounds
extern std::unique_ptr<FontManager> pFontManager; ///< manager for loading and managing fonts
extern std::unique_ptr<TextManager> pTextManager; ///< manager for loading and managing texts and providing localization
extern std::unique_ptr<NetworkManager>
    pNetworkManager; ///< manager for all network events (nullptr if not in multiplayer game)

// game stuff
extern std::unique_ptr<Game> currentGame;          ///< the current running game
extern std::unique_ptr<ScreenBorder> screenborder; ///< the screen border for the current running game
extern Map* currentGameMap;                        ///< the map for the current running game
extern House* pLocalHouse; ///< the house of the human player that is playing the current running game on this computer
extern HumanPlayer* pLocalPlayer; ///< the player that is playing the current running game on this computer

extern RobustList<UnitBase*> unitList;                  ///< the list of all units
extern RobustList<StructureBase*> structureList;        ///< the list of all structures
extern std::vector<std::unique_ptr<Bullet>> bulletList; ///< the list of all bullets

// misc
extern SettingsClass settings; ///< the settings read from the settings file

extern bool debug; ///< is set for debugging purposes

// constants
inline static constexpr int houseToPaletteIndex[static_cast<int>(HOUSETYPE::NUM_HOUSES)] = {
    PALCOLOR_HARKONNEN, PALCOLOR_ATREIDES,  PALCOLOR_ORDOS,
    PALCOLOR_FREMEN,    PALCOLOR_SARDAUKAR, PALCOLOR_MERCENARY};           ///< the base colors for the different houses
inline static constexpr char houseChar[] = {'H', 'A', 'O', 'F', 'S', 'M'}; ///< character for each house

#endif // GLOBALS_H
