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

#include <FileClasses/music/MusicPlayer.h>
#include <SoundPlayer.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/SFXManager.h>
#include <FileClasses/TextManager.h>
#include <Network/NetworkManager.h>

#include <Bullet.h>
#include <globals.h>

// SDL stuff
SDL_Window* window;         ///< the window
SDL_Renderer* renderer;     ///< the renderer
SDL_Texture* screenTexture; ///< the texture
Palette palette;            ///< the palette for the screen
int drawnMouseX;            ///< the current mouse position (x coordinate)
int drawnMouseY;            ///< the current mouse position (y coordinate)
int cursorFrame;            ///< the current mouse cursor
int currentZoomlevel;       ///< 0 = the smallest zoom level, 1 = medium zoom level, 2 = maximum zoom level

// abstraction layers
std::unique_ptr<SoundPlayer> soundPlayer; ///< manager for playing sfx and voice
std::unique_ptr<MusicPlayer> musicPlayer; ///< manager for playing background music

std::unique_ptr<FileManager> pFileManager;       ///< manager for loading files from PAKs
std::unique_ptr<GFXManager> pGFXManager;         ///< manager for loading and managing graphics
std::unique_ptr<SFXManager> pSFXManager;         ///< manager for loading and managing sounds
std::unique_ptr<FontManager> pFontManager;       ///< manager for loading and managing fonts
std::unique_ptr<TextManager> pTextManager;       ///< manager for loading and managing texts and providing localization
std::unique_ptr<NetworkManager> pNetworkManager; ///< manager for all network events (nullptr if not in multiplayer game)

// game stuff
std::unique_ptr<Game> currentGame;          ///< the current running game
std::unique_ptr<ScreenBorder> screenborder; ///< the screen border for the current running game
Map* currentGameMap;                        ///< the map for the current running game
House* pLocalHouse;        ///< the house of the human player that is playing the current running game on this computer
HumanPlayer* pLocalPlayer; ///< the player that is playing the current running game on this computer

RobustList<UnitBase*> unitList;                  ///< the list of all units
RobustList<StructureBase*> structureList;        ///< the list of all structures
std::vector<std::unique_ptr<Bullet>> bulletList; ///< the list of all bullets

// misc
SettingsClass settings; ///< the settings read from the settings file

bool debug; ///< is set for debugging purposes
