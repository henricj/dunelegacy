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

#ifndef GAMECORE_H
#define GAMECORE_H

#include <array>
#include <cstdint>
#include <filesystem>

class Game;
class GameContext;
class InputStream;
class OutputStream;

class GameCore final {
public:
    explicit GameCore(Game& game) noexcept;

    void processObjects();
    void updateGame(const GameContext& context);
    void stepSimulation(uint32_t ticks);

    bool loadSaveGame(const std::filesystem::path& filename);
    bool loadSaveGame(InputStream& stream);
    bool saveGame(const std::filesystem::path& filename);

    void serializeCanonicalState(OutputStream& stream) const;
    [[nodiscard]] std::array<uint8_t, 48> computeStateHash() const;

private:
    Game& game_;
};

#endif // GAMECORE_H
