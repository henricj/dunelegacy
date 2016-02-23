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

#include <GUI/dune/BuilderList.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/TextManager.h>
#include <misc/draw_util.h>

#include <Game.h>
#include <House.h>
#include <SoundPlayer.h>
#include <sand.h>

#include <structures/BuilderBase.h>
#include <structures/StarPort.h>

#include <sstream>

BuilderList::BuilderList(Uint32 builderObjectID) : StaticContainer() {
	enableResizing(false,true);
	this->builderObjectID = builderObjectID;

	SDL_Surface* surf, *surfPressed;

	surf = pGFXManager->getUIGraphic(UI_ButtonUp,pLocalHouse->getHouseID());
	surfPressed = pGFXManager->getUIGraphic(UI_ButtonUp_Pressed,pLocalHouse->getHouseID());

	upButton.setSurfaces(surf,false,surfPressed,false);
	upButton.resize(surf->w,surf->h);

	surf = pGFXManager->getUIGraphic(UI_ButtonDown,pLocalHouse->getHouseID());
	surfPressed = pGFXManager->getUIGraphic(UI_ButtonDown_Pressed,pLocalHouse->getHouseID());

	downButton.setSurfaces(surf,false,surfPressed,false);
	downButton.resize(surf->w,surf->h);

	addWidget(&upButton,Point( (WIDGET_WIDTH - ARROWBTN_WIDTH)/2,0),upButton.getSize());
	upButton.setOnClick(std::bind(&BuilderList::onUp, this));

	addWidget(&downButton,
				Point( (WIDGET_WIDTH - ARROWBTN_WIDTH)/2,
						(ARROWBTN_HEIGHT + BUILDERBTN_SPACING)),
				downButton.getSize());
	downButton.setOnClick(std::bind(&BuilderList::onDown, this));

	addWidget(&orderButton,
				Point(0,(ARROWBTN_HEIGHT + BUILDERBTN_SPACING) + BUILDERBTN_SPACING),
				Point(WIDGET_WIDTH,ORDERBTN_HEIGHT));
	orderButton.setOnClick(std::bind(&BuilderList::onOrder, this));
	orderButton.setText(_("Order"));

	currentListPos = 0;

	mouseLeftButton = -1;
	mouseRightButton = -1;

	pLastTooltip = NULL;
	tooltipText = "";
	lastMouseMovement = (1u<<31);

	resize(getMinimumSize().x,getMinimumSize().y);
}

BuilderList::~BuilderList() {
	if(pLastTooltip != NULL) {
		SDL_FreeSurface(pLastTooltip);
		pLastTooltip = NULL;
	}
}

void BuilderList::handleMouseMovement(Sint32 x, Sint32 y, bool insideOverlay) {
	StaticContainer::handleMouseMovement(x,y,insideOverlay);

	if((x >= 0) && (x < getSize().x) && (y >= 0) && (y < getSize().y) && !insideOverlay) {
        lastMouseMovement = SDL_GetTicks();
		lastMousePos.x = x;
		lastMousePos.y = y;
	} else {
        lastMousePos.x = INVALID_POS;
        lastMousePos.y = INVALID_POS;
	}
}

bool BuilderList::handleMouseLeft(Sint32 x, Sint32 y, bool pressed) {
	StaticContainer::handleMouseLeft(x,y,pressed);

    BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderObjectID));

	StarPort* pStarport = dynamic_cast<StarPort*>(pBuilder);
	if((pStarport != NULL) && (pStarport->okToOrder() == false)) {
		return false;
	}

	if(pressed == true) {
		mouseLeftButton = getButton(x,y);
	} else {
		if(mouseLeftButton == getButton(x,y)) {
			// button released
			if((getItemIDFromIndex(mouseLeftButton) == (int) pBuilder->getCurrentProducedItem()) && (pBuilder->isWaitingToPlace() == true)) {
				soundPlayer->playSound(ButtonClick);
				if(currentGame->currentCursorMode == Game::CursorMode_Placing) {
                    currentGame->currentCursorMode = Game::CursorMode_Normal;
				} else {
				    currentGame->currentCursorMode = Game::CursorMode_Placing;
				}
			} else if((getItemIDFromIndex(mouseLeftButton) == (int) pBuilder->getCurrentProducedItem()) && (pBuilder->isOnHold() == true)) {
				soundPlayer->playSound(ButtonClick);
                pBuilder->handleSetOnHoldClick(false);
			} else {
			    if(getItemIDFromIndex(mouseLeftButton) != ItemID_Invalid) {
                    soundPlayer->playSound(ButtonClick);
                    pBuilder->handleProduceItemClick(getItemIDFromIndex(mouseLeftButton), SDL_GetModState() & KMOD_SHIFT);
			    }
			}
		}

		mouseLeftButton = -1;
	}

	return true;
}

bool BuilderList::handleMouseRight(Sint32 x, Sint32 y, bool pressed) {
	StaticContainer::handleMouseRight(x,y,pressed);

    BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderObjectID));

	StarPort* pStarport = dynamic_cast<StarPort*>(pBuilder);
	if((pStarport != NULL) && (pStarport->okToOrder() == false)) {
		return false;
	}

	if(pressed == true) {
		mouseRightButton = getButton(x,y);
	} else {
		if(mouseRightButton == getButton(x,y)) {
			// button released
			if((getItemIDFromIndex(mouseRightButton) == (int) pBuilder->getCurrentProducedItem()) && (pBuilder->isOnHold() == false)) {
				soundPlayer->playSound(ButtonClick);
				pBuilder->handleSetOnHoldClick(true);
			} else {
			    if(getItemIDFromIndex(mouseRightButton) != ItemID_Invalid) {
			        soundPlayer->playSound(ButtonClick);
                    pBuilder->handleCancelItemClick(getItemIDFromIndex(mouseRightButton), SDL_GetModState() & KMOD_SHIFT);
			    }
			}
		}

		mouseRightButton = -1;
	}

	return true;
}

bool BuilderList::handleMouseWheel(Sint32 x, Sint32 y, bool up) {
	if((x >= 0) && (x < getSize().x) && (y >= 0) && (y < getSize().y)) {
        if(up) {
            onUp();
        } else {
            onDown();
        }
        return true;
	} else {
        return false;
	}
}

bool BuilderList::handleKeyPress(SDL_KeyboardEvent& key) {
	return StaticContainer::handleKeyPress(key);
}

void BuilderList::draw(SDL_Surface* screen, Point position) {
	SDL_Rect blackRectDest = {	position.x, position.y + ARROWBTN_HEIGHT + BUILDERBTN_SPACING,
                                getSize().x, getRealHeight(getSize().y) - 2*(ARROWBTN_HEIGHT + BUILDERBTN_SPACING) - BUILDERBTN_SPACING - ORDERBTN_HEIGHT };
	SDL_FillRect(screen, &blackRectDest, COLOR_BLACK);

    BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderObjectID));
	if(pBuilder != NULL) {
		StarPort* pStarport = dynamic_cast<StarPort*>(pBuilder);

		if(pStarport != NULL) {
			orderButton.setVisible(true);
			orderButton.setEnabled(pStarport->okToOrder());
		} else {
			orderButton.setVisible(false);
		}

		if(getNumButtons(getSize().y) < (int) pBuilder->getBuildList().size()) {
            upButton.setVisible(true);
            downButton.setVisible(true);

            if(currentListPos == 0) {
                upButton.setEnabled(false);
            } else {
                upButton.setEnabled(true);
            }

            if(currentListPos + getNumButtons(getSize().y) < (int) pBuilder->getBuildList().size() ) {
                downButton.setEnabled(true);
            } else {
                downButton.setEnabled(false);
            }
		} else {
            upButton.setVisible(false);
            downButton.setVisible(false);
		}

		int i = 0;
		std::list<BuildItem>::const_iterator iter;
		for(iter = pBuilder->getBuildList().begin(); iter != pBuilder->getBuildList().end(); ++iter, i++) {

			if((i >= currentListPos) && (i < currentListPos+getNumButtons(getSize().y) )) {
				SDL_Surface* pSurface = resolveItemPicture(iter->itemID);

                SDL_Rect dest = calcDrawingRect(pSurface, position.x + getButtonPosition(i - currentListPos).x, position.y + getButtonPosition(i - currentListPos).y);

                if(pSurface != NULL) {
                    SDL_Rect tmpDest = dest;
                    SDL_BlitSurface(pSurface, NULL, screen, &tmpDest);
                }

				if(isStructure(iter->itemID)) {
                    SDL_Surface* pLattice = pGFXManager->getUIGraphic(UI_StructureSizeLattice);
                    SDL_Rect destLattice = calcDrawingRect(pLattice, dest.x + 2, dest.y + 2);
                    SDL_BlitSurface(pLattice, NULL, screen, &destLattice);

                    SDL_Surface* pConcrete = pGFXManager->getUIGraphic(UI_StructureSizeConcrete);
                    SDL_Rect srcConcrete = { 0, 0, 1 + getStructureSize(iter->itemID).x*6, 1 + getStructureSize(iter->itemID).y*6 };
                    SDL_Rect destConcrete = { dest.x + 2, dest.y + 2, srcConcrete.w, srcConcrete.h };
                    SDL_BlitSurface(pConcrete, &srcConcrete, screen, &destConcrete);
				}

				// draw price
				char text[50];
				sprintf(text, "%d", iter->price);
				SDL_Surface* textSurface = pFontManager->createSurfaceWithText(text, COLOR_WHITE, FONT_STD10);
				SDL_Rect drawLocation = calcDrawingRect(textSurface, dest.x + 2, dest.y + BUILDERBTN_HEIGHT - textSurface->h + 3);
				SDL_BlitSurface(textSurface, NULL, screen, &drawLocation);
				SDL_FreeSurface(textSurface);

				if(pStarport != NULL) {
				    bool soldOut = (pStarport->getOwner()->getChoam().getNumAvailable(iter->itemID) == 0);

					if((pStarport->okToOrder() == false) || (soldOut == true)) {
                        SDL_Rect progressBar = { dest.x, dest.y, BUILDERBTN_WIDTH, BUILDERBTN_HEIGHT };
                        SDL_Surface* progressSurface = SDL_CreateRGBSurface(0, BUILDERBTN_WIDTH, BUILDERBTN_HEIGHT, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK);

                        SDL_FillRect(progressSurface, NULL, COLOR_HALF_TRANSPARENT);
                        SDL_BlitSurface(progressSurface, NULL, screen, &progressBar);
                        SDL_FreeSurface(progressSurface);
					}

					if(soldOut == true) {
						SDL_Surface* textSurface = pFontManager->createSurfaceWithMultilineText(_("SOLD OUT"), COLOR_WHITE, FONT_STD10, true);
						SDL_Rect drawLocation = calcDrawingRect(textSurface, dest.x + BUILDERBTN_WIDTH/2, dest.y + BUILDERBTN_HEIGHT/2, HAlign::Center, VAlign::Center);
						SDL_BlitSurface(textSurface, NULL, screen, &drawLocation);
						SDL_FreeSurface(textSurface);
					}

				} else if(currentGame->getGameInitSettings().getGameOptions().onlyOnePalace && iter->itemID == Structure_Palace && pBuilder->getOwner()->getNumItems(Structure_Palace) > 0) {

                    SDL_Rect progressBar = { dest.x, dest.y, BUILDERBTN_WIDTH, BUILDERBTN_HEIGHT };
                    SDL_Surface* progressSurface = SDL_CreateRGBSurface(0, BUILDERBTN_WIDTH, BUILDERBTN_HEIGHT, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK);

                    SDL_FillRect(progressSurface, NULL, COLOR_HALF_TRANSPARENT);
                    SDL_BlitSurface(progressSurface, NULL, screen, &progressBar);
                    SDL_FreeSurface(progressSurface);

                    SDL_Surface* textSurface = pFontManager->createSurfaceWithMultilineText(_("ALREADY\nBUILT"), COLOR_WHITE, FONT_STD10, true);
                    SDL_Rect drawLocation = calcDrawingRect(textSurface, dest.x + BUILDERBTN_WIDTH/2, dest.y + BUILDERBTN_HEIGHT/2, HAlign::Center, VAlign::Center);
                    SDL_BlitSurface(textSurface, NULL, screen, &drawLocation);
                    SDL_FreeSurface(textSurface);
				} else if(iter->itemID == pBuilder->getCurrentProducedItem()) {
					FixPoint progress = pBuilder->getProductionProgress();
					FixPoint price = iter->price;
					int max_x = lround((progress/price)*BUILDERBTN_WIDTH);

                    SDL_Rect progressBar = { dest.x, dest.y, max_x, BUILDERBTN_HEIGHT };
                    SDL_Surface* progressSurface = SDL_CreateRGBSurface(0, max_x, BUILDERBTN_HEIGHT, SCREEN_BPP, RMASK, GMASK, BMASK, AMASK);

                    SDL_FillRect(progressSurface, NULL, COLOR_HALF_TRANSPARENT);
                    SDL_BlitSurface(progressSurface, NULL, screen, &progressBar);
                    SDL_FreeSurface(progressSurface);

					if(pBuilder->isWaitingToPlace() == true) {
						SDL_Surface* textSurface = pFontManager->createSurfaceWithMultilineText(_("PLACE IT"), COLOR_WHITE, FONT_STD10, true);
						SDL_Rect drawLocation = calcDrawingRect(textSurface, dest.x + BUILDERBTN_WIDTH/2, dest.y + BUILDERBTN_HEIGHT/2, HAlign::Center, VAlign::Center);
						SDL_BlitSurface(textSurface, NULL, screen, &drawLocation);
						SDL_FreeSurface(textSurface);
					} else if(pBuilder->isOnHold() == true) {
						SDL_Surface* textSurface = pFontManager->createSurfaceWithMultilineText(_("ON HOLD"), COLOR_WHITE, FONT_STD10, true);
						SDL_Rect drawLocation = calcDrawingRect(textSurface, dest.x + BUILDERBTN_WIDTH/2, dest.y + BUILDERBTN_HEIGHT/2, HAlign::Center, VAlign::Center);
						SDL_BlitSurface(textSurface, NULL, screen, &drawLocation);
						SDL_FreeSurface(textSurface);
					}
				}

				if(iter->num > 0) {
					// draw number of this in build list
					sprintf(text, "%d", iter->num);
					textSurface = pFontManager->createSurfaceWithText(text, COLOR_RED, FONT_STD10);
                    SDL_Rect drawLocation = calcDrawingRect(textSurface, dest.x + BUILDERBTN_WIDTH - 3, dest.y + BUILDERBTN_HEIGHT + 2, HAlign::Right, VAlign::Bottom);
					SDL_BlitSurface(textSurface, NULL, screen, &drawLocation);
					SDL_FreeSurface(textSurface);
				}
			}
		}
	}

	SDL_Surface* pBuilderListUpperCap = pGFXManager->getUIGraphic(UI_BuilderListUpperCap);
	SDL_Rect builderListUpperCapDest = calcDrawingRect(pBuilderListUpperCap, blackRectDest.x - 3, blackRectDest.y - 13 + 4);
    SDL_BlitSurface(pBuilderListUpperCap, NULL, screen, &builderListUpperCapDest);

	SDL_Surface* pBuilderListLowerCap = pGFXManager->getUIGraphic(UI_BuilderListLowerCap);
	SDL_Rect builderListLowerCapDest = calcDrawingRect(pBuilderListLowerCap, blackRectDest.x - 3, blackRectDest.y + blackRectDest.h - 3 - 4);
    SDL_BlitSurface(pBuilderListLowerCap, NULL, screen, &builderListLowerCapDest);

    drawVLine(screen,builderListUpperCapDest.x + builderListUpperCapDest.w - 8, builderListUpperCapDest.y + builderListUpperCapDest.h, builderListLowerCapDest.y, 227);

	StaticContainer::draw(screen,position);
}

void BuilderList::drawOverlay(SDL_Surface* screen, Point position) {
	if((SDL_GetTicks() - lastMouseMovement) > 800) {
		// Draw tooltip

		int btn = getButton(lastMousePos.x,lastMousePos.y);

		if(btn != -1) {

            BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderObjectID));

			int j = 0;
			std::list<BuildItem>::const_iterator iter;
			for(iter = pBuilder->getBuildList().begin(); iter != pBuilder->getBuildList().end(); ++iter, j++) {
				if(j == btn) {
					break;
				}
			}

            std::string text = resolveItemName(iter->itemID);

            if(iter->itemID == pBuilder->getCurrentProducedItem() && pBuilder->isWaitingToPlace()) {
                text += " (Hotkey: P)";
            }

			if(text != tooltipText) {
				if(pLastTooltip != NULL) {
					SDL_FreeSurface(pLastTooltip);
					pLastTooltip = NULL;
				}
			}

			if(pLastTooltip == NULL) {
				pLastTooltip = GUIStyle::getInstance().createToolTip(text);
				tooltipText = text;
			}

			SDL_Rect dest = calcDrawingRect(pLastTooltip, position.x + getButtonPosition(btn).x - 6, position.y + lastMousePos.y, HAlign::Right, VAlign::Center);
			SDL_BlitSurface(pLastTooltip, NULL, screen, &dest);
		}

	}
}


void BuilderList::resize(Uint32 width, Uint32 height) {
	setWidgetGeometry(  &upButton,Point( (WIDGET_WIDTH - ARROWBTN_WIDTH)/2,-2),upButton.getSize());
	setWidgetGeometry(  &downButton,
						Point( (WIDGET_WIDTH - ARROWBTN_WIDTH)/2, getRealHeight(height) - ARROWBTN_HEIGHT - ORDERBTN_HEIGHT - BUILDERBTN_SPACING + 2),
						downButton.getSize());

	setWidgetGeometry(  &orderButton,
						Point( 0, getRealHeight(height) - ORDERBTN_HEIGHT + 2),
						Point(WIDGET_WIDTH,ORDERBTN_HEIGHT));

	StaticContainer::resize(width,height);


	// move list to show currently produced item
    BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderObjectID));
	if(pBuilder != NULL) {
        int currentProducedItemPos = 0;
        for(std::list<BuildItem>::const_iterator iter = pBuilder->getBuildList().begin();
             iter != pBuilder->getBuildList().end();
             ++iter, ++currentProducedItemPos)
        {
            if(iter->itemID == pBuilder->getCurrentProducedItem()) {
                break;
            }
        }

        if(currentProducedItemPos < (int)pBuilder->getBuildList().size()) {
            int biggestLegalPosition = ((int)pBuilder->getBuildList().size()) - getNumButtons(getSize().y);
            currentListPos = std::max(0, std::min(currentProducedItemPos-1,biggestLegalPosition));
        }
	}
}

int BuilderList::getRealHeight(int height) {
	int tmp = height;
	tmp -= (ARROWBTN_HEIGHT + BUILDERBTN_SPACING)*2;
	tmp -= BUILDERBTN_SPACING;
	tmp -= ORDERBTN_HEIGHT;
	tmp -= BUILDERBTN_SPACING;
	int numButtons = tmp / (BUILDERBTN_HEIGHT + BUILDERBTN_SPACING);

	return numButtons * (BUILDERBTN_HEIGHT + BUILDERBTN_SPACING) + 3*BUILDERBTN_SPACING + 2*ARROWBTN_HEIGHT + ORDERBTN_HEIGHT + BUILDERBTN_SPACING;
}

void BuilderList::onUp() {
	if(currentListPos > 0) {
		currentListPos--;
	}
}

void BuilderList::onDown() {
    BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderObjectID));

	if(currentListPos < ((int)pBuilder->getBuildList().size()) - getNumButtons(getSize().y)) {
		currentListPos++;
	}
}

void BuilderList::onOrder() {
    BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderObjectID));

	StarPort* pStarport = dynamic_cast<StarPort*>(pBuilder);
	if(pStarport != NULL) {
		pStarport->handlePlaceOrderClick();
	}
}

int BuilderList::getNumButtons(int height) {
	int tmp = height;
	tmp -= (ARROWBTN_HEIGHT + BUILDERBTN_SPACING)*2;
	tmp -= BUILDERBTN_SPACING;
	tmp -= ORDERBTN_HEIGHT;
	tmp -= BUILDERBTN_SPACING;
	return tmp / (BUILDERBTN_HEIGHT + BUILDERBTN_SPACING);
}

Point BuilderList::getButtonPosition(int BtnNumber) {
	return Point(BUILDERBTN_SPACING,
					ARROWBTN_HEIGHT+2*BUILDERBTN_SPACING
					+ BtnNumber*(BUILDERBTN_HEIGHT+BUILDERBTN_SPACING));

}

int BuilderList::getButton(int x, int y) {

    BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderObjectID));

	if(pBuilder != NULL) {
		int i = 0;
		std::list<BuildItem>::const_iterator iter;
		for(iter = pBuilder->getBuildList().begin(); iter != pBuilder->getBuildList().end(); ++iter, i++) {

			if((i >= currentListPos) && (i < currentListPos+getNumButtons(getSize().y) )) {
				if(		(x >= getButtonPosition(i - currentListPos).x)
					&&	(x < getButtonPosition(i - currentListPos).x + BUILDERBTN_WIDTH)
					&&	(y >= getButtonPosition(i - currentListPos).y)
					&&	(y < getButtonPosition(i - currentListPos).y + BUILDERBTN_HEIGHT)) {

					return i;
				}
			}
		}
	}

	return -1;
}

int BuilderList::getItemIDFromIndex(int i) {

    BuilderBase* pBuilder = dynamic_cast<BuilderBase*>(currentGame->getObjectManager().getObject(builderObjectID));

	if(pBuilder != NULL) {
		int j = 0;
		std::list<BuildItem>::const_iterator iter;
		for(iter = pBuilder->getBuildList().begin(); iter != pBuilder->getBuildList().end(); ++iter, j++) {
			if(j == i) {
				return iter->itemID;
			}
		}
	}

	return ItemID_Invalid;
}
