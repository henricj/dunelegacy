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

#include <units/SandWorm.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <misc/draw_util.h>

#include <units/InfantryBase.h>

namespace {
inline constexpr auto MAX_SANDWORMSLEEPTIME = 50000;
inline constexpr auto MIN_SANDWORMSLEEPTIME = 10000;

inline constexpr auto SANDWORM_ATTACKFRAMETIME = 10;

class SandwormConstants : public GroundUnitConstants {
public:
    constexpr SandwormConstants() : GroundUnitConstants{Sandworm::item_id} { canAttackStuff_ = true; }
};

constexpr SandwormConstants sandworm_constants;
} // namespace

Sandworm::Sandworm(uint32_t objectID, const ObjectInitializer& initializer)
    : GroundUnit(sandworm_constants, objectID, initializer) {

    Sandworm::init();

    Sandworm::setHealth(getMaxHealth());

    respondable_ = false;

    for (auto& lastLoc : lastLocs_) {
        lastLoc.invalidate();
    }
}

Sandworm::Sandworm(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : GroundUnit(sandworm_constants, objectID, initializer) {

    Sandworm::init();

    auto& stream = initializer.stream();

    kills_                      = stream.readSint32();
    attackFrameTimer_           = stream.readSint32();
    sleepTimer_                 = stream.readSint32();
    warningWormSignPlayedFlags_ = stream.readUint8();
    shimmerOffsetIndex_         = stream.readSint32();
    for (auto& lastLoc : lastLocs_) {
        lastLoc.x = stream.readSint32();
        lastLoc.y = stream.readSint32();
    }
}

void Sandworm::init() {
    assert(itemID_ == Unit_Sandworm);
    owner_->incrementUnits(itemID_);

    graphicID_ = ObjPic_Sandworm;
    graphic_   = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());

    numImagesX_ = 1;
    numImagesY_ = 9;

    drawnFrame = INVALID;
}

Sandworm::~Sandworm() = default;

void Sandworm::save(OutputStream& stream) const {
    GroundUnit::save(stream);

    stream.writeSint32(kills_);
    stream.writeSint32(attackFrameTimer_);
    stream.writeSint32(sleepTimer_);
    stream.writeUint8(warningWormSignPlayedFlags_);
    stream.writeSint32(shimmerOffsetIndex_);
    for (const auto lastLoc : lastLocs_) {
        stream.writeSint32(lastLoc.x);
        stream.writeSint32(lastLoc.y);
    }
}

void Sandworm::assignToMap(const GameContext& context, const Coord& pos) {
    if (auto* tile = context.map.tryGetTile(pos.x, pos.y)) {
        tile->assignUndergroundUnit(getObjectID());
        // do not unhide map cause this would give Fremen players an advantage
        // currentGameMap->viewMap(owner->getHouseID(), location, getViewRange());
    }
}

bool Sandworm::attack([[maybe_unused]] const GameContext& context) {
    if (primaryWeaponTimer == 0) {
        if (target_) {
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_WormAttack, location_);
            drawnFrame         = 0;
            attackFrameTimer_  = SANDWORM_ATTACKFRAMETIME;
            primaryWeaponTimer = getWeaponReloadTime();
            return true;
        }
    }
    return false;
}

void Sandworm::deploy(const GameContext& context, const Coord& newLocation) {
    parent::deploy(context, newLocation);

    respondable_ = false;
}

void Sandworm::blitToScreen() {
    static constexpr auto shimmerOffset = std::to_array<int>({1, 3, 2, 5, 4, 3, 2, 1});

    auto* const renderer           = dune::globals::renderer.get();
    const auto* const screenborder = dune::globals::screenborder.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    if (shimmerOffsetIndex_ >= 0) {
        // render sandworm's shimmer

        auto* const gfx = dune::globals::pGFXManager.get();

        const auto* shimmerMaskTex = gfx->getZoomedObjPic(ObjPic_SandwormSegment, zoom);

        CoordF previous{};

        for (int i = 0; i < SANDWORM_SEGMENTS; i++) {
            const auto& loc = lastLocs_[i];
            if (loc.isInvalid()) {
                continue;
            }

            const auto offset = static_cast<float>(shimmerOffset[(shimmerOffsetIndex_ + i) % shimmerOffset.size()] * 2);

            const auto dest = calcDrawingRect(shimmerMaskTex, screenborder->world2screenX(loc.x),
                                              screenborder->world2screenY(loc.y), HAlign::Center, VAlign::Center);

            SDL_SetTextureAlphaMod(shimmerMaskTex->texture_, 4);
            SDL_SetTextureBlendMode(shimmerMaskTex->texture_, SDL_BLENDMODE_ADD);

            const CoordF current{dest.x + offset, dest.y};

            if (i > 0) {
                static constexpr auto steps = 8;

                for (auto j = 1; j < steps; ++j) {
                    const auto p =
                        previous * static_cast<float>(j) / steps + current * static_cast<float>(steps - j) / steps;

                    shimmerMaskTex->draw(renderer, p.x, p.y, i * 11.0 + j * 3.0);
                }
            }

            shimmerMaskTex->draw(renderer, dest.x + offset, dest.y, i * 11.0);

            previous = current;

            SDL_SetTextureAlphaMod(shimmerMaskTex->texture_, 255);
            SDL_SetTextureBlendMode(shimmerMaskTex->texture_, SDL_BLENDMODE_BLEND);
        }
    }

    if (drawnFrame != INVALID) {
        const auto* graphic = graphic_[zoom];

        const auto dest =
            calcSpriteDrawingRect(graphic, screenborder->world2screenX(realX_), screenborder->world2screenY(realY_),
                                  numImagesX_, numImagesY_, HAlign::Center, VAlign::Center);
        const auto source = calcSpriteSourceRect(graphic, 0, numImagesX_, drawnFrame, numImagesY_);
        Dune_RenderCopyF(renderer, graphic, &source, &dest);
    }
}

void Sandworm::checkPos(const GameContext& context) {
    if (justStoppedMoving) {
        realX_ = location_.x * TILESIZE + TILESIZE / 2;
        realY_ = location_.y * TILESIZE + TILESIZE / 2;

        const auto* const infantry = context.map.tryGetInfantry(context, location_.x, location_.y);

        if (infantry && infantry->getOwner() == dune::globals::pLocalHouse) {
            dune::globals::soundPlayer->playVoice(Voice_enum::SomethingUnderTheSand,
                                                  infantry->getOwner()->getHouseID());
        }
    }
}

void Sandworm::engageTarget(const GameContext& context) {
    if (isEating()) {
        return;
    }

    parent::engageTarget(context);

    if (target_) {
        FixPoint maxDistance;

        if (forced_) {
            maxDistance = FixPt_MAX;
        } else {
            switch (attackMode_) {
                case GUARD:
                case AMBUSH: {
                    maxDistance = getViewRange();
                } break;

                case AREAGUARD:
                case HUNT: {
                    maxDistance = FixPt_MAX;
                } break;

                case STOP:
                case CAPTURE:
                default: {
                    maxDistance = 0;
                } break;
            }
        }

        if (targetDistance > maxDistance) {
            // give up
            setDestination(guardPoint);
            setTarget(nullptr);
        }
    }
}

void Sandworm::setLocation(const GameContext& context, int xPos, int yPos) {
    if (context.map.tileExists(xPos, yPos) || ((xPos == INVALID_POS) && (yPos == INVALID_POS))) {
        parent::setLocation(context, xPos, yPos);
    }
}

/**
    Put sandworm to sleep for a while
*/
void Sandworm::sleep(const GameContext& context) {
    sleepTimer_ = context.game.randomGen.rand(MIN_SANDWORMSLEEPTIME, MAX_SANDWORMSLEEPTIME);
    setActive(false);
    setVisible(VIS_ALL, false);
    setForced(false);
    context.map.removeObjectFromMap(getObjectID()); // no map point will reference now
    setLocation(context, INVALID_POS, INVALID_POS);
    setHealth(getMaxHealth());
    kills_                      = 0;
    warningWormSignPlayedFlags_ = 0;
    drawnFrame                  = INVALID;
    attackFrameTimer_           = 0;
    shimmerOffsetIndex_         = -1;
    for (auto& lastLoc : lastLocs_) {
        lastLoc.invalidate();
    }
}

bool Sandworm::sleepOrDie(const GameContext& context) {

    // Make sand worms always drop spice, even if they don't die
    if (context.game.getGameInitSettings().getGameOptions().killedSandwormsDropSpice) {
        context.map.createSpiceField(context, location_, 4);
    }

    if (context.game.getGameInitSettings().getGameOptions().sandwormsRespawn) {
        sleep(context);
        return true;
    }
    destroy(context);

    return false;
}

void Sandworm::setTarget(const ObjectBase* newTarget) {
    parent::setTarget(newTarget);

    const auto* const house = dune::globals::pLocalHouse;

    if (newTarget == nullptr || newTarget->getOwner() != house
        || (warningWormSignPlayedFlags_ & 1 << static_cast<int>(house->getHouseID())) != 0)
        return;

    dune::globals::soundPlayer->playVoice(Voice_enum::WarningWormSign, house->getHouseID());
    warningWormSignPlayedFlags_ |= (1 << static_cast<int>(house->getHouseID()));
}

void Sandworm::handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner) {
    if (damage > 0)
        attackMode_ = HUNT;

    parent::handleDamage(context, damage, damagerID, damagerOwner);
}

bool Sandworm::update(const GameContext& context) {
    auto& [game, map, objectManager] = context;

    if (getHealth() <= getMaxHealth() / 2) {
        if (!sleepOrDie(context)) {
            return false;
        }
    } else {
        if (!parent::update(context)) {
            return false;
        }

        if (isActive() && (moving || justStoppedMoving) && !game.isGamePaused() && !game.isGameFinished()) {
            const Coord realLocation = getLocation() * TILESIZE + Coord(TILESIZE / 2, TILESIZE / 2);
            if (lastLocs_[1] != realLocation) {
                for (int i = (SANDWORM_SEGMENTS - 1); i > 0; i--) {
                    lastLocs_[i] = lastLocs_[i - 1];
                }
                lastLocs_[1] = realLocation;
            }
            lastLocs_[0].x      = lround(realX_);
            lastLocs_[0].y      = lround(realY_);
            shimmerOffsetIndex_ = ((game.getGameCycleCount() + getObjectID()) % 48) / 6;
        }

        if (attackFrameTimer_ > 0) {
            attackFrameTimer_--;

            // death frame has started
            if (attackFrameTimer_ == 0) {
                drawnFrame++;
                if (drawnFrame >= 9) {
                    drawnFrame = INVALID;
                    if (kills_ >= 3) {
                        if (!sleepOrDie(context)) {
                            return false;
                        }
                    }
                } else {
                    attackFrameTimer_ = SANDWORM_ATTACKFRAMETIME;
                    if (drawnFrame == 1) {
                        // the close mouth bit of graphic is currently shown => eat unit
                        if (target_) {
                            const auto* object = target_.getObjPointer();

                            if (object) {
                                const bool wasAlive =
                                    object->isVisible(getOwner()->getTeamID()); // see if unit was alive before attack
                                const Coord realPos = Coord(lround(realX_), lround(realY_));
                                map.damage(context, objectID_, getOwner(), realPos, Bullet_Sandworm, 5000, NONE_ID,
                                           false);
                                // TODO: map.damage() might have invalidated "object"?  Do we need an object->isAlive()
                                // method?
                                if (wasAlive && target_
                                    && (!target_.getObjPointer()->isVisible(getOwner()->getTeamID()))) {
                                    kills_++;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (sleepTimer_ > 0) {
            sleepTimer_--;

            if (sleepTimer_ == 0) {
                // awaken the worm!

                for (int tries = 0; tries < 1000; tries++) {
                    const int x = game.randomGen.rand(0, context.map.getSizeX() - 1);
                    const int y = game.randomGen.rand(0, context.map.getSizeY() - 1);

                    if (canPass(x, y)) {
                        deploy(context, map.getTile(x, y)->getLocation());
                        break;
                    }
                }

                if (!isActive()) {
                    // no room for sandworm on map => take another nap
                    if (!sleepOrDie(context)) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool Sandworm::canAttack(const ObjectBase* object) const {
    if (!object || !object->isAGroundUnit() || object->getItemID() == Unit_Sandworm)
        return false;

    auto* map = dune::globals::currentGameMap;

    if (!map->tileExists(object->getLocation()))
        return false;

    const auto* const pTile = map->getTile(object->getLocation());

    if (!canPassTile(pTile))
        return false;

    return pTile->getSandRegion() == map->getTile(location_)->getSandRegion();
}

bool Sandworm::canPassTile(const Tile* pTile) const {
    return !pTile->isRock()
        && (!pTile->hasAnUndergroundUnit()
            || (pTile->getUndergroundUnit(dune::globals::currentGame->getObjectManager()) == this));
}

const ObjectBase* Sandworm::findTarget() const {
    if (isEating())
        return nullptr;

    if ((attackMode_ == HUNT) || (attackMode_ == AREAGUARD)) {
        const ObjectBase* closestTarget = nullptr;
        auto closestDistance            = FixPt_MAX;

        for (const auto* pUnit : dune::globals::unitList) {
            if (canAttack(pUnit) && (blockDistance(location_, pUnit->getLocation()) < closestDistance)) {
                closestTarget   = pUnit;
                closestDistance = blockDistance(location_, pUnit->getLocation());
            }
        }

        return closestTarget;
    }

    return ObjectBase::findTarget();
}

ANGLETYPE Sandworm::getCurrentAttackAngle() const {
    // we can always attack an target
    return targetAngle;
}

void Sandworm::playAttackSound() {
    dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_WormAttack, location_);
}

FixPoint Sandworm::getTerrainDifficulty(TERRAINTYPE terrainType) const {
    switch (terrainType) {
        case TERRAINTYPE::Terrain_Slab: return 1.0_fix;
        case TERRAINTYPE::Terrain_Sand: return 1.25_fix;
        case TERRAINTYPE::Terrain_Rock: return 1.0_fix;
        case TERRAINTYPE::Terrain_Dunes: return 1.25_fix;
        case TERRAINTYPE::Terrain_Mountain: return 1.0_fix;
        case TERRAINTYPE::Terrain_Spice: return 1.25_fix;
        case TERRAINTYPE::Terrain_ThickSpice: return 1.25_fix;
        case TERRAINTYPE::Terrain_SpiceBloom: return 1.25_fix;
        case TERRAINTYPE::Terrain_SpecialBloom: return 1.25_fix;
        default: return 1.0_fix;
    }
}
