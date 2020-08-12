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

#ifndef CHOAM_H
#define CHOAM_H

#include <structures/BuilderBase.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

#include <vector>

namespace Dune::Engine {

class House;

class Choam final {
public:
    explicit Choam(House* pHouse);
    ~Choam();

    void save(OutputStream& stream) const;
    void load(InputStream& stream);

    [[nodiscard]] int  getPrice(ItemID_enum itemID) const;
    [[nodiscard]] bool isCheap(ItemID_enum itemID) const;
    [[nodiscard]] int  getNumAvailable(ItemID_enum itemID) const;

    bool setNumAvailable(ItemID_enum itemID, int newValue);

    void addItem(ItemID_enum itemID, int num);

    void update(const GameContext& context);

private:
    House*                 house;          ///< The house of this choam
    std::vector<BuildItem> availableItems; ///< This list contains all the things that can be bought from a Starport
};

} // namespace Dune::Engine

#endif // CHOAM_H
