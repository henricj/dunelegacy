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

#include <GUI/RadioButtonManager.h>
#include <GUI/RadioButton.h>

void RadioButtonManager::registerRadioButton(RadioButton* pRadioButton) {
    if(isRegistered(pRadioButton) == false) {
        radioButtonList.push_back(pRadioButton);
    }
    pRadioButton->registerRadioButtonManager(this);
}

void RadioButtonManager::unregisterRadioButton(RadioButton* pRadioButton) {
    std::vector<RadioButton*>::iterator iter = radioButtonList.begin();
    while(iter != radioButtonList.end()) {
        if(*iter == pRadioButton) {
            radioButtonList.erase(iter);
            break;
        }
        ++iter;
    }

    pRadioButton->unregisterFromRadioButtonManager();
}

void RadioButtonManager::setChecked(RadioButton* pRadioButton) const {
    for(RadioButton* pTmpRadioButton : radioButtonList) {
        if(pTmpRadioButton == pRadioButton) {
            pTmpRadioButton->Button::setToggleState(true);
        } else {
            pTmpRadioButton->Button::setToggleState(false);
        }
    }
}
