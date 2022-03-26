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

#ifndef BUILDERINTERFACE_H
#define BUILDERINTERFACE_H

#include "DefaultStructureInterface.h"

#include <FileClasses/TextManager.h>

#include <GUI/Label.h>
#include <GUI/ProgressBar.h>
#include <GUI/TextButton.h>
#include <GUI/dune/BuilderList.h>

#include <structures/BuilderBase.h>
#include <structures/StarPort.h>

class BuilderInterface final : public DefaultStructureInterface {
public:
    static std::unique_ptr<BuilderInterface> create(const GameContext& context, int objectID) {
        auto tmp        = std::unique_ptr<BuilderInterface> {new BuilderInterface {context, objectID}};
        tmp->pAllocated = true;
        return tmp;
    }

protected:
    explicit BuilderInterface(const GameContext& context, int objectID) : DefaultStructureInterface(context, objectID) {
        const auto color = SDL2RGB(palette[houseToPaletteIndex[static_cast<int>(pLocalHouse->getHouseID())] + 3]);

        upgradeButton.setText(_("Upgrade"));
        upgradeButton.setTextColor(color);
        upgradeButton.setVisible(false);
        upgradeButton.setTooltipText(_("Upgrade this structure (Hotkey: U)"));
        upgradeButton.setOnClick([this] { onUpgrade(); });

        upgradeProgressBar.setText(_("Upgrade"));
        upgradeProgressBar.setTextColor(color);
        upgradeProgressBar.setVisible(false);
        topBox.addWidget(&upgradeButton, Point(18, 2), Point(83, 18));
        topBox.addWidget(&upgradeProgressBar, Point(18, 2), Point(83, 18));

        mainHBox.addWidget(Spacer::create());

        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        auto* pBuilder      = dynamic_cast<BuilderBase*>(pObject);
        if (pBuilder) {
            pBuilderList = BuilderList::create(pBuilder->getObjectID());
            mainHBox.addWidget(pBuilderList);
        } else {
            pBuilderList = nullptr;
        }

        mainHBox.addWidget(Spacer::create());

        auto* pStarport = dynamic_cast<StarPort*>(pObject);
        if (pStarport != nullptr) {
            starportTimerLabel.setTextFontSize(28);
            starportTimerLabel.setTextColor(COLOR_WHITE, COLOR_TRANSPARENT);
            starportTimerLabel.setAlignment(static_cast<Alignment_Enum>(Alignment_HCenter | Alignment_VCenter));
            topBox.addWidget(&starportTimerLabel,
                             topBox.getWidgetPosition(&topBoxHBox) + topBoxHBox.getWidgetPosition(&objPicture)
                                 + Point(0, 4),
                             objPicture.getSize());
        }
    }

    void onUpgrade() const {
        auto* const pObject  = currentGame->getObjectManager().getObject(objectID);
        auto* const pBuilder = dynamic_cast<BuilderBase*>(pObject);
        if (pBuilder != nullptr && !pBuilder->isUpgrading()) {
            pBuilder->handleUpgradeClick();
        }
    }

    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override {
        auto* const pObject = currentGame->getObjectManager().getObject(objectID);
        if (pObject == nullptr) {
            return false;
        }

        auto* const pBuilder = dynamic_cast<BuilderBase*>(pObject);
        if (pBuilder != nullptr) {
            auto* const pStarport = dynamic_cast<StarPort*>(pBuilder);
            if (pStarport != nullptr) {
                const auto arrivalTimer = pStarport->getArrivalTimer();
                if (arrivalTimer > 0) {
                    const int seconds = ((arrivalTimer * 10) / (MILLI2CYCLES(30 * 1000))) + 1;
                    starportTimerLabel.setText(std::to_string(seconds));
                } else {
                    starportTimerLabel.setText("");
                }
            }

            upgradeProgressBar.setVisible(pBuilder->isUpgrading());
            upgradeButton.setVisible(!pBuilder->isUpgrading());
            upgradeButton.setEnabled(!pBuilder->isUpgrading());

            if (pBuilder->isUpgrading()) {
                upgradeProgressBar.setProgress(
                    ((pBuilder->getUpgradeProgress() * 100) / pBuilder->getUpgradeCost(context_)).toDouble());
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
        }

        return true;
    }

    TextButton upgradeButton;
    TextProgressBar upgradeProgressBar;
    Label starportTimerLabel;
    BuilderList* pBuilderList;
};

#endif // BUILDERINTERFACE_H
