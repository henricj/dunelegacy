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

#include <units/Saboteur.h>

#include <globals.h>

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

namespace {
class SaboteurConstants : public InfantryBaseConstants {
public:
    constexpr SaboteurConstants() : InfantryBaseConstants{Saboteur::item_id} { canAttackStuff_ = true; }
};

constexpr SaboteurConstants saboteur_constants;
} // namespace

Saboteur::Saboteur(uint32_t objectID, const ObjectInitializer& initializer)
    : InfantryBase(saboteur_constants, objectID, initializer) {
    Saboteur::init();

    Saboteur::setHealth(getMaxHealth());

    setVisible(VIS_ALL, false);
    setVisible(getOwner()->getTeamID(), true);
    attackMode_ = GUARD;
}

Saboteur::Saboteur(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : InfantryBase(saboteur_constants, objectID, initializer) {
    Saboteur::init();
}

void Saboteur::init() {
    assert(itemID_ == Unit_Saboteur);
    owner_->incrementUnits(itemID_);

    graphicID_ = ObjPic_Saboteur;
    graphic_   = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());

    numImagesX_ = 4;
    numImagesY_ = 3;
}

Saboteur::~Saboteur() = default;

void Saboteur::checkPos(const GameContext& context) {
    parent::checkPos(context);

    if (!active_)
        return;

    std::array<bool, NUM_TEAMS> canBeSeen{};
    context.map.for_each(location_.x - 2, location_.x + 3, location_.y - 2, location_.y + 3, [&](const auto& tile) {
        if (const auto* const object = tile.getObject(context.objectManager)) {
            if (const auto* const owner = object->getOwner())
                canBeSeen[owner->getTeamID()] = true;
        }
    });

    setVisible(getOwner()->getTeamID(), true); // owner team can always see it
    // setVisible(pLocalHouse->getTeamID(), true);
}

bool Saboteur::update(const GameContext& context) {
    if (active_ && !moving) {
        // check to see if close enough to blow up target
        if (auto* pObject = target_.getObjPointer()) { //&& target.getObjPointer()->isAStructure()
            if (getOwner()->getTeamID() != pObject->getOwner()->getTeamID()) {
                const auto closestPoint = pObject->getClosestPoint(location_);

                if (blockDistance(location_, closestPoint) <= 1.5_fix) {
                    if (isVisible(getOwner()->getTeamID())) {
                        dune::globals::screenborder->shakeScreen(18);
                    }
                }

                destroy(context);
                pObject->setHealth(0);
                pObject->destroy(context);
                return false;
            }
        }
    }

    return parent::update(context);
}

void Saboteur::deploy(const GameContext& context, const Coord& newLocation) {
    parent::deploy(context, newLocation);

    setVisible(VIS_ALL, false);
    setVisible(getOwner()->getTeamID(), true);
}

bool Saboteur::canAttack(const ObjectBase* object) const {
    return object != nullptr
        && ((object->isAStructure()
             || (object->isAGroundUnit() && !object->isInfantry()
                 && object->getItemID() != Unit_Sandworm)) /* allow attack tanks*/
            && (object->getOwner()->getTeamID() != owner_->getTeamID()) && object->isVisible(getOwner()->getTeamID()));
}

void Saboteur::destroy(const GameContext& context) {
    Coord realPos(lround(realX_), lround(realY_));
    const auto explosionID = context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2);
    context.game.addExplosion(explosionID, realPos, owner_->getHouseID());

    if (isVisible(getOwner()->getTeamID())) {
        dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionLarge, location_);
    }

    parent::destroy(context);
}
