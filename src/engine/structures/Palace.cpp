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

#include <structures/Palace.h>

#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Bullet.h>

#include <players/HumanPlayer.h>

#include <units/InfantryBase.h>
#include <units/Trooper.h>
#include <units/Saboteur.h>

namespace
{
constexpr int PALACE_DEATHHAND_WEAPONDAMAGE = 100;
} // namespace

namespace {
using namespace Dune::Engine;

class PalaceConstants : public StructureBaseConstants {
public:
    constexpr PalaceConstants() : StructureBaseConstants(Palace::item_id, Coord{3, 3}) { canAttackStuff_ = true; }
};

constexpr PalaceConstants palace_constants;
} // namespace

namespace Dune::Engine {

Palace::Palace(uint32_t objectID, const ObjectInitializer& initializer)
    : StructureBase(palace_constants, objectID, initializer) {
    Palace::init();

    Palace::setHealth(initializer.game(), getMaxHealth(initializer.game()));
    specialWeaponTimer = getMaxSpecialWeaponTimer();

    // TODO: Special weapon is available immediately but AI uses it only after first visual contact
    // specialTimer = 1; // we want the special weapon to be immediately ready
}

Palace::Palace(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : StructureBase(palace_constants, objectID, initializer) {
    Palace::init();

    auto& stream = initializer.stream();

    specialWeaponTimer = stream.readSint32();
}

void Palace::init() {
    assert(itemID == Structure_Palace);
    owner->incrementStructures(itemID);
}

Palace::~Palace() = default;

void Palace::save(const Game& game, OutputStream& stream) const {
    StructureBase::save(game, stream);
    stream.writeSint32(specialWeaponTimer);
}

void Palace::doSpecialWeapon(const GameContext& context) {
    if(!isSpecialWeaponReady()) { return; }

    switch(originalHouseID) {
        case HOUSETYPE::HOUSE_HARKONNEN:
        case HOUSETYPE::HOUSE_SARDAUKAR: {
            // wrong house (see DoLaunchDeathhand)
            return;
        }

        case HOUSETYPE::HOUSE_ATREIDES:
        case HOUSETYPE::HOUSE_FREMEN: {
            if(callFremen(context)) { specialWeaponTimer = getMaxSpecialWeaponTimer(); }
        } break;

        case HOUSETYPE::HOUSE_ORDOS:
        case HOUSETYPE::HOUSE_MERCENARY: {
            if(spawnSaboteur(context)) { specialWeaponTimer = getMaxSpecialWeaponTimer(); }
        } break;

        default: {
            THROW(std::runtime_error, "Palace::DoSpecialWeapon(): Invalid house");
        }
    }
}

void Palace::doLaunchDeathhand(const GameContext& context, int x, int y) {
    if(!isSpecialWeaponReady()) { return; }

    if((originalHouseID != HOUSETYPE::HOUSE_HARKONNEN) && (originalHouseID != HOUSETYPE::HOUSE_SARDAUKAR)) {
        // wrong house (see DoSpecialWeapon)
        return;
    }

    const auto randAngle = 2 * FixPt_PI * context.game.randomGen.randFixPoint();
    const auto radius    = context.game.randomGen.rand(0, 10 * TILESIZE);
    const auto deathOffX = lround(FixPoint::sin(randAngle) * radius);
    const auto deathOffY = lround(FixPoint::cos(randAngle) * radius);

    const auto  centerPoint = getCenterPoint();
    const Coord dest(x * TILESIZE + TILESIZE / 2 + deathOffX, y * TILESIZE + TILESIZE / 2 + deathOffY);

    context.game.add_bullet(context, objectID, &centerPoint, &dest, Bullet_LargeRocket, PALACE_DEATHHAND_WEAPONDAMAGE, false,
                           nullptr);

    specialWeaponTimer = getMaxSpecialWeaponTimer();
}

void Palace::updateStructureSpecificStuff(const GameContext& context) {
    if(specialWeaponTimer > 0) {
        --specialWeaponTimer;
        if(specialWeaponTimer <= 0) {
            specialWeaponTimer = 0;

            if(getOwner()->isAI()) {
                if((originalHouseID == HOUSETYPE::HOUSE_HARKONNEN) || (originalHouseID == HOUSETYPE::HOUSE_SARDAUKAR)) {
                    // Harkonnen and Sardaukar

                    // old targeting logic used by default AI
                    /*
                    const StructureBase* closestStructure = findClosestTargetStructure();
                    if(closestStructure) {
                        Coord temp = closestStructure->getClosestPoint(getLocation());
                        doLaunchDeathhand(temp.x, temp.y);
                    }*/
                } else {
                    // other houses
                    doSpecialWeapon(context);
                }
            }
        }
    }
}

bool Palace::callFremen(const GameContext& context) {
    int count = 0;
    int x     = 0;
    int y     = 0;

    auto& map       = context.map;
    auto& randomGen = context.game.randomGen;

    do {
        x = randomGen.rand(1, map.getSizeX() - 2);
        y = randomGen.rand(1, map.getSizeY() - 2);
    } while((map.getTile(x - 1, y - 1)->hasAGroundObject() || map.getTile(x, y - 1)->hasAGroundObject() ||
             map.getTile(x + 1, y - 1)->hasAGroundObject() || map.getTile(x - 1, y)->hasAGroundObject() ||
             map.getTile(x, y)->hasAGroundObject() || map.getTile(x + 1, y)->hasAGroundObject() ||
             map.getTile(x - 1, y + 1)->hasAGroundObject() || map.getTile(x, y + 1)->hasAGroundObject() ||
             map.getTile(x + 1, y + 1)->hasAGroundObject()) &&
            (count++ <= 1000));

    if(count < 1000) {

        for(int numFremen = 0; numFremen < 15; numFremen++) {
            if(randomGen.rand(0, 5) == 0) { continue; }

            auto* pFremen = getOwner()->createUnit<Trooper>();

            int i = 0;
            int j = 0;
            do {
                i = randomGen.rand(-1, 1);
                j = randomGen.rand(-1, 1);
            } while(!map.getTile(x + i, y + j)->infantryNotFull());

            pFremen->deploy(context, Coord(x + i, y + j));

            pFremen->doSetAttackMode(context, HUNT);
            pFremen->setRespondable(false);

            const auto* const closestStructure = pFremen->findClosestTargetStructure(context);
            if(closestStructure) {
                const auto closestPoint = closestStructure->getClosestPoint(pFremen->getLocation());
                pFremen->setGuardPoint(context, closestPoint);
                pFremen->setDestination(context, closestPoint);
            } else {
                const auto* const closestUnit = pFremen->findClosestTargetUnit(context);
                if(closestUnit) {
                    pFremen->setGuardPoint(context, closestUnit->getLocation());
                    pFremen->setDestination(context, closestUnit->getLocation());
                }
            }
        }

        return true;
    }

    return false;
}

bool Palace::spawnSaboteur(const GameContext& context) {
    auto*      saboteur = getOwner()->createUnit<Saboteur>();
    const auto spot     = context.map.findDeploySpot(saboteur, getLocation(), getDestination(), getStructureSize());

    saboteur->deploy(context, spot);

    if(getOwner()->isAI()) {
        saboteur->doSetAttackMode(context, HUNT);
    }

    return true;
}

} // namespace Dune::Engine
