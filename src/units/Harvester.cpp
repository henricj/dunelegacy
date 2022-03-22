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

#include <FileClasses/GFXManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Explosion.h>
#include <SoundPlayer.h>
#include <ScreenBorder.h>

#include <players/HumanPlayer.h>
#include <players/QuantBot.h>

#include <structures/Refinery.h>

#include <misc/draw_util.h>

/* how often is the same sandframe redrawn */
#define HARVESTERDELAY 30

/* how often to change harvester position while harvesting */
#define RANDOMHARVESTMOVE 500

/* how much is the harvester movement slowed down when full  */
#define MAXIMUMHARVESTERSLOWDOWN 0.4_fix

/* number spice output frames - 1 */
#define LASTSANDFRAME 2

namespace {
constexpr TrackedUnitConstants harvester_constants{Harvester::item_id};
}

Harvester::Harvester(uint32_t objectID, const ObjectInitializer& initializer)
    : TrackedUnit(harvester_constants, objectID, initializer) {
    Harvester::init();

    setHealth(getMaxHealth());

    spice = 0;
    harvestingMode = false;
    returningToRefinery = false;
    spiceCheckCounter = 0;

    attackMode = GUARD;
}

Harvester::Harvester(uint32_t objectID, const ObjectStreamInitializer& initializer)
    : TrackedUnit(harvester_constants, objectID, initializer) {
    Harvester::init();

    auto& stream = initializer.stream();

    harvestingMode = stream.readBool();
    returningToRefinery = stream.readBool();
    spice = stream.readFixPoint();
    spiceCheckCounter = stream.readUint32();
}

void Harvester::init()
{
    assert(itemID == Unit_Harvester);
    owner->incrementUnits(itemID);

    graphicID = ObjPic_Harvester;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());

    numImagesX = static_cast<int>(ANGLETYPE::NUM_ANGLES);
    numImagesY = 1;
}

Harvester::~Harvester() = default;

void Harvester::save(OutputStream& stream) const
{
    parent::save(stream);
    stream.writeBool(harvestingMode);
    stream.writeBool(returningToRefinery);
    stream.writeFixPoint(spice);
    stream.writeUint32(spiceCheckCounter);
}

void Harvester::blitToScreen()
{
    const int x = screenborder->world2screenX(realX);
    const int y = screenborder->world2screenY(realY);

    const auto* pUnitGraphic = graphic[currentZoomlevel];
    const auto source = calcSpriteSourceRect(pUnitGraphic, static_cast<int>(drawnAngle), numImagesX);
    const auto dest = calcSpriteDrawingRectF( pUnitGraphic, x, y, numImagesX, 1, HAlign::Center, VAlign::Center);

    Dune_RenderCopyF(renderer, pUnitGraphic, &source, &dest);

    if(isHarvesting()) {

        static constexpr Coord harvesterSandOffset[] = {   Coord(-56, 4),
                                                Coord(-28, 20),
                                                Coord(0, 24),
                                                Coord(28, 20),
                                                Coord(56, 4),
                                                Coord(40, -24),
                                                Coord(0, -36),
                                                Coord(-36, -24)
                                            };

        const auto* const pSandGraphic = pGFXManager->getZoomedObjPic(ObjPic_Harvester_Sand, getOwner()->getHouseID(), currentZoomlevel);

        int frame = ((currentGame->getGameCycleCount() + (getObjectID() * 10)) / HARVESTERDELAY) % (2*LASTSANDFRAME);
        if(frame > LASTSANDFRAME) {
            frame -= LASTSANDFRAME;
        }

        const auto sandSource = calcSpriteSourceRect(pSandGraphic, static_cast<int>(drawnAngle), static_cast<int>(ANGLETYPE::NUM_ANGLES), frame, LASTSANDFRAME+1);
        const auto sandDest   = calcSpriteDrawingRectF(  pSandGraphic,
                                                         screenborder->world2screenX(realX + harvesterSandOffset[static_cast<int>(drawnAngle)].x),
                                                         screenborder->world2screenY(realY + harvesterSandOffset[static_cast<int>(drawnAngle)].y),
                                                         static_cast<int>(ANGLETYPE::NUM_ANGLES), LASTSANDFRAME+1,
                                                         HAlign::Center, VAlign::Center);

        Dune_RenderCopyF(renderer, pSandGraphic, &sandSource, &sandDest);
    }

    if(isBadlyDamaged()) {
        drawSmoke(x, y);
    }
}

void Harvester::checkPos(const GameContext& context) {
    parent::checkPos(context);

    if(attackMode == STOP) {
        harvestingMode = false;

        if(getOwner()->isAI()) { doSetAttackMode(context, HARVEST); } /*The AI doesn't like STOP*/
    }

    if(!active) return;

    auto& map = context.map;
    if(returningToRefinery) {
        if(const auto* const pRefinery = dune_cast<Refinery>(target.getObjPointer())) {
            auto* pObject = map.getGroundObject(context, location.x, location.y);

            if(justStoppedMoving && (pObject != nullptr) && (pObject->getObjectID() == target.getObjectID())) {
                if(pRefinery->isFree()) {
                    awaitingPickup = false;
                    setReturned(context);
                } else {
                    // the repair yard is already in use by some other unit => move out
                    const auto newDestination = map.findDeploySpot(this, target.getObjPointer()->getLocation(),
                                                                   getLocation(), pRefinery->getStructureSize());
                    doMove2Pos(context, newDestination, true);
                    requestCarryall(context);
                }
            } else if(!awaitingPickup && owner->hasCarryalls() && pRefinery->isFree() &&
                      blockDistance(location, pRefinery->getClosestPoint(location)) >= MIN_CARRYALL_LIFT_DISTANCE) {
                requestCarryall(context);
            }

            return;
        }

        int leastNumBookings = std::numeric_limits<int>::max(); // huge amount so refinery couldn't possibly compete
                                                                // with any refinery num bookings
        FixPoint  closestLeastBookedRefineryDistance = FixPt32_MAX;
        Refinery* pBestRefinery                      = nullptr;

        for(auto* pStructure : structureList) {
            if(pStructure->getOwner() != owner) continue;

            if(auto* const pRefinery = dune_cast<Refinery>(pStructure)) {
                Coord    closestPoint     = pRefinery->getClosestPoint(location);
                FixPoint refineryDistance = blockDistance(location, closestPoint);

                if(pRefinery->getNumBookings() < leastNumBookings) {
                    leastNumBookings                   = pRefinery->getNumBookings();
                    closestLeastBookedRefineryDistance = refineryDistance;
                    pBestRefinery                      = pRefinery;
                } else if(pRefinery->getNumBookings() == leastNumBookings) {
                    if(refineryDistance < closestLeastBookedRefineryDistance) {
                        closestLeastBookedRefineryDistance = refineryDistance;
                        pBestRefinery                      = pRefinery;
                    }
                }
            }
        }

        if(pBestRefinery) {
            doMove2Object(context, pBestRefinery);
            pBestRefinery->startAnimate();
        } else {
            setDestination(location);
        }

        return;
    }

    if(harvestingMode && !hasBookedCarrier() && destination.isValid() &&
              (blockDistance(location, destination) >= MIN_CARRYALL_LIFT_DISTANCE)) {
        requestCarryall(context);

        return;
    }

    if(respondable && !harvestingMode && attackMode != STOP) {
        if(spiceCheckCounter == 0) {
            // Find harvest location nearest to our base
            Coord newDestination;
            if(currentGameMap->findSpice(newDestination, guardPoint)) {
                setDestination(newDestination);
                setGuardPoint(newDestination);
                harvestingMode = true;
            } else {
                setDestination(location);
                setGuardPoint(location);
                harvestingMode = false;
            }
            spiceCheckCounter = 100;
        } else {
            spiceCheckCounter--;
        }

        return;
    }
}

void Harvester::deploy(const GameContext& context, const Coord& newLocation)
{
    if(currentGameMap->tileExists(newLocation)) {
        parent::deploy(context, newLocation);
        if(spice == 0) {
            Coord newDestination;
            if((attackMode != STOP) && context.map.findSpice(newDestination, guardPoint)) {
                harvestingMode = true;
                setDestination(newDestination);
                setGuardPoint(newDestination);

            } else {
                harvestingMode = false;
            }
        }
    }
}

void Harvester::destroy(const GameContext& context)
{
    if(currentGameMap->tileExists(location) && isVisible()) {
        const int xpos = location.x;
        const int ypos = location.y;

        if(currentGameMap->tileExists(xpos,ypos)) {
            const auto spiceSpreaded = spice * 0.75_fix;
            auto availableSandPos = 0;

            const auto circleRadius = lround(spice / 210);

            /* how many regions have sand */
            for(int i = -circleRadius; i <= circleRadius; i++) {
                for(int j = -circleRadius; j <= circleRadius; j++) {
                    if(currentGameMap->tileExists(xpos + i, ypos + j)
                        && (distanceFrom(xpos, ypos, xpos + i, ypos + j) + 0.0005_fix <= circleRadius))
                    {
                        auto *pTile = currentGameMap->getTile(xpos + i, ypos + j);
                        if((pTile != nullptr) & ((pTile->isSand()) || (pTile->isSpice()) )) {
                            availableSandPos++;
                        }
                    }
                }
            }

            /* now we can spread spice */
            for(int i = -circleRadius; i <= circleRadius; i++) {
                for(int j = -circleRadius; j <= circleRadius; j++) {
                    if(currentGameMap->tileExists(xpos + i, ypos + j)
                        && (distanceFrom(xpos, ypos, xpos + i, ypos + j) + 0.0005_fix  <= circleRadius))
                    {
                        Tile *pTile = currentGameMap->getTile(xpos + i, ypos + j);
                        if((pTile != nullptr) & ((pTile->isSand()) || (pTile->isSpice()) )) {
                            pTile->setSpice(pTile->getSpice() + spiceSpreaded / availableSandPos);
                        }
                    }
                }
            }
        }

        setTarget(nullptr);

        Coord    realPos(lround(realX), lround(realY));
        uint32_t explosionID = context.game.randomGen.getRandOf(Explosion_Medium1, Explosion_Medium2);
        context.game.addExplosion(explosionID, realPos, owner->getHouseID());

        if(isVisible(getOwner()->getTeamID())) {
            screenborder->shakeScreen(18);
            soundPlayer->playSoundAt(Sound_ExplosionLarge,location);
        }
    }

    parent::destroy(context);
}

void Harvester::drawSelectionBox()
{
    const DuneTexture* selectionBox = nullptr;

    switch(currentZoomlevel) {
        case 0:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel0);   break;
        case 1:     selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel1);   break;
        case 2:
        default:    selectionBox = pGFXManager->getUIGraphic(UI_SelectionBox_Zoomlevel2);   break;
    }

    auto dest = calcDrawingRectF(selectionBox, screenborder->world2screenX(realX), screenborder->world2screenY(realY), HAlign::Center, VAlign::Center);
    Dune_RenderCopyF(renderer, selectionBox, nullptr, &dest);

    dest.x += 1;
    dest.y -= static_cast<float>(currentZoomlevel + 1);
    dest.h = static_cast<float>(currentZoomlevel + 1);
    dest.w = static_cast<float>(lround((getHealth() / getMaxHealth()) * (getWidth(selectionBox) - 3)));

    renderFillRectF(renderer, &dest, getHealthColor());

    if((getOwner() == pLocalHouse) && (spice > 0)) {
        dest.y -= static_cast<float>(currentZoomlevel + 1);
        dest.w = static_cast<float>(lround(((spice) / HARVESTERMAXSPICE) * (getWidth(selectionBox) - 3)));
        renderFillRectF(renderer, &dest, COLOR_ORANGE);
    }
}

void Harvester::handleDamage(const GameContext& context, int damage, uint32_t damagerID, House* damagerOwner)
{
    parent::handleDamage(context, damage, damagerID, damagerOwner);

    auto* const damager = context.objectManager.getObject(damagerID);

    if(!target && !forced && damager && canAttack(damager) && (attackMode != STOP)) {
        setTarget(damager);
    }
}

void Harvester::handleReturnClick(const GameContext& context) {
    context.game.getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMDTYPE::CMD_HARVESTER_RETURN,objectID));
}

void Harvester::doReturn()
{
    if(!returningToRefinery && isActive()) {
        returningToRefinery = true;
        harvestingMode = false;

        if(getAttackMode() == STOP) {
            setGuardPoint(Coord::Invalid());
        }
    }
}

void Harvester::setAmountOfSpice(FixPoint newSpice)
{
    if((newSpice >= 0) && (newSpice <= HARVESTERMAXSPICE)) {
        spice = newSpice;
    }
}

void Harvester::setDestination(int newX, int newY)
{
    parent::setDestination(newX, newY);

    harvestingMode =  (attackMode != STOP) && (currentGameMap->tileExists(newX, newY) && currentGameMap->getTile(newX,newY)->hasSpice());
}

void Harvester::setTarget(const ObjectBase* newTarget)
{
    if(returningToRefinery && target && (target.getObjPointer()!= nullptr)
        && (target.getObjPointer()->getItemID() == Structure_Refinery))
    {
        static_cast<Refinery*>(target.getObjPointer())->unBook();
        returningToRefinery = false;
    }

    parent::setTarget(newTarget);

    if(target && (target.getObjPointer() != nullptr)
        && (target.getObjPointer()->getOwner() == getOwner())
        && (target.getObjPointer()->getItemID() == Structure_Refinery))
    {
        static_cast<Refinery*>(target.getObjPointer())->book();
        returningToRefinery = true;
    }

}

void Harvester::setReturned(const GameContext& context) {
    context.map.removeObjectFromMap(getObjectID());

    if(auto* refinery = dune_cast<Refinery>(target.getObjPointer())) refinery->assignHarvester(this);

    returningToRefinery = false;
    moving = false;
    respondable = false;
    setActive(false);

    setLocation(context, INVALID_POS, INVALID_POS);
    setVisible(VIS_ALL, false);
}


void Harvester::move(const GameContext& context)
{
    parent::move(context);

    if(active && !moving && !justStoppedMoving) {
        if(harvestingMode) {

            if(location == destination) {
                if(spice < HARVESTERMAXSPICE) {

                    auto* tile = context.map.tryGetTile(location.x, location.y);
                    if(!tile) return;

                    if(tile->hasSpice()) {

                        const int beforeTileType = tile->getType();
                        spice += tile->harvestSpice(context);
                        const int afterTileType = tile->getType();

                        if(beforeTileType != afterTileType) {
                            context.map.spiceRemoved(context, location);
                            if(!context.map.findSpice(destination, location)) {
                                doReturn();
                            } else {
                                doMove2Pos(context, destination, false);
                            }
                        }
                    } else if (!context.map.findSpice(destination, location)) {
                        if(spice > 0) {
                            doReturn();
                        }
                    } else {
                        doMove2Pos(context, destination, false);
                    }
                } else {
                    doReturn();
                }
            }
        }
    }
}

bool Harvester::isHarvesting() const {
    return  harvestingMode
            && (spice < HARVESTERMAXSPICE)
            && (blockDistance(location, destination) <= FixPt_SQRT2)
            && currentGameMap->tileExists(location) && currentGameMap->getTile(location)->hasSpice();
}

bool Harvester::canAttack(const ObjectBase* object) const
{
    return((object != nullptr)
            && object->isInfantry()
            && (object->getOwner()->getTeamID() != owner->getTeamID())
            && object->isVisible(getOwner()->getTeamID()));
}

FixPoint Harvester::extractSpice(FixPoint extractionSpeed)
{
    const FixPoint oldSpice = spice;

    if((spice - extractionSpeed) >= 0) {
        spice -= extractionSpeed;
    } else {
        spice = 0;
    }

    return (oldSpice - spice);
}

void Harvester::setSpeeds(const GameContext& context)
{
    FixPoint speed = getMaxSpeed(context);

    if(isBadlyDamaged()) {
        speed *= HEAVILYDAMAGEDSPEEDMULTIPLIER;
    }

    const FixPoint percentFull = spice/HARVESTERMAXSPICE;
    speed = speed * (1 - MAXIMUMHARVESTERSLOWDOWN*percentFull);

    // clang-format off
    switch(drawnAngle){
        case ANGLETYPE::LEFT:      xSpeed = -speed;                    ySpeed = 0;         break;
        case ANGLETYPE::LEFTUP:    xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = xSpeed;    break;
        case ANGLETYPE::UP:        xSpeed = 0;                         ySpeed = -speed;    break;
        case ANGLETYPE::RIGHTUP:   xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = -xSpeed;   break;
        case ANGLETYPE::RIGHT:     xSpeed = speed;                     ySpeed = 0;         break;
        case ANGLETYPE::RIGHTDOWN: xSpeed = speed*DIAGONALSPEEDCONST;  ySpeed = xSpeed;    break;
        case ANGLETYPE::DOWN:      xSpeed = 0;                         ySpeed = speed;     break;
        case ANGLETYPE::LEFTDOWN:  xSpeed = -speed*DIAGONALSPEEDCONST; ySpeed = -xSpeed;   break;
    }
    // clang-format off
}
