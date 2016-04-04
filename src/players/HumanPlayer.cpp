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

#include <Game.h>

#include <globals.h>

#include <Network/NetworkManager.h>

HumanPlayer::HumanPlayer(House* associatedHouse, std::string playername) : Player(associatedHouse, playername) {
    HumanPlayer::init();
}

HumanPlayer::HumanPlayer(InputStream& stream, House* associatedHouse) : Player(stream, associatedHouse) {
    HumanPlayer::init();

    for(int i=0;i < NUMSELECTEDLISTS; i++) {
        selectedLists[i] = stream.readUint32Set();
    }
}

void HumanPlayer::init() {
    nextExpectedCommandsCycle = 0;
}

HumanPlayer::~HumanPlayer() {
}

void HumanPlayer::save(OutputStream& stream) const {
    Player::save(stream);

    // write out selection groups (Key 1 to 9)
    for(int i=0; i < NUMSELECTEDLISTS; i++) {
        stream.writeUint32Set(selectedLists[i]);
    }
}

void HumanPlayer::update() {
}

void HumanPlayer::setGroupList(int groupListIndex, const std::set<Uint32>& newGroupList) {
    selectedLists[groupListIndex].clear();

    std::set<Uint32>::const_iterator iter;
    for(iter = newGroupList.begin(); iter != newGroupList.end(); ++iter) {
        if(currentGame->getObjectManager().getObject(*iter) != nullptr) {
            selectedLists[groupListIndex].insert(*iter);
        }
    }

    if((pNetworkManager != nullptr) && (pLocalPlayer == this)) {
        // the local player has changed his group assignment
        pNetworkManager->sendSelectedList(selectedLists[groupListIndex], groupListIndex);
    }
}
