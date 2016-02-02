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
		PalaceInterface* tmp = new PalaceInterface(objectID);
		tmp->pAllocated = true;
		return tmp;
	}

protected:
	PalaceInterface(int objectID) : DefaultStructureInterface(objectID) {
		mainHBox.addWidget(&weaponBox);

		SDL_Surface* pSurface = pGFXManager->getSmallDetailPic(Picture_DeathHand);
		weaponBox.addWidget(&weaponProgressBar,Point((SIDEBARWIDTH - 25 - pSurface->w)/2,5),
									Point(pSurface->w, pSurface->h));

		weaponBox.addWidget(&weaponSelectButton,Point((SIDEBARWIDTH - 25 - pSurface->w)/2,5),
									Point(pSurface->w, pSurface->h));

		SDL_Surface* pText = pFontManager->createSurfaceWithText(_("READY"), COLOR_WHITE, FONT_STD10);

		SDL_Surface* pReady = SDL_CreateRGBSurface(SDL_HWSURFACE, pSurface->w, pSurface->h, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK);
		SDL_FillRect(pReady, NULL, COLOR_TRANSPARENT);

		SDL_Rect dest = { static_cast<Sint16>((pReady->w - pText->w)/2),static_cast<Sint16>((pReady->h - pText->h)/2), static_cast<Uint16>(pText->w), static_cast<Uint16>(pText->h) };
		SDL_BlitSurface(pText, NULL, pReady, &dest);

		SDL_FreeSurface(pText);
		weaponSelectButton.setSurfaces(pReady,true);
		weaponSelectButton.setVisible(false);

		weaponSelectButton.setOnClick(std::bind(&PalaceInterface::onSpecial, this));
	}

	/**
		This method updates the object interface.
		If the object doesn't exists anymore then update returns false.
		\return true = everything ok, false = the object container should be removed
	*/
	virtual bool update() {
		ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
		if(pObject == NULL) {
			return false;
		}

		Palace* pPalace = dynamic_cast<Palace*>(pObject);

		if(pPalace != NULL) {
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

			weaponProgressBar.setSurface(pGFXManager->getSmallDetailPic(picID),false);
			weaponProgressBar.setProgress(pPalace->getPercentComplete());

			weaponSelectButton.setVisible(pPalace->isSpecialWeaponReady());
		}

		return DefaultStructureInterface::update();
	}

private:
	void onSpecial() {
		ObjectBase* pObject = currentGame->getObjectManager().getObject(objectID);
		if(pObject == NULL) {
			return;
		}

		Palace* pPalace = dynamic_cast<Palace*>(pObject);

		if(pPalace != NULL) {
		    if((pPalace->getOriginalHouseID() == HOUSE_HARKONNEN) || (pPalace->getOriginalHouseID() == HOUSE_SARDAUKAR)) {
                currentGame->currentCursorMode = Game::CursorMode_Attack;
		    } else {
                pPalace->handleSpecialClick();
		    }
		}
	};

	StaticContainer		weaponBox;
	PictureProgressBar	weaponProgressBar;
	PictureButton		weaponSelectButton;
};

#endif // PALACEINTERFACE_H
