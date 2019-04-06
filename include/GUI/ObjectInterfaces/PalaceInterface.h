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

#ifndef PALACEINTERFACE_H
#define PALACEINTERFACE_H

#include "DefaultStructureInterface.h"

#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>

#include <GUI/ProgressBar.h>

#include <structures/Palace.h>

class PalaceInterface : public DefaultStructureInterface {
public:
    static PalaceInterface* create(int objectID) {
        const auto tmp = new PalaceInterface(objectID);
        tmp->pAllocated = true;
        return tmp;
    }

protected:
    explicit PalaceInterface(int objectID) : DefaultStructureInterface(objectID) {
        mainHBox.addWidget(&weaponBox);

        SDL_Texture* pTexture = pGFXManager->getSmallDetailPic(Picture_DeathHand);
        weaponBox.addWidget(&weaponProgressBar, Point((SIDEBARWIDTH - 25 - getWidth(pTexture))/2,5), getTextureSize(pTexture));

        weaponBox.addWidget(&weaponSelectButton, Point((SIDEBARWIDTH - 25 - getWidth(pTexture))/2,5), getTextureSize(pTexture));

        sdl2::surface_ptr pText{ pFontManager->createSurfaceWithText(_("READY"), COLOR_WHITE, 12) };

        sdl2::surface_ptr pReady{ SDL_CreateRGBSurface(0, getWidth(pTexture), getHeight(pTexture), SCREEN_BPP, RMASK, GMASK, BMASK, AMASK) };
        SDL_FillRect(pReady.get(), nullptr, COLOR_TRANSPARENT);

        SDL_Rect dest = calcAlignedDrawingRect(pText.get(), pReady.get());
        SDL_BlitSurface(pText.get(), nullptr, pReady.get(), &dest);

        weaponSelectButton.setTextures(convertSurfaceToTexture(pReady.get()));
        weaponSelectButton.setVisible(false);

        weaponSelectButton.setOnClick(std::bind(&PalaceInterface::onSpecial, this));
    }

    /**
        This method updates the object interface.
        If the object doesn't exists anymore then update returns false.
        \return true = everything ok, false = the object container should be removed
    */
    bool update() override
    {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        if(pObject == nullptr) {
            return false;
        }

        Palace* pPalace = dynamic_cast<Palace*>(pObject);
        if(pPalace != nullptr) {
            int picID;

            switch(pPalace->getOwner()->getHouseID()) {
                case HOUSE_HARKONNEN:
                case HOUSE_SARDAUKAR: {
                    picID = Picture_DeathHand;
                } break;

                case HOUSE_ATREIDES:
                case HOUSE_FREMEN: {
                    picID = Picture_Fremen;
                } break;

                case HOUSE_ORDOS:
                case HOUSE_MERCENARY: {
                    picID = Picture_Saboteur;
                } break;

                default: {
                    picID = Picture_Fremen;
                } break;
            }

            weaponProgressBar.setTexture(pGFXManager->getSmallDetailPic(picID));
            weaponProgressBar.setProgress(pPalace->getPercentComplete());

            weaponSelectButton.setVisible(pPalace->isSpecialWeaponReady());
        }

        return DefaultStructureInterface::update();
    }

private:
    void onSpecial() {
        ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
        if(pObject == nullptr) {
            return;
        }

        Palace* pPalace = dynamic_cast<Palace*>(pObject);
        if(pPalace != nullptr) {
            if((pPalace->getOriginalHouseID() == HOUSE_HARKONNEN) || (pPalace->getOriginalHouseID() == HOUSE_SARDAUKAR)) {
                currentGame->currentCursorMode = Game::CursorMode_Attack;
            } else {
                pPalace->handleSpecialClick();
            }
        }
    };

    StaticContainer     weaponBox;
    PictureProgressBar  weaponProgressBar;
    PictureButton       weaponSelectButton;
};

#endif // PALACEINTERFACE_H
