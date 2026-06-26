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

#include <View/PublishedFrameBuilder.h>

#include <DataTypes.h>
#include <Definitions.h>
#include <Game.h>
#include <House.h>
#include <globals.h>
#include <structures/StructureBase.h>
#include <units/UnitBase.h>

void PublishedFrameBuilder::buildFrame(const Game& game) {
    // Lightweight reset: clears tick and each PlayerView's visibleObjects
    // without releasing any allocated capacity.
    frame_.reset();

    // One-time setup: allocate one PlayerView slot per possible house ID so
    // every slot can be addressed by index for the lifetime of the builder.
    if (frame_.playerViews.size() != static_cast<size_t>(NUM_HOUSES)) {
        frame_.playerViews.resize(NUM_HOUSES);
        for (auto i = 0; i < NUM_HOUSES; ++i)
            frame_.playerViews[static_cast<size_t>(i)].playerHouse = static_cast<HOUSETYPE>(i);
    }

    // Populate each active house's view with visibility-filtered object records.
    game.for_each_house([&](const House& house) {
        const auto houseIdx = static_cast<size_t>(static_cast<int>(house.getHouseID()));
        auto& pv            = frame_.playerViews[houseIdx];

        pv.credits       = static_cast<uint32_t>(house.getCredits());
        pv.powerProduced = static_cast<uint32_t>(house.getProducedPower());
        pv.powerRequired = static_cast<uint32_t>(house.getPowerRequirement());
        pv.spiceStored   = static_cast<uint32_t>(house.getCredits());
        pv.spiceCapacity = static_cast<uint32_t>(house.getCapacity());

        const int teamID = house.getTeamID();

        for (const auto* pUnit : dune::globals::unitList) {
            if (pUnit->isVisible(teamID))
                pUnit->appendPublicView(pv.visibleObjects.emplace_back());
        }

        for (const auto* pStructure : dune::globals::structureList) {
            if (pStructure->isVisible(teamID))
                pStructure->appendPublicView(pv.visibleObjects.emplace_back());
        }
    });

    frame_.tick = game.getGameCycleCount();
}
