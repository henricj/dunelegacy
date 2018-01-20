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

#ifndef RADIOBUTTONMANAGER_H
#define RADIOBUTTONMANAGER_H

#include <stdarg.h>
#include <vector>

class RadioButton;

/// A class for managing a bunch of radio buttons
class RadioButtonManager {
public:
    /// Default constructor
    RadioButtonManager()  = default;

    /// destructor
    virtual ~RadioButtonManager() {
        while(radioButtonList.empty() == false) {
            unregisterRadioButton(*radioButtonList.begin());
        }
    }

    void registerRadioButton(RadioButton* pRadioButton);

    void registerRadioButtons(int numRadioButtons, ...) {
        va_list valist;
        va_start(valist,numRadioButtons);

        for(int i=0;i<numRadioButtons;i++) {
            RadioButton* pRadioButton = va_arg(valist, RadioButton*);
            registerRadioButton(pRadioButton);
        }
        va_end(valist);
    }

    void unregisterRadioButton(RadioButton* pRadioButton);

    bool isRegistered(RadioButton* pRadioButton) const {
        for(const RadioButton* pTmpRadioButton : radioButtonList) {
            if(pTmpRadioButton == pRadioButton) {
                return true;
            }
        }

        return false;
    }

    void setChecked(RadioButton* pRadioButton) const;


private:
    std::vector<RadioButton*>   radioButtonList;
};

#endif // RADIOBUTTONMANAGER_H
