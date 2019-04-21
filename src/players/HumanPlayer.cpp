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

#include <players/HumanPlayer.h>

#include <FileClasses/TextManager.h>
#include <FileClasses/GFXManager.h>
#include <GUI/dune/ChatManager.h>

#include <Game.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <structures/StructureBase.h>
#include <units/UnitBase.h>

#include <globals.h>
#include <sand.h>

#include <Network/NetworkManager.h>

#define ATTACKNOTIFICATIONTIME MILLI2CYCLES(2*60*1000)

HumanPlayer::HumanPlayer(House* associatedHouse, const std::string& playername) : Player(associatedHouse, playername) {
    HumanPlayer::init();
    alreadyShownTutorialHints = currentGame->getGameInitSettings().getAlreadyShownTutorialHints();
    lastAttackNotificationCycle = INVALID_GAMECYCLE;
}

HumanPlayer::HumanPlayer(InputStream& stream, House* associatedHouse) : Player(stream, associatedHouse) {
    HumanPlayer::init();

    for(int i=0;i < NUMSELECTEDLISTS; i++) {
        selectedLists[i] = stream.readUint32Set();
    }

    alreadyShownTutorialHints = stream.readUint32();
    lastAttackNotificationCycle = stream.readUint32();
}

void HumanPlayer::init() {
    nextExpectedCommandsCycle = 0;
}

HumanPlayer::~HumanPlayer() = default;

void HumanPlayer::save(OutputStream& stream) const {
    Player::save(stream);

    // write out selection groups (Key 1 to 9)
    for(int i=0; i < NUMSELECTEDLISTS; i++) {
        stream.writeUint32Set(selectedLists[i]);
    }

    stream.writeUint32(alreadyShownTutorialHints);
    stream.writeUint32(lastAttackNotificationCycle);
}

void HumanPlayer::update() {
}

void HumanPlayer::onDamage(const ObjectBase* pObject, int damage, Uint32 damagerID) {
    if((lastAttackNotificationCycle != INVALID_GAMECYCLE)
        && (getGameCycleCount() - lastAttackNotificationCycle < ATTACKNOTIFICATIONTIME)) {
        return;
    }

    if(pObject->isAStructure() && pObject->getOwner() == getHouse()) {
        soundPlayer->playVoice(BaseIsUnderAttack, getHouse()->getHouseID());
        lastAttackNotificationCycle = getGameCycleCount();
    }
}

void HumanPlayer::onProduceItem(Uint32 itemID) {
    if(!settings.general.showTutorialHints || (currentGame->gameState != GameState::Running)) {
        return;
    }

    if(!isStructure(itemID)) {
        return;
    }

    if(itemID == Structure_Slab1 || itemID == Structure_Slab4) {
        return;
    }

    if(alreadyShownTutorialHints & (1 << static_cast<int>(TutorialHint::NotEnoughConrete))) {
        return;
    }

    if(!hasConcreteOfSize(getStructureSize(itemID))) {
        ChatManager& chatManager = currentGame->getGameInterface().getChatManager();
        chatManager.addHintMessage( _("@MESSAGE.ENG|20#There is not enough concrete for this structure. You may continue building this structure but it will need repairing."),
                                    pGFXManager->getTinyPicture(static_cast<TinyPicture_Enum>(itemID)));
        alreadyShownTutorialHints |= (1 << static_cast<int>(TutorialHint::NotEnoughConrete));
    }
}

void HumanPlayer::onPlaceStructure(const StructureBase* pStructure) {
    if(!settings.general.showTutorialHints || (currentGame->gameState != GameState::Running)) {
        return;
    }

    int itemID = (pStructure == nullptr) ? Structure_Slab1 : pStructure->getItemID();

    triggerStructureTutorialHint(itemID);
}

void HumanPlayer::onUnitDeployed(const UnitBase* pUnit) {
    if(!settings.general.showTutorialHints || (currentGame->gameState != GameState::Running)) {
        return;
    }

    if(alreadyShownTutorialHints & (1 << static_cast<int>(TutorialHint::HarvestSpice))) {
        return;
    }

    if(pUnit->getItemID() == Unit_Harvester) {
        ChatManager& chatManager = currentGame->getGameInterface().getChatManager();
        chatManager.addHintMessage(_("@MESSAGE.ENG|27#Look out for spice fields."), pGFXManager->getTinyPicture(TinyPicture_Spice));
        alreadyShownTutorialHints |= (1 << static_cast<int>(TutorialHint::HarvestSpice));
    }


}

void HumanPlayer::onSelectionChanged(const std::set<Uint32>& selectedObjectIDs) {
    if(!settings.general.showTutorialHints || (currentGame->gameState != GameState::Running)) {
        return;
    }

    if(selectedObjectIDs.size() != 1) {
        return;
    }

    for(Uint32 objectID : selectedObjectIDs) {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        if(pObject->getOwner() != getHouse()) {
            continue;
        }

        triggerStructureTutorialHint(pObject->getItemID());
    }
}

void HumanPlayer::setGroupList(int groupListIndex, const std::set<Uint32>& newGroupList) {
    selectedLists[groupListIndex].clear();

    for(Uint32 objectID : newGroupList) {
        if(currentGame->getObjectManager().getObject(objectID) != nullptr) {
            selectedLists[groupListIndex].insert(objectID);
        }
    }

    if((pNetworkManager != nullptr) && (pLocalPlayer == this)) {
        // the local player has changed his group assignment
        pNetworkManager->sendSelectedList(selectedLists[groupListIndex], groupListIndex);
    }
}

void HumanPlayer::triggerStructureTutorialHint(Uint32 itemID) {
    if(alreadyShownTutorialHints & (1 << itemID)) {
        return;
    }

    ChatManager& chatManager = currentGame->getGameInterface().getChatManager();
    switch(itemID) {

        case Structure_Slab1: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|2#Concrete: Build your buildings on a stable foundation."), pGFXManager->getTinyPicture(TinyPicture_Slab1));
        } break;

        case Structure_Palace: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|3#Palace: This is your palace."), pGFXManager->getTinyPicture(TinyPicture_Palace));
        } break;

        case Structure_LightFactory: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|4#Light Factory: In the Light Factory small vehicles are produced."), pGFXManager->getTinyPicture(TinyPicture_LightFactory));
        } break;

        case Structure_HeavyFactory: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|5#Heavy Factory: In the Heavy Factory tracked vehicles are produced."), pGFXManager->getTinyPicture(TinyPicture_HeavyFactory));
        } break;

        case Structure_HighTechFactory: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|6#High-Tech Factory: In the High-Tech Factory flying units are produced."), pGFXManager->getTinyPicture(TinyPicture_HighTechFactory));
        } break;

        case Structure_IX: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|7#House IX: The science labs of House IX strengthens the technology of your house."), pGFXManager->getTinyPicture(TinyPicture_IX));
        } break;

        case Structure_WOR: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|8#WOR: In this facility heavily armed troppers are trained."), pGFXManager->getTinyPicture(TinyPicture_WOR));
        } break;

        case Structure_ConstructionYard: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|9#Construction Yard: All structures are produced by the Construction Yard."), pGFXManager->getTinyPicture(TinyPicture_ConstructionYard));
        } break;

        case Structure_WindTrap: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|10#Windtrap: Windtraps a supplying your base with energy. Without energy your buildings are decaying over time."), pGFXManager->getTinyPicture(TinyPicture_WindTrap));
        } break;

        case Structure_Barracks: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|11#Barracks: In this facility light armed infantry is trained."), pGFXManager->getTinyPicture(TinyPicture_Barracks));
        } break;

        case Structure_StarPort: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|12#Starport: Ordering and receiving shipments from C.H.O.A.M is the task of the Starport."), pGFXManager->getTinyPicture(TinyPicture_StarPort));
        } break;

        case Structure_Refinery: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|13#Refinery: In the Refinery Spice is converted into credits."), pGFXManager->getTinyPicture(TinyPicture_Refinery));
        } break;

        case Structure_RepairYard: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|14#Repair Yard: Damanged units can be repaired in the Repair Yard."), pGFXManager->getTinyPicture(TinyPicture_RepairYard));
        } break;

        case Structure_Wall: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|15#Wall: Walls help to defend your base."), pGFXManager->getTinyPicture(TinyPicture_Wall));
        } break;

        case Structure_GunTurret: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|16#Gun Turret: This turret type provides short range defense for your base."), pGFXManager->getTinyPicture(TinyPicture_GunTurret));
        } break;

        case Structure_RocketTurret: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|17#Rocket Turret: This turret type provides short and medium range defense for your base."), pGFXManager->getTinyPicture(TinyPicture_RocketTurret));
        } break;

        case Structure_Silo: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|18#Spice Silo: This building stores refined spice."), pGFXManager->getTinyPicture(TinyPicture_Silo));
        } break;

        case Structure_Radar: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|19#Outpost: The Outpost allows the usage of a radar and supports commanding distant vehicles."), pGFXManager->getTinyPicture(TinyPicture_Radar));
        } break;

        default: {
            return;
        } break;
    }

    alreadyShownTutorialHints |= (1 << itemID);
}

bool HumanPlayer::hasConcreteOfSize(const Coord& concreteSize) const {
    Coord pos;
    for(pos.y = 0; pos.y < currentGameMap->getSizeY() - concreteSize.y + 1; pos.y++) {
        for(pos.x = 0; pos.x < currentGameMap->getSizeX() - concreteSize.x + 1; pos.x++) {
            if(hasConcreteAtPositionOfSize(pos, concreteSize)) {
                return true;
            }
        }
    }
    return false;
}

bool HumanPlayer::hasConcreteAtPositionOfSize(const Coord& pos, const Coord& concreteSize) const {
    for(int y = pos.y; y < pos.y + concreteSize.y; y++) {
        for(int x = pos.x; x < pos.x + concreteSize.x; x++) {
            if(!currentGameMap->tileExists(x,y)) {
                return false;
            }

            Tile* pTile = currentGameMap->getTile(x, y);
            if((pTile->getType() != Terrain_Slab) || (pTile->getOwner() != getHouse()->getHouseID()) || pTile->isBlocked()) {
                return false;
            }
        }
    }
    return true;
}
