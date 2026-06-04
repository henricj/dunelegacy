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

#include <Game.h>

#include <config.h>
#include <globals.h>

#include <Bullet.h>
#include <Explosion.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <misc/IFileStream.h>
#include <misc/OFileStream.h>
#include <misc/OHashStream.h>
#include <misc/dune_sdlpp.h>
#include <misc/string_error.h>
#include <players/HumanPlayer.h>

bool Game::loadSaveGame(const std::filesystem::path& filename) {
    return core_.loadSaveGame(filename);
}

bool Game::loadSaveGame(InputStream& stream) {
    return core_.loadSaveGame(stream);
}

bool Game::saveGame(const std::filesystem::path& filename) {
    return core_.saveGame(filename);
}

void Game::saveObject(OutputStream& stream, ObjectBase* obj) {
    if (obj == nullptr)
        return;

    stream.writeUint32(obj->getItemID());
    obj->save(stream);
}

void Game::serializeCanonicalState(OutputStream& stream) const {
    core_.serializeCanonicalState(stream);
}

std::array<uint8_t, 48> Game::computeStateHash() const {
    return core_.computeStateHash();
}
