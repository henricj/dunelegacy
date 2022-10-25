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

#include <units/Harvester.h>

#include <globals.h>

#include <Explosion.h>
#include <FileClasses/GFXManager.h>
#include <Game.h>
#include <House.h>
#include <Map.h>
#include <ScreenBorder.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>
#include <players/QuantBot.h>

#include <structures/Refinery.h>

#include <misc/draw_util.h>

namespace {
/* how often is the same sandframe redrawn */
inline constexpr auto HARVESTERDELAY = 30;

/* how often to change harvester position while harvesting */
// inline constexpr auto RANDOMHARVESTMOVE = 500;

/* how much is the harvester movement slowed down when full  */
inline constexpr auto MAXIMUMHARVESTERSLOWDOWN = 0.4_fix;

/* number spice output frames - 1 */
inline constexpr auto LASTSANDFRAME = 2;

constexpr TrackedUnitConstants harvester_constants{Harvester::item_id};
} // namespace

Harvester::Harvester(uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(harvester_constants, objectID, initializer) {
    Harvester::init();

    Harvester::setHealth(getMaxHealth());

    attackMode_ = GUARD;
}

Harvester::Harvester(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(harvester_constants, objectID, initializer) {
    Harvester::init();

    auto& stream = initializer.stream();

    harvestingMode      = stream.readBool();
    returningToRefinery = stream.readBool();
    spice               = stream.readFixPoint();
    spiceCheckCounter   = stream.readUint32();
}

void Harvester::init() {
    assert(itemID_ == Unit_Harvester);
    owner_->incrementUnits(itemID_);

    graphicID_ = ObjPic_Harvester;
    graphic_   = dune::globals::pGFXManager->getObjPic(graphicID_, getOwner()->getHouseID());

    numImagesX_ = NUM_ANGLES;
    numImagesY_ = 1;
}

Harvester::~Harvester() = default;

void Harvester::save(OutputStream& stream) const {
    parent::save(stream);
    stream.writeBool(harvestingMode);
    stream.writeBool(returningToRefinery);
    stream.writeFixPoint(spice);
    stream.writeUint32(spiceCheckCounter);
}

void Harvester::blitToScreen() {
    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto zoom                = dune::globals::currentZoomlevel;

    const int x = screenborder->world2screenX(realX_);
    const int y = screenborder->world2screenY(realY_);

    const auto* pUnitGraphic = graphic_[zoom];
    const auto source        = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle_), numImagesX_);
    const auto dest = calcSpriteDrawingRectF(pUnitGraphic, x, y, numImagesX_, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pUnitGraphic, &source, &dest);

    if (isHarvesting()) {

        static constexpr auto harvesterSandOffset =
            std::to_array<Coord>({{-56, 4}, {-28, 20}, {0, 24}, {28, 20}, {56, 4}, {40, -24}, {0, -36}, {-36, -24}});

        const auto* const pSandGraphic =
            dune::globals::pGFXManager->getZoomedObjPic(ObjPic_Harvester_Sand, getOwner()->getHouseID(), zoom);

        int frame = (dune::globals::currentGame->getGameCycleCount() + getObjectID() * 10) / HARVESTERDELAY
                  % (2 * LASTSANDFRAME);
        if (frame > LASTSANDFRAME) {
            frame -= LASTSANDFRAME;
        }

        const auto sandSource =
            calcSpriteSourceRect(pSandGraphic, static_cast<int>(drawnAngle_), NUM_ANGLES, frame, LASTSANDFRAME + 1);
        const auto sandDest = calcSpriteDrawingRectF(
            pSandGraphic, screenborder->world2screenX(realX_ + harvesterSandOffset[static_cast<int>(drawnAngle_)].x),
            screenborder->world2screenY(realY_ + harvesterSandOffset[static_cast<int>(drawnAngle_)].y), NUM_ANGLES,
            LASTSANDFRAME + 1, HAlign::Center, VAlign::Center);

        Dune_RenderCopyF(renderer, pSandGraphic, &sandSource, &sandDest);
    }

    if (isBadlyDamaged()) {
        drawSmoke(x, y);
    }
}

void Harvester::checkPos(const GameContext& context) {
    parent::checkPos(context);

    if (attackMode_ == STOP) {
        harvestingMode = false;

        if (getOwner()->isAI()) {
            doSetAttackMode(context, HARVEST);
        } /*The AI doesn't like STOP*/
    }

    if (!active_)
        return;

    auto& map = context.map;
    if (returningToRefinery) {
        if (const auto* const pRefinery = dune_cast<Refinery>(target_.getObjPointer())) {
            const auto* pObject = map.getGroundObject(context, location_.x, location_.y);

            if (justStoppedMoving && pObject != nullptr && pObject->getObjectID() == target_.getObjectID()) {
                if (pRefinery->isFree()) {
                    awaitingPickup = false;
                    setReturned(context);
                } else {
                    // the repair yard is already in use by some other unit => move out
                    const auto newDestination = map.findDeploySpot(this, target_.getObjPointer()->getLocation(),
                                                                   getLocation(), pRefinery->getStructureSize());
                    doMove2Pos(context, newDestination, true);
                    requestCarryall(context);
                }
            } else if (!awaitingPickup && owner_->hasCarryalls() && pRefinery->isFree()
                       && blockDistance(location_, pRefinery->getClosestPoint(location_))
                              >= MIN_CARRYALL_LIFT_DISTANCE) {
                requestCarryall(context);
            }

            return;
        }

        int leastNumBookings = std::numeric_limits<int>::max(); // huge amount so refinery couldn't possibly compete
                                                                // with any refinery num bookings
        FixPoint closestLeastBookedRefineryDistance = FixPt32_MAX;
        Refinery* pBestRefinery                     = nullptr;

        for (auto* pStructure : dune::globals::structureList) {
            if (pStructure->getOwner() != owner_)
                continue;

            if (auto* const pRefinery = dune_cast<Refinery>(pStructure)) {
                Coord closestPoint        = pRefinery->getClosestPoint(location_);
                FixPoint refineryDistance = blockDistance(location_, closestPoint);

                if (pRefinery->getNumBookings() < leastNumBookings) {
                    leastNumBookings                   = pRefinery->getNumBookings();
                    closestLeastBookedRefineryDistance = refineryDistance;
                    pBestRefinery                      = pRefinery;
                } else if (pRefinery->getNumBookings() == leastNumBookings) {
                    if (refineryDistance < closestLeastBookedRefineryDistance) {
                        closestLeastBookedRefineryDistance = refineryDistance;
                        pBestRefinery                      = pRefinery;
                    }
                }
            }
        }

        if (pBestRefinery) {
            doMove2Object(context, pBestRefinery);
            pBestRefinery->startAnimate();
        } else {
            setDestination(location_);
        }

        return;
    }

    if (harvestingMode && !hasBookedCarrier() && destination_.isValid()
        && blockDistance(location_, destination_) >= MIN_CARRYALL_LIFT_DISTANCE) {
        requestCarryall(context);

        return;
    }

    if (respondable_ && !harvestingMode && attackMode_ != STOP) {
        if (spiceCheckCounter == 0) {
            // Find harvest location nearest to our base
            Coord newDestination;
            if (context.map.findSpice(newDestination, guardPoint)) {
                setDestination(newDestination);
                setGuardPoint(newDestination);
                harvestingMode = true;
            } else {
                setDestination(location_);
                setGuardPoint(location_);
                harvestingMode = false;
            }
            spiceCheckCounter = 100;
        } else {
            spiceCheckCounter--;
        }
    }
}

void Harvester::deploy(const GameContext& context, const Coord& newLocation) {
    if (!context.map.tileExists(newLocation))
        return;

    parent::deploy(context, newLocation);

    if (spice != 0)
        return;

    Coord newDestination;
    if (attackMode_ != STOP && context.map.findSpice(newDestination, guardPoint)) {
        harvestingMode = true;
        setDestination(newDestination);
        setGuardPoint(newDestination);
    } else
        harvestingMode = false;
}

void Harvester::destroy(const GameContext& context) {
    if (isVisible()) {
        auto& game = context.game;
        auto& map  = context.map;

        const int xpos = location_.x;
        const int ypos = location_.y;

        if (map.tileExists(xpos, ypos)) {
            const auto spiceSpread = spice * 0.75_fix;
            auto availableSandPos  = 0;

            const auto circleRadius = lround(spice / 210);

            /* how many regions have sand */
            map.for_each(xpos - circleRadius, ypos - circleRadius, xpos + circleRadius, ypos + circleRadius,
                         [xpos, ypos, circleRadius, &availableSandPos](auto& tile) {
                             if (distanceFrom({xpos, ypos}, tile.location_) + 0.0005_fix > circleRadius)
                                 return;

                             if (tile.isSand() || tile.isSpice())
                                 availableSandPos++;
                         });

            /* now we can spread spice */
            map.for_each(xpos - circleRadius, ypos - circleRadius, xpos + circleRadius, ypos + circleRadius,
                         [xpos, ypos, circleRadius, availableSandPos, spiceSpread](auto& tile) {
                             if (distanceFrom({xpos, ypos}, tile.location_) + 0.0005_fix > circleRadius)
                                 return;

                             if (tile.isSand() || tile.isSpice())
                                 tile.setSpice(tile.getSpice() + spiceSpread / availableSandPos);
                         });
        }

        Coord realPos(lround(realX_), lround(realY_));
        uint32_t explosionID = game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2);
        game.addExplosion(explosionID, realPos, owner_->getHouseID());

        if (isVisible(getOwner()->getTeamID())) {
            dune::globals::screenborder->shakeScreen(18);
            dune::globals::soundPlayer->playSoundAt(Sound_enum::Sound_ExplosionLarge, location_);
        }
    }

    parent::destroy(context);
}

void Harvester::drawSelectionBox() {
    const DuneTexture* selectionBox = nullptr;

    const auto* const screenborder = dune::globals::screenborder.get();
    auto* const renderer           = dune::globals::renderer.get();
    const auto* const gfx          = dune::globals::pGFXManager.get();

    switch (dune::globals::currentZoomlevel) {
        case 0: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel0); break;
        case 1: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel1); break;
        case 2:
        default: selectionBox = gfx->getUIGraphic(UI_SelectionBox_Zoomlevel2); break;
    }

    auto dest = calcDrawingRectF(selectionBox, screenborder->world2screenX(realX_), screenborder->world2screenY(realY_),
                                 HAlign::Center, VAlign::Center);
    Dune_RenderCopyF(renderer, selectionBox, nullptr, &dest);

    const auto zoom = dune::globals::currentZoomlevel;
    dest.x += 1;
    dest.y -= static_cast<float>(zoom + 1);
    dest.h = static_cast<float>(zoom + 1);
    dest.w = (getHealth() / getMaxHealth()).toFloat() * (getWidth(selectionBox) - 3);

    renderFillRectF(renderer, &dest, getHealthColor());

    if (getOwner() == dune::globals::pLocalHouse && spice > 0) {
        dest.y -= static_cast<float>(zoom + 1);
        dest.w = (spice / HARVESTERMAXSPICE).toFloat() * (getWidth(selectionBox) - 3);
        renderFillRectF(renderer, &dest, COLOR_ORANGE);
    }
}

void Harvester::handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner) {
    parent::handleDamage(context, damage, damagerID, damagerOwner);

    const auto* const damager = context.objectManager.getObject(damagerID);

    if (!target_ && !forced_ && damager && canAttack(damager) && attackMode_ != STOP) {
        setTarget(damager);
    }
}

void Harvester::handleReturnClick(const GameContext& context) {
    context.game.getCommandManager().addCommand(
        Command(dune::globals::pLocalPlayer->getPlayerID(), CMDTYPE::CMD_HARVESTER_RETURN, objectID_));
}

void Harvester::doReturn() {
    if (!returningToRefinery && isActive()) {
        returningToRefinery = true;
        harvestingMode      = false;

        if (getAttackMode() == STOP) {
            setGuardPoint(Coord::Invalid());
        }
    }
}

void Harvester::setAmountOfSpice(FixPoint newSpice) {
    if (newSpice >= 0 && newSpice <= HARVESTERMAXSPICE) {
        spice = newSpice;
    }
}

void Harvester::setDestination(int newX, int newY) {
    parent::setDestination(newX, newY);

    auto* const map = dune::globals::currentGameMap;

    harvestingMode = attackMode_ != STOP && (map->tileExists(newX, newY) && map->getTile(newX, newY)->hasSpice());
}

void Harvester::setTarget(const ObjectBase* newTarget) {
    if (returningToRefinery) {
        if (auto* const refinery = dune_cast<Refinery>(target_.getObjPointer())) {
            refinery->unBook();
            returningToRefinery = false;
        }
    }

    parent::setTarget(newTarget);

    if (auto* const refinery = dune_cast<Refinery>(target_.getObjPointer())) {
        refinery->book();
        returningToRefinery = true;
    }
}

void Harvester::setReturned(const GameContext& context) {
    context.map.removeObjectFromMap(getObjectID());

    if (auto* refinery = dune_cast<Refinery>(target_.getObjPointer()))
        refinery->assignHarvester(this);

    returningToRefinery = false;
    moving              = false;
    respondable_        = false;
    setActive(false);

    setLocation(context, INVALID_POS, INVALID_POS);
    setVisible(VIS_ALL, false);
}

void Harvester::move(const GameContext& context) {
    parent::move(context);

    if (active_ && !moving && !justStoppedMoving) {
        if (harvestingMode) {

            if (location_ == destination_) {
                if (spice < HARVESTERMAXSPICE) {

                    auto* tile = context.map.tryGetTile(location_.x, location_.y);
                    if (!tile)
                        return;

                    if (tile->hasSpice()) {

                        const auto beforeTileType = tile->getType();
                        spice += tile->harvestSpice(context);
                        const auto afterTileType = tile->getType();

                        if (beforeTileType != afterTileType) {
                            context.map.spiceRemoved(context, location_);
                            if (!context.map.findSpice(destination_, location_)) {
                                doReturn();
                            } else {
                                doMove2Pos(context, destination_, false);
                            }
                        }
                    } else if (!context.map.findSpice(destination_, location_)) {
                        if (spice > 0) {
                            doReturn();
                        }
                    } else {
                        doMove2Pos(context, destination_, false);
                    }
                } else {
                    doReturn();
                }
            }
        }
    }
}

bool Harvester::isHarvesting() const {
    return harvestingMode && spice < HARVESTERMAXSPICE && blockDistance(location_, destination_) <= FixPt_SQRT2
        && dune::globals::currentGameMap->tileExists(location_)
        && dune::globals::currentGameMap->getTile(location_)->hasSpice();
}

bool Harvester::canAttack(const ObjectBase* object) const {
    return object != nullptr && object->isInfantry() && object->getOwner()->getTeamID() != owner_->getTeamID()
        && object->isVisible(getOwner()->getTeamID());
}

FixPoint Harvester::extractSpice(FixPoint extractionSpeed) {
    const FixPoint oldSpice = spice;

    if (spice - extractionSpeed >= 0) {
        spice -= extractionSpeed;
    } else {
        spice = 0;
    }

    return oldSpice - spice;
}

void Harvester::setSpeeds(const GameContext& context) {
    FixPoint speed = getMaxSpeed(context);

    if (isBadlyDamaged()) {
        speed *= HEAVILYDAMAGEDSPEEDMULTIPLIER;
    }

    const FixPoint percentFull = spice / HARVESTERMAXSPICE;
    speed                      = speed * (1 - MAXIMUMHARVESTERSLOWDOWN * percentFull);

    // clang-format off
    switch(drawnAngle_){
        case ANGLETYPE::LEFT:      xSpeed = -speed;                    ySpeed = 0;         break;
        case ANGLETYPE::LEFTUP:    xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = xSpeed;    break;
        case ANGLETYPE::UP:        xSpeed = 0;                         ySpeed = -speed;    break;
        case ANGLETYPE::RIGHTUP:   xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = -xSpeed;   break;
        case ANGLETYPE::RIGHT:     xSpeed = speed;                     ySpeed = 0;         break;
        case ANGLETYPE::RIGHTDOWN: xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = xSpeed;    break;
        case ANGLETYPE::DOWN:      xSpeed = 0;                         ySpeed = speed;     break;
        case ANGLETYPE::LEFTDOWN:  xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = -xSpeed;   break;
        default : break;
    }
    // clang-format off
}
