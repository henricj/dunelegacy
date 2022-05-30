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

#include <GUI/RadioButton.h>
#include <GUI/RadioButtonManager.h>

#include <ranges>

RadioButtonManager::RadioButtonManager() = default;

RadioButtonManager::~RadioButtonManager() {
    while (!radioButtonList.empty()) {
        unregisterRadioButton(radioButtonList.front());
    }
}

void RadioButtonManager::registerRadioButtons(std::initializer_list<RadioButton*> buttons) {
    for (auto* pRadioButton : buttons)
        registerRadioButton(pRadioButton);
}

bool RadioButtonManager::isRegistered(RadioButton* pRadioButton) {
    return std::end(radioButtonList) != std::ranges::find(radioButtonList, pRadioButton);
}

void RadioButtonManager::registerRadioButton(RadioButton* pRadioButton) {
    if (!isRegistered(pRadioButton)) {
        radioButtonList.push_back(pRadioButton);
    }
    pRadioButton->registerRadioButtonManager(this);
}

void RadioButtonManager::unregisterRadioButton(RadioButton* pRadioButton) {

    for (auto iter = radioButtonList.begin(); iter != radioButtonList.end(); ++iter) {
        if (*iter == pRadioButton) {
            radioButtonList.erase(iter);
            break;
        }
    }

    pRadioButton->unregisterFromRadioButtonManager();
}

void RadioButtonManager::setChecked(RadioButton* pRadioButton) const {
    for (auto* pTmpRadioButton : radioButtonList) {
        if (pTmpRadioButton == pRadioButton) {
            pTmpRadioButton->Button::setToggleState(true);
        } else {
            pTmpRadioButton->Button::setToggleState(false);
        }
    }
}
