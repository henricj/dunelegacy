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

#include "GUI/Spacer.h"
#include <FileClasses/TextManager.h>
#include <Network/NetworkManager.h>
#include <players/HumanPlayer.h>

#include <Game.h>

WaitingForOtherPlayers::WaitingForOtherPlayers() : Window(50, 50, 50, 50) {
    WaitingForOtherPlayers::setWindowWidget(&vbox);
    vbox.addWidget(create<VSpacer>(6).release());

    vbox.addWidget(&textLabel);
    vbox.addWidget(create<VSpacer>(3).release());
    vbox.addWidget(&hbox);
    vbox.addWidget(create<VSpacer>(6).release());
    hbox.addWidget(create<Spacer>().release());
    hbox.addWidget(&vbox2);
    vbox2.addWidget(create<VSpacer>(4).release());
    removeButton.setText(_("Remove"));
    removeButton.setOnClick([] { onRemove(); });
    removeButton.setVisible(false);
    removeButton.setEnabled(false);
    vbox2.addWidget(&removeButton);
    vbox2.addWidget(create<VSpacer>(4).release());
    hbox.addWidget(create<Spacer>().release());
    textLabel.setAlignment(Alignment_HCenter);

    update();

    const auto size = getSize();

    const int xpos = std::max(0, (getRendererWidth() - size.x) / 2);
    const int ypos = std::max(0, (getRendererHeight() - size.y) / 2);

    WaitingForOtherPlayers::setCurrentPosition(xpos, ypos);
}

WaitingForOtherPlayers::~WaitingForOtherPlayers() = default;

void WaitingForOtherPlayers::update() {
    auto text = std::string{_("Waiting for other players... ")};

    // test if we need to wait for data to arrive
    for (const auto& playername : dune::globals::pNetworkManager->getConnectedPeers()) {
        const auto* const game = dune::globals::currentGame.get();

        if (const auto* pPlayer = dynamic_cast<HumanPlayer*>(game->getPlayerByName(playername))) {
            if (pPlayer->nextExpectedCommandsCycle <= game->getGameCycleCount()) {
                text += "\n" + pPlayer->getPlayername();
            }
        }
    }

    setText(std::move(text));
}

void WaitingForOtherPlayers::onRemove() {
    dune::globals::currentGame->resumeGame();
}
