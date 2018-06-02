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

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <House.h>
#include <Game.h>
#include <Map.h>
#include <Bullet.h>
#include <SoundPlayer.h>

#include <players/HumanPlayer.h>

#include <units/InfantryBase.h>
#include <units/Trooper.h>
#include <units/Saboteur.h>

#include <GUI/ObjectInterfaces/PalaceInterface.h>

#define PALACE_DEATHHAND_WEAPONDAMAGE       100

Palace::Palace(House* newOwner) : StructureBase(newOwner) {
    Palace::init();

    setHealth(getMaxHealth());
    specialWeaponTimer = getMaxSpecialWeaponTimer();

    // TODO: Special weapon is available immediately but AI uses it only after first visual contact
    //specialTimer = 1; // we want the special weapon to be immediately ready
}

Palace::Palace(InputStream& stream) : StructureBase(stream) {
    Palace::init();

    specialWeaponTimer = stream.readSint32();


}

void Palace::init() {
    itemID = Structure_Palace;
    owner->incrementStructures(itemID);

    structureSize.x = 3;
    structureSize.y = 3;

    graphicID = ObjPic_Palace;
    graphic = pGFXManager->getObjPic(graphicID,getOwner()->getHouseID());
    numImagesX = 4;
    numImagesY = 1;
    firstAnimFrame = 2;
    lastAnimFrame = 3;

    canAttackStuff = true;
}

Palace::~Palace() = default;

void Palace::save(OutputStream& stream) const {
    StructureBase::save(stream);
    stream.writeSint32(specialWeaponTimer);
}

ObjectInterface* Palace::getInterfaceContainer() {
    if((pLocalHouse == owner) || (debug == true)) {
        return PalaceInterface::create(objectID);
    } else {
        return DefaultObjectInterface::create(objectID);
    }
}

void Palace::handleSpecialClick() {
    currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PALACE_SPECIALWEAPON,objectID));
}

void Palace::handleDeathhandClick(int xPos, int yPos) {
    if (currentGameMap->tileExists(xPos, yPos)) {
        currentGame->getCommandManager().addCommand(Command(pLocalPlayer->getPlayerID(), CMD_PALACE_DEATHHAND,objectID, (Uint32) xPos, (Uint32) yPos));
    }
}

void Palace::doSpecialWeapon() {
    if(!isSpecialWeaponReady()) {
        return;
    }

    switch (originalHouseID) {
        case HOUSE_HARKONNEN:
        case HOUSE_SARDAUKAR: {
            // wrong house (see DoLaunchDeathhand)
            return;
        } break;

        case HOUSE_ATREIDES:
        case HOUSE_FREMEN: {
            if(callFremen()) {
                specialWeaponTimer = getMaxSpecialWeaponTimer();
            }
        } break;

        case HOUSE_ORDOS:
        case HOUSE_MERCENARY: {
            if(spawnSaboteur()) {
                specialWeaponTimer = getMaxSpecialWeaponTimer();
            }
        } break;

        default: {
            THROW(std::runtime_error, "Palace::DoSpecialWeapon(): Invalid house");
        } break;
    }
}

void Palace::doLaunchDeathhand(int x, int y) {
    if(!isSpecialWeaponReady()) {
        return;
    }

    if((originalHouseID != HOUSE_HARKONNEN) && (originalHouseID != HOUSE_SARDAUKAR)) {
        // wrong house (see DoSpecialWeapon)
        return;
    }

    FixPoint randAngle = 2 * FixPt_PI * currentGame->randomGen.randFixPoint();
    int radius = currentGame->randomGen.rand(0,10*TILESIZE);
    int deathOffX = lround(FixPoint::sin(randAngle) * radius);
    int deathOffY = lround(FixPoint::cos(randAngle) * radius);

    Coord centerPoint = getCenterPoint();
    Coord dest( x * TILESIZE + TILESIZE/2 + deathOffX,
                y * TILESIZE + TILESIZE/2 + deathOffY);

    bulletList.push_back(new Bullet(objectID, &centerPoint, &dest, Bullet_LargeRocket, PALACE_DEATHHAND_WEAPONDAMAGE, false, nullptr));
    soundPlayer->playSoundAt(Sound_Rocket, getLocation());

    if(getOwner() != pLocalHouse) {
        currentGame->addToNewsTicker(_("@DUNE.ENG|81#Missile is approaching"));
        soundPlayer->playVoice(MissileApproaching, pLocalHouse->getHouseID());
    }

    specialWeaponTimer = getMaxSpecialWeaponTimer();

}

void Palace::updateStructureSpecificStuff() {
    if(specialWeaponTimer > 0) {
        --specialWeaponTimer;
        if(specialWeaponTimer <= 0) {
            specialWeaponTimer = 0;

            if(getOwner() == pLocalHouse) {
                currentGame->addToNewsTicker(_("Palace is ready"));
            } else if(getOwner()->isAI()) {

                if((originalHouseID == HOUSE_HARKONNEN) || (originalHouseID == HOUSE_SARDAUKAR)) {
                    // Harkonnen and Sardaukar

                    //old tergetting logic used by default AI
                    /*
                    const StructureBase* closestStructure = findClosestTargetStructure();
                    if(closestStructure) {
                        Coord temp = closestStructure->getClosestPoint(getLocation());
                        doLaunchDeathhand(temp.x, temp.y);
                    }*/
                } else {
                    // other houses
                    doSpecialWeapon();
                }
            }
        }
    }
}

bool Palace::callFremen() {
    int count = 0;
    int x;
    int y;
    do {
        x = currentGame->randomGen.rand(1, currentGameMap->getSizeX()-2);
        y = currentGame->randomGen.rand(1, currentGameMap->getSizeY()-2);
    } while((currentGameMap->getTile(x-1, y-1)->hasAGroundObject()
            || currentGameMap->getTile(x, y-1)->hasAGroundObject()
            || currentGameMap->getTile(x+1, y-1)->hasAGroundObject()
            || currentGameMap->getTile(x-1, y)->hasAGroundObject()
            || currentGameMap->getTile(x, y)->hasAGroundObject()
            || currentGameMap->getTile(x+1, y)->hasAGroundObject()
            || currentGameMap->getTile(x-1, y+1)->hasAGroundObject()
            || currentGameMap->getTile(x, y+1)->hasAGroundObject()
            || currentGameMap->getTile(x+1, y+1)->hasAGroundObject())
            && (count++ <= 1000));

    if(count < 1000) {

        for(int numFremen = 0; numFremen < 15; numFremen++) {
            if(currentGame->randomGen.rand(0, 5) == 0) {
                continue;
            }

            Trooper *pFremen = static_cast<Trooper*>(getOwner()->createUnit(Unit_Trooper));

            int i;
            int j;
            do {
                i = currentGame->randomGen.rand(-1, 1);
                j = currentGame->randomGen.rand(-1, 1);
            } while (!currentGameMap->getTile(x + i, y + j)->infantryNotFull());

            pFremen->deploy(Coord(x + i,y + j));

            pFremen->doSetAttackMode(HUNT);
            pFremen->setRespondable(false);

            const StructureBase* closestStructure = pFremen->findClosestTargetStructure();
            if(closestStructure) {
                Coord closestPoint = closestStructure->getClosestPoint(pFremen->getLocation());
                pFremen->setGuardPoint(closestPoint);
                pFremen->setDestination(closestPoint);
            } else {
                const UnitBase* closestUnit = pFremen->findClosestTargetUnit();
                if(closestUnit) {
                    pFremen->setGuardPoint(closestUnit->getLocation());
                    pFremen->setDestination(closestUnit->getLocation());
                }
            }
        }

        return true;
    } else {
        if(getOwner() == pLocalHouse) {
            currentGame->addToNewsTicker(_("Unable to spawn Fremen"));
        }

        return false;
    }
}

bool Palace::spawnSaboteur() {
    Saboteur* saboteur = static_cast<Saboteur*>(getOwner()->createUnit(Unit_Saboteur));
    Coord spot = currentGameMap->findDeploySpot(saboteur, getLocation(), currentGame->randomGen, getDestination(), getStructureSize());

    saboteur->deploy(spot);

    if(getOwner()->isAI()) {
        saboteur->doSetAttackMode(HUNT);
        currentGame->addToNewsTicker(_("@DUNE.ENG|79#Saboteur is approaching"));
        soundPlayer->playVoice(SaboteurApproaching, pLocalHouse->getHouseID());
    }

    return true;
}
