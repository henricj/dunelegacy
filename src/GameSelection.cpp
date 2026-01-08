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

#include <Game.h>

#include <globals.h>

#include <GameInterface.h>
#include <ScreenBorder.h>
#include <players/HumanPlayer.h>
#include <structures/StructureBase.h>

void Game::selectAll(const dune::selected_set_type& aList) const {
    for (const auto objectID : aList) {
        if (auto* object = objectManager_.getObject(objectID))
            object->setSelected(true);
    }
}

void Game::unselectAll(const dune::selected_set_type& aList) const {
    for (const auto objectID : aList) {
        if (auto* object = objectManager_.getObject(objectID))
            object->setSelected(false);
    }
}

void Game::onReceiveSelectionList(const std::string& name, const dune::selected_set_type& newSelectionList,
                                  int groupListIndex) {
    auto* pHumanPlayer = dynamic_cast<HumanPlayer*>(getPlayerByName(name));

    if (pHumanPlayer == nullptr) {
        return;
    }

    if (groupListIndex == -1) {
        // the other player controlling the same house has selected some units

        if (pHumanPlayer->getHouse() != dune::globals::pLocalHouse) {
            return;
        }

        for (const auto objectID : selectedByOtherPlayerList_) {
            auto* pObject = objectManager_.getObject(objectID);
            if (pObject != nullptr) {
                pObject->setSelectedByOtherPlayer(false);
            }
        }

        selectedByOtherPlayerList_ = newSelectionList;

        for (const uint32_t objectID : selectedByOtherPlayerList_) {
            ObjectBase* pObject = objectManager_.getObject(objectID);
            if (pObject != nullptr) {
                pObject->setSelectedByOtherPlayer(true);
            }
        }
    } else {
        // some other player has assigned a number to a list of units
        pHumanPlayer->setGroupList(groupListIndex, newSelectionList);
    }
}

void Game::onPeerDisconnected(const std::string& name, [[maybe_unused]] bool bHost, [[maybe_unused]] int cause) const {
    pInterface_->getChatManager().addInfoMessage(name + " disconnected!");
}

bool Game::removeFromSelectionLists(ObjectBase* pObject) {
    if (!pObject->isSelected() && !pObject->isSelectedByOtherPlayer())
        return false;

    const auto objectID = pObject->getObjectID();

    assert(getSelectedList().contains(objectID) || getSelectedByOtherPlayerList().contains(objectID));

    getSelectedList().erase(objectID);
    getSelectedByOtherPlayerList().erase(objectID);

    pObject->setSelected(false);

    return true;
}

void Game::removeFromQuickSelectionLists(uint32_t objectID) {
    auto* const local_player = dune::globals::pLocalPlayer;

    for (int i = 0; i < NUMSELECTEDLISTS; i++) {
        local_player->getGroupList(i).erase(objectID);
    }
}

void Game::clearSelectedList() {
    unselectAll(selectedList_);
    selectedList_.clear();
    selectionChanged();
}

void Game::selectNextStructureOfType(const dune::selected_set_type& itemIDs) {
    bool bSelectNext = true;

    if (selectedList_.size() == 1) {
        if (const auto* const pObject = getObjectManager().getObject(*selectedList_.begin()))
            if (itemIDs.count(pObject->getItemID()) == 1) {
                bSelectNext = false;
            }
    }

    StructureBase* pStructure2Select = nullptr;

    for (auto* const pStructure : dune::globals::structureList) {
        if (bSelectNext) {
            if ((itemIDs.count(pStructure->getItemID()) == 1)
                && (pStructure->getOwner() == dune::globals::pLocalHouse)) {
                pStructure2Select = pStructure;
                break;
            }
        } else {
            if (selectedList_.size() == 1 && pStructure->isSelected()) {
                bSelectNext = true;
            }
        }
    }

    if (pStructure2Select == nullptr) {
        // start over at the beginning
        for (auto* pStructure : dune::globals::structureList) {
            if ((itemIDs.count(pStructure->getItemID()) == 1) && (pStructure->getOwner() == dune::globals::pLocalHouse)
                && !pStructure->isSelected()) {
                pStructure2Select = pStructure;
                break;
            }
        }
    }

    if (pStructure2Select != nullptr) {
        unselectAll(selectedList_);
        selectedList_.clear();

        pStructure2Select->setSelected(true);
        selectedList_.insert(pStructure2Select->getObjectID());
        selectionChanged();

        // we center around the newly selected construction yard
        dune::globals::screenborder->setNewScreenCenter(pStructure2Select->getLocation() * TILESIZE);
    }
}
