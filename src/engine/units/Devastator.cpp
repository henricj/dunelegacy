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

#include <units/Devastator.h>

#include <House.h>
#include <Game.h>
#include <Map.h>

#include <players/HumanPlayer.h>

namespace {
using namespace Dune::Engine;

constexpr TrackedUnitConstants devastator_constants{Devastator::item_id, 2, Bullet_ShellLarge};
}

namespace Dune::Engine {

Devastator::Devastator(uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(devastator_constants, objectID, initializer) {
    Devastator::init();

    Devastator::setHealth(initializer.game(), getMaxHealth(initializer.game()));

    devastateTimer = 0;
}

Devastator::Devastator(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(devastator_constants, objectID, initializer) {
    Devastator::init();

    auto& stream = initializer.stream();

    devastateTimer = stream.readSint32();
}

void Devastator::init() {
    assert(itemID == Unit_Devastator);
    owner->incrementUnits(itemID);
}

Devastator::~Devastator() = default;

void Devastator::save(const Game& game, OutputStream& stream) const {
    parent::save(game, stream);
    stream.writeSint32(devastateTimer);
}

void Devastator::doStartDevastate() {
    if(devastateTimer <= 0) devastateTimer = 200;
}

void Devastator::destroy(const GameContext& context) {
    if(context.map.tileExists(location) && isVisible()) {
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                Coord realPos(lround(realX) + (i - 1) * TILESIZE, lround(realY) + (j - 1) * TILESIZE);

                context.map.damage(context, objectID, owner, realPos, itemID, 150, 16, false);

                uint32_t explosionID = context.game.randomGen.getRandOf(Explosion_Large1, Explosion_Large2);
                context.game.addExplosion(explosionID, realPos, owner->getHouseID());
            }
        }

        if(isVisible(getOwner()->getTeamID())) { }
    }

    parent::destroy(context);
}

bool Devastator::update(const GameContext& context) {
    if(active) {
        if((devastateTimer > 0) && (--devastateTimer == 0)) {
            destroy(context);
            return false;
        }
    }

    return UnitBase::update(context);
}

} // namespace Dune::Engine

