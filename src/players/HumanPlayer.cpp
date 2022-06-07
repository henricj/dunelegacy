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

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <GUI/dune/ChatManager.h>

#include <Game.h>
#include <Map.h>
#include <SoundPlayer.h>

#include <structures/StructureBase.h>
#include <units/UnitBase.h>

#include <globals.h>
#include <sand.h>

#include <Network/NetworkManager.h>

static constexpr auto ATTACKNOTIFICATIONTIME = MILLI2CYCLES(2 * 60 * 1000);

HumanPlayer::HumanPlayer(const GameContext& context, House* associatedHouse, const std::string& playername,
                         const Random& random)
    : Player(context, associatedHouse, playername, random),
      alreadyShownTutorialHints(context_.game.getGameInitSettings().getAlreadyShownTutorialHints()),
      lastAttackNotificationCycle(INVALID_GAMECYCLE) { }

HumanPlayer::HumanPlayer(const GameContext& context, InputStream& stream, House* associatedHouse)
    : Player(context, stream, associatedHouse) {
    for (auto& selectedList : selectedLists) {
        selectedList = stream.readUint32Set();
    }

    alreadyShownTutorialHints   = stream.readUint32();
    lastAttackNotificationCycle = stream.readUint32();
}

HumanPlayer::~HumanPlayer() = default;

void HumanPlayer::save(OutputStream& stream) const {
    Player::save(stream);

    // write out selection groups (Key 1 to 9)
    for (const auto& selectedList : selectedLists) {
        stream.writeUint32Set(selectedList);
    }

    stream.writeUint32(alreadyShownTutorialHints);
    stream.writeUint32(lastAttackNotificationCycle);
}

void HumanPlayer::update() { }

void HumanPlayer::onDamage(const ObjectBase* pObject, [[maybe_unused]] int damage,
                           [[maybe_unused]] uint32_t damagerID) {
    if ((lastAttackNotificationCycle != INVALID_GAMECYCLE)
        && (getGameCycleCount() - lastAttackNotificationCycle < ATTACKNOTIFICATIONTIME)) {
        return;
    }

    if (pObject->isAStructure() && pObject->getOwner() == getHouse()) {
        dune::globals::soundPlayer->playVoice(Voice_enum::BaseIsUnderAttack, getHouse()->getHouseID());
        lastAttackNotificationCycle = getGameCycleCount();
    }
}

void HumanPlayer::onProduceItem(ItemID_enum itemID) {
    if (!dune::globals::settings.general.showTutorialHints || (context_.game.gameState != GameState::Running)) {
        return;
    }

    if (!isStructure(itemID)) {
        return;
    }

    if (itemID == Structure_Slab1 || itemID == Structure_Slab4) {
        return;
    }

    if (alreadyShownTutorialHints & (1 << static_cast<int>(TutorialHint::NotEnoughConrete))) {
        return;
    }

    if (!hasConcreteOfSize(getStructureSize(itemID))) {
        ChatManager& chatManager = context_.game.getGameInterface().getChatManager();
        chatManager.addHintMessage(_("@MESSAGE.ENG|20#There is not enough concrete for this structure. You may "
                                     "continue building this structure but it will need repairing."),
                                   dune::globals::pGFXManager->getTinyPicture(static_cast<TinyPicture_Enum>(itemID)));
        alreadyShownTutorialHints |= (1 << static_cast<int>(TutorialHint::NotEnoughConrete));
    }
}

void HumanPlayer::onPlaceStructure(const StructureBase* pStructure) {
    if (!dune::globals::settings.general.showTutorialHints || (context_.game.gameState != GameState::Running)) {
        return;
    }

    const auto itemID = (pStructure == nullptr) ? Structure_Slab1 : pStructure->getItemID();

    triggerStructureTutorialHint(itemID);
}

void HumanPlayer::onUnitDeployed(const UnitBase* pUnit) {
    if (!dune::globals::settings.general.showTutorialHints || (context_.game.gameState != GameState::Running)) {
        return;
    }

    if (alreadyShownTutorialHints & (1 << static_cast<int>(TutorialHint::HarvestSpice))) {
        return;
    }

    if (pUnit->getItemID() == Unit_Harvester) {
        auto& chatManager = context_.game.getGameInterface().getChatManager();
        chatManager.addHintMessage(_("@MESSAGE.ENG|27#Look out for spice fields."),
                                   dune::globals::pGFXManager->getTinyPicture(TinyPicture_Spice));
        alreadyShownTutorialHints |= (1 << static_cast<int>(TutorialHint::HarvestSpice));
    }
}

void HumanPlayer::onSelectionChanged(const dune::selected_set_type& selectedObjectIDs) {
    if (!dune::globals::settings.general.showTutorialHints || (context_.game.gameState != GameState::Running)) {
        return;
    }

    if (selectedObjectIDs.size() != 1) {
        return;
    }

    for (const auto objectID : selectedObjectIDs) {
        const auto* pObject = context_.game.getObjectManager().getObject(objectID);
        if (pObject->getOwner() != getHouse()) {
            continue;
        }

        triggerStructureTutorialHint(pObject->getItemID());
    }
}

void HumanPlayer::setGroupList(int groupListIndex, const dune::selected_set_type& newGroupList) {
    selectedLists[groupListIndex].clear();

    for (auto objectID : newGroupList) {
        if (context_.game.getObjectManager().getObject(objectID) != nullptr) {
            selectedLists[groupListIndex].insert(objectID);
        }
    }

    auto* const network_manager = dune::globals::pNetworkManager.get();

    if ((network_manager != nullptr) && (dune::globals::pLocalPlayer == this)) {
        // the local player has changed his group assignment
        network_manager->sendSelectedList(selectedLists[groupListIndex], groupListIndex);
    }
}

void HumanPlayer::triggerStructureTutorialHint(ItemID_enum itemID) {
    // We can only store flags for the itemIDs below 32 in a 32-bit word.
    // Fortunately, the below switch() is only looking things that do
    // fit in alreadyShownTutorialHints.  We do a compare to avoid
    // undefined behavior (and the resulting UBSAN warning).
    if (itemID > 31 || alreadyShownTutorialHints & (1U << itemID)) {
        return;
    }

    const auto* const gfx = dune::globals::pGFXManager.get();

    ChatManager& chatManager = context_.game.getGameInterface().getChatManager();
    switch (itemID) {

        case Structure_Slab1: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|2#Concrete: Build your buildings on a stable foundation."),
                                       gfx->getTinyPicture(TinyPicture_Slab1));
        } break;

        case Structure_Palace: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|3#Palace: This is your palace."),
                                       gfx->getTinyPicture(TinyPicture_Palace));
        } break;

        case Structure_LightFactory: {
            chatManager.addHintMessage(
                _("@MESSAGE.ENG|4#Light Factory: In the Light Factory small vehicles are produced."),
                gfx->getTinyPicture(TinyPicture_LightFactory));
        } break;

        case Structure_HeavyFactory: {
            chatManager.addHintMessage(
                _("@MESSAGE.ENG|5#Heavy Factory: In the Heavy Factory tracked vehicles are produced."),
                gfx->getTinyPicture(TinyPicture_HeavyFactory));
        } break;

        case Structure_HighTechFactory: {
            chatManager.addHintMessage(
                _("@MESSAGE.ENG|6#High-Tech Factory: In the High-Tech Factory flying units are produced."),
                gfx->getTinyPicture(TinyPicture_HighTechFactory));
        } break;

        case Structure_IX: {
            chatManager.addHintMessage(
                _("@MESSAGE.ENG|7#House IX: The science labs of House IX strengthens the technology of your house."),
                gfx->getTinyPicture(TinyPicture_IX));
        } break;

        case Structure_WOR: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|8#WOR: In this facility heavily armed troppers are trained."),
                                       gfx->getTinyPicture(TinyPicture_WOR));
        } break;

        case Structure_ConstructionYard: {
            chatManager.addHintMessage(
                _("@MESSAGE.ENG|9#Construction Yard: All structures are produced by the Construction Yard."),
                gfx->getTinyPicture(TinyPicture_ConstructionYard));
        } break;

        case Structure_WindTrap: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|10#Windtrap: Windtraps a supplying your base with energy. "
                                         "Without energy your buildings are decaying over time."),
                                       gfx->getTinyPicture(TinyPicture_WindTrap));
        } break;

        case Structure_Barracks: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|11#Barracks: In this facility light armed infantry is trained."),
                                       gfx->getTinyPicture(TinyPicture_Barracks));
        } break;

        case Structure_StarPort: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|12#Starport: Ordering and receiving shipments from C.H.O.A.M is "
                                         "the task of the Starport."),
                                       gfx->getTinyPicture(TinyPicture_StarPort));
        } break;

        case Structure_Refinery: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|13#Refinery: In the Refinery Spice is converted into credits."),
                                       gfx->getTinyPicture(TinyPicture_Refinery));
        } break;

        case Structure_RepairYard: {
            chatManager.addHintMessage(
                _("@MESSAGE.ENG|14#Repair Yard: Damanged units can be repaired in the Repair Yard."),
                gfx->getTinyPicture(TinyPicture_RepairYard));
        } break;

        case Structure_Wall: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|15#Wall: Walls help to defend your base."),
                                       gfx->getTinyPicture(TinyPicture_Wall));
        } break;

        case Structure_GunTurret: {
            chatManager.addHintMessage(
                _("@MESSAGE.ENG|16#Gun Turret: This turret type provides short range defense for your base."),
                gfx->getTinyPicture(TinyPicture_GunTurret));
        } break;

        case Structure_RocketTurret: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|17#Rocket Turret: This turret type provides short and medium "
                                         "range defense for your base."),
                                       gfx->getTinyPicture(TinyPicture_RocketTurret));
        } break;

        case Structure_Silo: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|18#Spice Silo: This building stores refined spice."),
                                       gfx->getTinyPicture(TinyPicture_Silo));
        } break;

        case Structure_Radar: {
            chatManager.addHintMessage(_("@MESSAGE.ENG|19#Outpost: The Outpost allows the usage of a radar and "
                                         "supports commanding distant vehicles."),
                                       gfx->getTinyPicture(TinyPicture_Radar));
        } break;

        default: {
            return;
        }
    }

    alreadyShownTutorialHints |= (1U << itemID);
}

bool HumanPlayer::hasConcreteOfSize(const Coord& concreteSize) const {
    const auto& map = context_.map;

    Coord pos;
    for (pos.y = 0; pos.y < map.getSizeY() - concreteSize.y + 1; pos.y++) {
        for (pos.x = 0; pos.x < map.getSizeX() - concreteSize.x + 1; pos.x++) {
            if (hasConcreteAtPositionOfSize(pos, concreteSize)) {
                return true;
            }
        }
    }
    return false;
}

bool HumanPlayer::hasConcreteAtPositionOfSize(const Coord& pos, const Coord& concreteSize) const {
    const auto& map = context_.map;

    for (int y = pos.y; y < pos.y + concreteSize.y; y++) {
        for (int x = pos.x; x < pos.x + concreteSize.x; x++) {
            const auto* pTile = map.tryGetTile(x, y);

            if (nullptr == pTile)
                return false;

            if ((pTile->getType() != TERRAINTYPE::Terrain_Slab) || (pTile->getOwner() != getHouse()->getHouseID())
                || pTile->isBlocked()) {
                return false;
            }
        }
    }
    return true;
}
