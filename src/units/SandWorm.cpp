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

    respondable = false;

    for (auto& lastLoc : lastLocs) {
        lastLoc.invalidate();
    }
}

Sandworm::Sandworm(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : GroundUnit(sandworm_constants, objectID, initializer) {

    Sandworm::init();

    auto& stream = initializer.stream();

    kills                      = stream.readSint32();
    attackFrameTimer           = stream.readSint32();
    sleepTimer                 = stream.readSint32();
    warningWormSignPlayedFlags = stream.readUint8();
    shimmerOffsetIndex         = stream.readSint32();
    for (auto& lastLoc : lastLocs) {
        lastLoc.x = stream.readSint32();
        lastLoc.y = stream.readSint32();
    }
}

void Sandworm::init() {
    assert(itemID == Unit_Sandworm);
    owner->incrementUnits(itemID);

    graphicID = ObjPic_Sandworm;
    graphic   = dune::globals::pGFXManager->getObjPic(graphicID, getOwner()->getHouseID());

    numImagesX = 1;
    numImagesY = 9;

    drawnFrame = INVALID;
}

Sandworm::~Sandworm() = default;

void Sandworm::save(OutputStream& stream) const {
    GroundUnit::save(stream);

    stream.writeSint32(kills);
    stream.writeSint32(attackFrameTimer);
    stream.writeSint32(sleepTimer);
    stream.writeUint8(warningWormSignPlayedFlags);
    stream.writeSint32(shimmerOffsetIndex);
    for (const auto lastLoc : lastLocs) {
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

bool Sandworm::attack(const GameContext& context) {
    if (primaryWeaponTimer == 0) {
        if (target) {
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_WormAttack, location);
            drawnFrame         = 0;
            attackFrameTimer   = SANDWORM_ATTACKFRAMETIME;
            primaryWeaponTimer = getWeaponReloadTime();
            return true;
        }
    }
    return false;
}

void Sandworm::deploy(const GameContext& context, const Coord& newLocation) {
    parent::deploy(context, newLocation);

    respondable = false;
}

void Sandworm::blitToScreen() {
    static constexpr int shimmerOffset[] = {1, 3, 2, 5, 4, 3, 2, 1};

    using dune::globals::currentZoomlevel;
    auto* const renderer           = dune::globals::renderer.get();
    const auto* const screenborder = dune::globals::screenborder.get();

    if (shimmerOffsetIndex >= 0) {
        // render sandworm's shimmer

        auto* const gfx = dune::globals::pGFXManager.get();

        const auto* shimmerMaskTex = gfx->getZoomedObjPic(ObjPic_SandwormShimmerMask, currentZoomlevel);
        auto* shimmerTex = gfx->getTempStreamingTexture(renderer, shimmerMaskTex->source_.w, shimmerMaskTex->source_.h);

        for (int i = 0; i < SANDWORM_SEGMENTS; i++) {
            if (lastLocs[i].isInvalid()) {
                continue;
            }

            auto dest = calcDrawingRect(shimmerMaskTex, screenborder->world2screenX(lastLocs[i].x),
                                        screenborder->world2screenY(lastLocs[i].y), HAlign::Center, VAlign::Center);

            // switch to texture 'shimmerTex' for rendering
            SDL_Texture* oldRenderTarget = SDL_GetRenderTarget(renderer);
            SDL_SetRenderTarget(renderer, shimmerTex);

            // copy complete mask
            // contains solid black (0,0,0,255) for pixels to take from screen
            // and transparent (0,0,0,0) for pixels that should not be copied over
            SDL_SetTextureBlendMode(shimmerMaskTex->texture_, SDL_BLENDMODE_NONE);
            Dune_RenderCopy(renderer, shimmerMaskTex, nullptr, nullptr);
            SDL_SetTextureBlendMode(shimmerMaskTex->texture_, SDL_BLENDMODE_BLEND);

            auto* const screen_texture = dune::globals::screenTexture.get();

            // now copy r,g,b colors from screen but don't change alpha values in mask
            SDL_SetTextureBlendMode(screen_texture, SDL_BLENDMODE_ADD);
            auto source = as_rect(dest);
            source.x += shimmerOffset[(shimmerOffsetIndex + i) % 8] * 2;
            Dune_RenderCopyF(renderer, screen_texture, &source, nullptr);
            SDL_SetTextureBlendMode(screen_texture, SDL_BLENDMODE_NONE);

            // switch back to old rendering target (from texture 'shimmerTex')
            SDL_SetRenderTarget(renderer, oldRenderTarget);

            // now blend shimmerTex to screen (= make use of alpha values in mask)
            SDL_SetTextureBlendMode(shimmerTex, SDL_BLENDMODE_BLEND);
            Dune_RenderCopyF(renderer, shimmerTex, nullptr, &dest);
        }
    }

    if (drawnFrame != INVALID) {
        const auto dest   = calcSpriteDrawingRect(graphic[currentZoomlevel], screenborder->world2screenX(realX),
                                                  screenborder->world2screenY(realY), numImagesX, numImagesY,
                                                  HAlign::Center, VAlign::Center);
        const auto source = calcSpriteSourceRect(graphic[currentZoomlevel], 0, numImagesX, drawnFrame, numImagesY);
        Dune_RenderCopyF(renderer, graphic[currentZoomlevel], &source, &dest);
    }
}

void Sandworm::checkPos(const GameContext& context) {
    if (justStoppedMoving) {
        realX = location.x * TILESIZE + TILESIZE / 2;
        realY = location.y * TILESIZE + TILESIZE / 2;

        const auto* const infantry = context.map.tryGetInfantry(context, location.x, location.y);

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

    if (target) {
        FixPoint maxDistance;

        if (forced) {
            maxDistance = FixPt_MAX;
        } else {
            switch (attackMode) {
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
    sleepTimer = context.game.randomGen.rand(MIN_SANDWORMSLEEPTIME, MAX_SANDWORMSLEEPTIME);
    setActive(false);
    setVisible(VIS_ALL, false);
    setForced(false);
    context.map.removeObjectFromMap(getObjectID()); // no map point will reference now
    setLocation(context, INVALID_POS, INVALID_POS);
    setHealth(getMaxHealth());
    kills                      = 0;
    warningWormSignPlayedFlags = 0;
    drawnFrame                 = INVALID;
    attackFrameTimer           = 0;
    shimmerOffsetIndex         = -1;
    for (auto& lastLoc : lastLocs) {
        lastLoc.invalidate();
    }
}

bool Sandworm::sleepOrDie(const GameContext& context) {

    // Make sand worms always drop spice, even if they don't die
    if (context.game.getGameInitSettings().getGameOptions().killedSandwormsDropSpice) {
        context.map.createSpiceField(context, location, 4);
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
        || (warningWormSignPlayedFlags & 1 << static_cast<int>(house->getHouseID())) != 0)
        return;

    dune::globals::soundPlayer->playVoice(Voice_enum::WarningWormSign, house->getHouseID());
    warningWormSignPlayedFlags |= (1 << static_cast<int>(house->getHouseID()));
}

void Sandworm::handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner) {
    if (damage > 0)
        attackMode = HUNT;

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
            if (lastLocs[1] != realLocation) {
                for (int i = (SANDWORM_SEGMENTS - 1); i > 0; i--) {
                    lastLocs[i] = lastLocs[i - 1];
                }
                lastLocs[1] = realLocation;
            }
            lastLocs[0].x      = lround(realX);
            lastLocs[0].y      = lround(realY);
            shimmerOffsetIndex = ((game.getGameCycleCount() + getObjectID()) % 48) / 6;
        }

        if (attackFrameTimer > 0) {
            attackFrameTimer--;

            // death frame has started
            if (attackFrameTimer == 0) {
                drawnFrame++;
                if (drawnFrame >= 9) {
                    drawnFrame = INVALID;
                    if (kills >= 3) {
                        if (!sleepOrDie(context)) {
                            return false;
                        }
                    }
                } else {
                    attackFrameTimer = SANDWORM_ATTACKFRAMETIME;
                    if (drawnFrame == 1) {
                        // the close mouth bit of graphic is currently shown => eat unit
                        if (target) {
                            const auto* object = target.getObjPointer();

                            if (object) {
                                const bool wasAlive =
                                    object->isVisible(getOwner()->getTeamID()); // see if unit was alive before attack
                                const Coord realPos = Coord(lround(realX), lround(realY));
                                map.damage(context, objectID, getOwner(), realPos, Bullet_Sandworm, 5000, NONE_ID,
                                           false);
                                // TODO: map.damage() might have invalidated "object"?  Do we need an object->isAlive()
                                // method?
                                if (wasAlive && target
                                    && (!target.getObjPointer()->isVisible(getOwner()->getTeamID()))) {
                                    kills++;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (sleepTimer > 0) {
            sleepTimer--;

            if (sleepTimer == 0) {
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

    return pTile->getSandRegion() == map->getTile(location)->getSandRegion();
}

bool Sandworm::canPassTile(const Tile* pTile) const {
    return !pTile->isRock()
        && (!pTile->hasAnUndergroundUnit()
            || (pTile->getUndergroundUnit(dune::globals::currentGame->getObjectManager()) == this));
}

const ObjectBase* Sandworm::findTarget() const {
    if (isEating())
        return nullptr;

    if ((attackMode == HUNT) || (attackMode == AREAGUARD)) {
        const ObjectBase* closestTarget = nullptr;
        auto closestDistance            = FixPt_MAX;

        for (const auto* pUnit : dune::globals::unitList) {
            if (canAttack(pUnit) && (blockDistance(location, pUnit->getLocation()) < closestDistance)) {
                closestTarget   = pUnit;
                closestDistance = blockDistance(location, pUnit->getLocation());
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
    dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_WormAttack, location);
}
