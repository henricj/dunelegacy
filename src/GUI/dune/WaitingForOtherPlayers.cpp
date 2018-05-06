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

#include <GUI/dune/WaitingForOtherPlayers.h>

#include <globals.h>

#include <FileClasses/TextManager.h>
#include <players/HumanPlayer.h>
#include <Network/NetworkManager.h>

#include <Game.h>


WaitingForOtherPlayers::WaitingForOtherPlayers() : Window(50,50,50,50)
{
    setWindowWidget(&vbox);
    vbox.addWidget(VSpacer::create(6));

    vbox.addWidget(&textLabel);
    vbox.addWidget(VSpacer::create(3));
    vbox.addWidget(&hbox);
    vbox.addWidget(VSpacer::create(6));
    hbox.addWidget(Spacer::create());
    hbox.addWidget(&vbox2);
    vbox2.addWidget(VSpacer::create(4));
    removeButton.setText(_("Remove"));
    removeButton.setOnClick(std::bind(&WaitingForOtherPlayers::onRemove, this));
    removeButton.setVisible(false);
    removeButton.setEnabled(false);
    vbox2.addWidget(&removeButton);
    vbox2.addWidget(VSpacer::create(4));
    hbox.addWidget(Spacer::create());
    textLabel.setAlignment(Alignment_HCenter);

    update();

    int xpos = std::max(0,(getRendererWidth() - getSize().x)/2);
    int ypos = std::max(0,(getRendererHeight() - getSize().y)/2);

    setCurrentPosition(xpos,ypos,getSize().x,getSize().y);
}

WaitingForOtherPlayers::~WaitingForOtherPlayers()
{
    ;
}

void WaitingForOtherPlayers::update() {
    std::string text = _("Waiting for other players... ");

    // test if we need to wait for data to arrive
    for(const std::string& playername : pNetworkManager->getConnectedPeers()) {
        HumanPlayer* pPlayer = dynamic_cast<HumanPlayer*>(currentGame->getPlayerByName(playername));
        if(pPlayer != nullptr) {
            if(pPlayer->nextExpectedCommandsCycle <= currentGame->getGameCycleCount()) {
                text += "\n" + pPlayer->getPlayername();
            }
        }
    }

    setText(text);
}

void WaitingForOtherPlayers::onRemove()
{
    currentGame->resumeGame();
}
