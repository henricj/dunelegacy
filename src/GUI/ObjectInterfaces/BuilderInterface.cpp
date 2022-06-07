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

#include "GUI/ObjectInterfaces/BuilderInterface.h"

#include "GUI/Spacer.h"

#include <FileClasses/TextManager.h>

#include <structures/BuilderBase.h>
#include <structures/StarPort.h>

#include <type_traits>

BuilderInterface::BuilderInterface(const GameContext& context, int objectID)
    : DefaultStructureInterface(context, objectID) {
    const auto house_id = static_cast<int>(dune::globals::pLocalHouse->getHouseID());
    const auto color    = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[house_id] + 3]);

    upgradeButton.setText(_("Upgrade"));
    upgradeButton.setTextColor(color);
    upgradeButton.setVisible(false);
    upgradeButton.setTooltipText(_("Upgrade this structure (Hotkey: U)"));
    upgradeButton.setOnClick([this] { onUpgrade(); });

    upgradeProgressBar.setText(_("Upgrade"));
    upgradeProgressBar.setTextColor(color);
    upgradeProgressBar.setVisible(false);
    topBox.addWidget(&upgradeButton, {18, 2}, {83, 18});
    topBox.addWidget(&upgradeProgressBar, {18, 2}, {83, 18});

    mainHBox.addWidget(Widget::create<Spacer>().release());

    if (auto* const pBuilder = context_.objectManager.getObject<BuilderBase>(objectID)) {
        pBuilderList = BuilderList::create(pBuilder->getObjectID());
        mainHBox.addWidget(pBuilderList);

        static_assert(std::is_convertible_v<StarPort*, BuilderBase*>);

        if (dune_cast<StarPort>(pBuilder)) {
            starportTimerLabel.setTextFontSize(28);
            starportTimerLabel.setTextColor(COLOR_WHITE, COLOR_TRANSPARENT);
            starportTimerLabel.setAlignment(static_cast<Alignment_Enum>(Alignment_HCenter | Alignment_VCenter));
            const auto position =
                topBox.getWidgetPosition(&topBoxHBox) + topBoxHBox.getWidgetPosition(&objPicture) + Point(0, 4);
            topBox.addWidget(&starportTimerLabel, position, objPicture.getSize());
        }
    }

    mainHBox.addWidget(Widget::create<Spacer>().release());
}

BuilderInterface::~BuilderInterface() = default;

void BuilderInterface::onUpgrade() const {
    auto* const pBuilder = context_.objectManager.getObject<BuilderBase>(objectID);

    if (!pBuilder)
        return;

    if (!pBuilder->isUpgrading()) {
        pBuilder->handleUpgradeClick();
    }
}

bool BuilderInterface::update() {
    auto* const pBuilder = context_.objectManager.getObject<BuilderBase>(objectID);
    if (pBuilder == nullptr)
        return false;

    if (const auto* const pStarport = dune_cast<StarPort>(pBuilder)) {
        const auto arrivalTimer = pStarport->getArrivalTimer();
        if (arrivalTimer > 0) {
            const auto seconds = ((arrivalTimer * 10) / (MILLI2CYCLES(30 * 1000))) + 1;
            starportTimerLabel.setText(std::to_string(seconds));
        } else {
            starportTimerLabel.setText(std::string{});
        }
    }

    upgradeProgressBar.setVisible(pBuilder->isUpgrading());
    upgradeButton.setVisible(!pBuilder->isUpgrading());
    upgradeButton.setEnabled(!pBuilder->isUpgrading());

    if (pBuilder->isUpgrading()) {
        upgradeProgressBar.setProgress(
            ((pBuilder->getUpgradeProgress() * 100) / pBuilder->getUpgradeCost(context_)).toFloat());
    }

    if (pBuilder->getHealth() >= pBuilder->getMaxHealth()) {
        repairButton.setVisible(false);
        if (pBuilder->isAllowedToUpgrade()) {
            upgradeButton.setVisible(true);
        } else {
            upgradeButton.setVisible(false);
        }
    } else {
        repairButton.setVisible(true);
        upgradeButton.setVisible(false);
        repairButton.setToggleState(pBuilder->isRepairing());
    }

    return true;
}
