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

#include <FileClasses/Font.h>
#include <FileClasses/FontManager.h>
#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <misc/draw_util.h>

#include <Game.h>
#include <House.h>
#include <SoundPlayer.h>
#include <sand.h>

#include <structures/BuilderBase.h>
#include <structures/StarPort.h>

BuilderList::BuilderList(uint32_t builderObjectID) : builderObjectID(builderObjectID) {
    BuilderList::enableResizing(false, true);

    const auto* const gfx = dune::globals::pGFXManager.get();

    const auto house_id = dune::globals::pLocalHouse->getHouseID();

    upButton.setTextures(gfx->getUIGraphic(UI_ButtonUp, house_id), gfx->getUIGraphic(UI_ButtonUp_Pressed, house_id));

    downButton.setTextures(gfx->getUIGraphic(UI_ButtonDown, house_id),
                           gfx->getUIGraphic(UI_ButtonDown_Pressed, house_id));

    StaticContainer::addWidget(&upButton, Point((WIDGET_WIDTH - ARROWBTN_WIDTH) / 2, 0), upButton.getSize());
    upButton.setOnClick([this] { onUp(); });

    StaticContainer::addWidget(&downButton,
                               Point((WIDGET_WIDTH - ARROWBTN_WIDTH) / 2, ARROWBTN_HEIGHT + BUILDERBTN_SPACING),
                               downButton.getSize());
    downButton.setOnClick([this] { onDown(); });

    StaticContainer::addWidget(&orderButton, Point(0, ARROWBTN_HEIGHT + BUILDERBTN_SPACING + BUILDERBTN_SPACING),
                               Point(WIDGET_WIDTH, ORDERBTN_HEIGHT));
    orderButton.setOnClick([this] { onOrder(); });
    orderButton.setText(_("Order"));

    const auto& gui = GUIStyle::getInstance();

    auto* const renderer = dune::globals::renderer.get();

    pSoldOutTextTexture          = gui.createMultilineText(renderer, _("SOLD OUT"), COLOR_WHITE, 12, true);
    pAlreadyBuiltTextTexture     = gui.createMultilineText(renderer, _("ALREADY\nBUILT"), COLOR_WHITE, 12, true);
    pPlaceItTextTexture          = gui.createMultilineText(renderer, _("PLACE IT"), COLOR_WHITE, 12, true);
    pOnHoldTextTexture           = gui.createMultilineText(renderer, _("ON HOLD"), COLOR_WHITE, 12, true);
    pUnitLimitReachedTextTexture = gui.createMultilineText(renderer, _("UNIT LIMIT\nREACHED"), COLOR_WHITE, 12, true);

    resize(BuilderList::getMinimumSize().x, BuilderList::getMinimumSize().y);
}

BuilderList::~BuilderList() = default;

void BuilderList::handleMouseMovement(int32_t x, int32_t y, bool insideOverlay) {
    StaticContainer::handleMouseMovement(x, y, insideOverlay);

    if (x >= 0 && x < getSize().x && y >= 0 && y < getSize().y && !insideOverlay) {
        lastMouseMovement = dune::dune_clock::now();
        lastMousePos.x    = x;
        lastMousePos.y    = y;
    } else {
        lastMousePos.x = INVALID_POS;
        lastMousePos.y = INVALID_POS;
    }
}

bool BuilderList::handleMouseLeft(int32_t x, int32_t y, bool pressed) {
    StaticContainer::handleMouseLeft(x, y, pressed);

    auto* const currentGame = dune::globals::currentGame.get();

    auto* const pBuilder = currentGame->getObjectManager().getObject<BuilderBase>(builderObjectID);
    if (!pBuilder) {
        return false;
    }

    const auto* const pStarport = dune_cast<StarPort>(pBuilder);
    if (pStarport && !pStarport->okToOrder()) {
        return false;
    }

    if (pressed) {
        mouseLeftButton = getButton(x, y);
    } else {
        if (mouseLeftButton == getButton(x, y)) {
            // button released
            assert(pBuilder);

            const auto* const soundPlayer = dune::globals::soundPlayer.get();

            if (getItemIDFromIndex(mouseLeftButton) == static_cast<int>(pBuilder->getCurrentProducedItem())
                && pBuilder->isWaitingToPlace()) {
                soundPlayer->playSound(Sound_enum::Sound_ButtonClick);
                if (currentGame->currentCursorMode == Game::CursorMode_Placing) {
                    currentGame->currentCursorMode = Game::CursorMode_Normal;
                } else {
                    currentGame->currentCursorMode = Game::CursorMode_Placing;
                }
            } else if (getItemIDFromIndex(mouseLeftButton) == static_cast<int>(pBuilder->getCurrentProducedItem())
                       && pBuilder->isOnHold()) {
                soundPlayer->playSound(Sound_enum::Sound_ButtonClick);
                pBuilder->handleSetOnHoldClick(false);
            } else {
                if (getItemIDFromIndex(mouseLeftButton) != ItemID_Invalid) {
                    soundPlayer->playSound(Sound_enum::Sound_ButtonClick);
                    pBuilder->handleProduceItemClick(getItemIDFromIndex(mouseLeftButton),
                                                     SDL_GetModState() & KMOD_SHIFT);
                }
            }
        }

        mouseLeftButton = -1;
    }

    return true;
}

bool BuilderList::handleMouseRight(int32_t x, int32_t y, bool pressed) {
    StaticContainer::handleMouseRight(x, y, pressed);

    auto* pBuilder =
        dynamic_cast<BuilderBase*>(dune::globals::currentGame->getObjectManager().getObject(builderObjectID));
    if (!pBuilder) {
        return false;
    }

    const auto* pStarport = dynamic_cast<StarPort*>(pBuilder);
    if (pStarport && !pStarport->okToOrder()) {
        return false;
    }

    if (pressed) {
        mouseRightButton = getButton(x, y);
    } else {
        if (mouseRightButton == getButton(x, y)) {
            // button released
            assert(pBuilder);

            const auto* const soundPlayer = dune::globals::soundPlayer.get();

            if (getItemIDFromIndex(mouseRightButton) == static_cast<int>(pBuilder->getCurrentProducedItem())
                && !pBuilder->isOnHold()) {
                soundPlayer->playSound(Sound_enum::Sound_ButtonClick);
                pBuilder->handleSetOnHoldClick(true);
            } else {
                if (getItemIDFromIndex(mouseRightButton) != ItemID_Invalid) {
                    soundPlayer->playSound(Sound_enum::Sound_ButtonClick);
                    pBuilder->handleCancelItemClick(getItemIDFromIndex(mouseRightButton),
                                                    SDL_GetModState() & KMOD_SHIFT);
                }
            }
        }

        mouseRightButton = -1;
    }

    return true;
}

bool BuilderList::handleMouseWheel(int32_t x, int32_t y, bool up) {
    if (x >= 0 && x < getSize().x && y >= 0 && y < getSize().y) {
        if (up) {
            onUp();
        } else {
            onDown();
        }
        return true;
    }
    return false;
}

bool BuilderList::handleKeyPress(SDL_KeyboardEvent& key) {
    return StaticContainer::handleKeyPress(key);
}

void BuilderList::draw(Point position) {
    auto* const renderer    = dune::globals::renderer.get();
    auto* const currentGame = dune::globals::currentGame.get();
    auto* const gfx         = dune::globals::pGFXManager.get();

    SDL_Rect blackRectDest{position.x, position.y + ARROWBTN_HEIGHT + BUILDERBTN_SPACING, getSize().x,
                           getRealHeight(getSize().y) - 2 * (ARROWBTN_HEIGHT + BUILDERBTN_SPACING) - BUILDERBTN_SPACING
                               - ORDERBTN_HEIGHT};
    renderFillRect(renderer, &blackRectDest, COLOR_BLACK);

    const auto* const pBuilder = currentGame->getObjectManager().getObject<BuilderBase>(builderObjectID);

    if (pBuilder != nullptr) {
        auto* const pStarport = dune_cast<StarPort>(pBuilder);

        if (pStarport != nullptr) {
            orderButton.setVisible(true);
            orderButton.setEnabled(pStarport->okToOrder());
        } else {
            orderButton.setVisible(false);
        }

        if (getNumButtons(getSize().y) < static_cast<int>(pBuilder->getBuildList().size())) {
            upButton.setVisible(true);
            downButton.setVisible(true);

            if (currentListPos == 0) {
                upButton.setEnabled(false);
            } else {
                upButton.setEnabled(true);
            }

            if (currentListPos + getNumButtons(getSize().y) < static_cast<int>(pBuilder->getBuildList().size())) {
                downButton.setEnabled(true);
            } else {
                downButton.setEnabled(false);
            }
        } else {
            upButton.setVisible(false);
            downButton.setVisible(false);
        }

        int i = 0;
        for (const auto& buildItem : pBuilder->getBuildList()) {

            if (i >= currentListPos && i < currentListPos + getNumButtons(getSize().y)) {
                const auto* pTexture = resolveItemPicture(buildItem.itemID);

                const auto dest_x = position.x + getButtonPosition(i - currentListPos).x;
                const auto dest_y = position.y + getButtonPosition(i - currentListPos).y;

                const auto dest = calcDrawingRectF(pTexture, dest_x, dest_y);

                if (pTexture != nullptr) {
                    pTexture->draw(renderer, dest.x, dest.y);
                }

                if (isStructure(buildItem.itemID)) {
                    if (const auto* const pLattice = dune::globals::pGFXManager->getUIGraphic(UI_StructureSizeLattice))
                        pLattice->draw(renderer, dest.x + 2, dest.y + 2);

                    if (const auto* const pConcrete = gfx->getUIGraphic(UI_StructureSizeConcrete)) {
                        const SDL_Rect srcConcrete   = {0, 0, 1 + getStructureSize(buildItem.itemID).x * 6,
                                                        1 + getStructureSize(buildItem.itemID).y * 6};
                        const SDL_FRect destConcrete = {(dest.x + 2), (dest.y + 2), static_cast<float>(srcConcrete.w),
                                                        static_cast<float>(srcConcrete.h)};
                        Dune_RenderCopyF(renderer, pConcrete, &srcConcrete, &destConcrete);
                    }
                }

                const auto& gui = GUIStyle::getInstance();

                // draw price
                {
                    const auto pPriceTexture =
                        gui.createText(renderer, fmt::sprintf("%d", buildItem.price), COLOR_WHITE, 12);

                    pPriceTexture.draw(renderer, dest.x + 2.f, dest.y + BUILDERBTN_HEIGHT - pPriceTexture.height_ - 3);
                }

                if (pStarport != nullptr) {
                    const auto bSoldOut = pStarport->getOwner()->getChoam().getNumAvailable(buildItem.itemID) == 0;

                    if (!pStarport->okToOrder() || bSoldOut) {
                        SDL_FRect progressBar = {static_cast<float>(dest.x), static_cast<float>(dest.y),
                                                 static_cast<float>(BUILDERBTN_WIDTH),
                                                 static_cast<float>(BUILDERBTN_HEIGHT)};
                        renderFillRectF(renderer, &progressBar, COLOR_HALF_TRANSPARENT);
                    }

                    if (bSoldOut) {
                        const auto x = static_cast<float>(dest.x) + (BUILDERBTN_WIDTH - pSoldOutTextTexture.width_) / 2;
                        const auto y =
                            static_cast<float>(dest.y) + (BUILDERBTN_HEIGHT - pSoldOutTextTexture.height_) / 2;

                        pSoldOutTextTexture.draw(renderer, x, y);
                    }

                } else if (currentGame->getGameInitSettings().getGameOptions().onlyOnePalace
                           && buildItem.itemID == Structure_Palace
                           && pBuilder->getOwner()->getNumItems(Structure_Palace) > 0) {

                    SDL_FRect progressBar = {dest.x, dest.y, BUILDERBTN_WIDTH, BUILDERBTN_HEIGHT};
                    renderFillRectF(renderer, &progressBar, COLOR_HALF_TRANSPARENT);

                    auto drawLocationAlreadyBuilt =
                        calcDrawingRect(pAlreadyBuiltTextTexture, dest.x + BUILDERBTN_WIDTH / 2,
                                        dest.y + BUILDERBTN_HEIGHT / 2, HAlign::Center, VAlign::Center);
                    Dune_RenderCopyF(renderer, pAlreadyBuiltTextTexture.get(), nullptr, &drawLocationAlreadyBuilt);
                } else if (buildItem.itemID == pBuilder->getCurrentProducedItem()) {
                    const auto progress = pBuilder->getProductionProgress();
                    const auto price    = buildItem.price;

                    auto max_x = (progress / price * BUILDERBTN_WIDTH).toFloat();

                    SDL_FRect progressBar{dest.x, dest.y, max_x, BUILDERBTN_HEIGHT};
                    renderFillRectF(renderer, &progressBar, COLOR_HALF_TRANSPARENT);

                    if (pBuilder->isWaitingToPlace()) {
                        const auto x = static_cast<float>(dest.x) + (BUILDERBTN_WIDTH - pPlaceItTextTexture.width_) / 2;
                        const auto y =
                            static_cast<float>(dest.y) + (BUILDERBTN_HEIGHT - pPlaceItTextTexture.height_) / 2;

                        pPlaceItTextTexture.draw(renderer, x, y);
                    } else if (pBuilder->isOnHold()) {
                        const auto x = static_cast<float>(dest.x) + (BUILDERBTN_WIDTH - pOnHoldTextTexture.width_) / 2;
                        const auto y =
                            static_cast<float>(dest.y) + (BUILDERBTN_HEIGHT - pOnHoldTextTexture.height_) / 2;

                        pOnHoldTextTexture.draw(renderer, x, y);
                    } else if (pBuilder->isUnitLimitReached(buildItem.itemID)) {
                        const auto x =
                            static_cast<float>(dest.x) + (BUILDERBTN_WIDTH - pUnitLimitReachedTextTexture.width_) / 2;
                        const auto y =
                            static_cast<float>(dest.y) + (BUILDERBTN_HEIGHT - pUnitLimitReachedTextTexture.height_) / 2;

                        pUnitLimitReachedTextTexture.draw(renderer, x, y);
                    }
                }

                if (buildItem.num > 0) {
                    // draw number of this in build list
                    const auto pNumberTexture = gui.createText(renderer, std::to_string(buildItem.num), COLOR_RED, 12);

                    const auto size = getRendererSize();

                    const auto x = dest.x + BUILDERBTN_WIDTH - 3 - pNumberTexture.width_;
                    const auto y = dest.y + BUILDERBTN_HEIGHT - 2 - pNumberTexture.height_;

                    pNumberTexture.draw(renderer, x, y);
                }
            }

            i++;
        }
    }

    if (const auto* const pBuilderListUpperCap = gfx->getUIGraphic(UI_BuilderListUpperCap)) {
        pBuilderListUpperCap->draw(renderer, blackRectDest.x - 3, blackRectDest.y - 13 + 4);

        const auto builderListUpperCapDest =
            calcDrawingRect(pBuilderListUpperCap, blackRectDest.x - 3, blackRectDest.y - 13 + 4);

        if (const auto* const pBuilderListLowerCap = gfx->getUIGraphic(UI_BuilderListLowerCap)) {
            pBuilderListLowerCap->draw(renderer, blackRectDest.x - 3, blackRectDest.y + blackRectDest.h - 3 - 4);

            const auto builderListLowerCapDest =
                calcDrawingRect(pBuilderListLowerCap, blackRectDest.x - 3, blackRectDest.y + blackRectDest.h - 3 - 4);

            renderDrawVLine(renderer, builderListUpperCapDest.x + builderListUpperCapDest.w - 8,
                            builderListUpperCapDest.y + builderListUpperCapDest.h, builderListLowerCapDest.y,
                            COLOR_RGB(125, 80, 0));
        }
    }

    StaticContainer::draw(position);
}

void BuilderList::drawOverlay(Point position) {
    using namespace std::chrono_literals;

    if (dune::dune_clock::now() - lastMouseMovement <= 800ms)
        return;

    auto* const currentGame = dune::globals::currentGame.get();
    auto* const renderer    = dune::globals::renderer.get();

    // Draw tooltip
    const auto btn = getButton(lastMousePos.x, lastMousePos.y);
    if (btn != -1) {
        const auto* const pBuilder = currentGame->getObjectManager().getObject<BuilderBase>(builderObjectID);
        if (!pBuilder) {
            return;
        }

        const auto buildItemIter = std::next(pBuilder->getBuildList().begin(), btn);

        auto text = resolveItemName(buildItemIter->itemID);

        if (buildItemIter->itemID == pBuilder->getCurrentProducedItem() && pBuilder->isWaitingToPlace()) {
            text += " (Hotkey: P)";
        }

        if (text != tooltipText) {
            pLastTooltip.reset();
        }

        if (!pLastTooltip) {
            pLastTooltip = GUIStyle::getInstance().createToolTip(renderer, text);
            tooltipText  = text;
        }

        const auto dest = calcDrawingRect(pLastTooltip, position.x + getButtonPosition(btn).x - 6,
                                          position.y + lastMousePos.y, HAlign::Right, VAlign::Center);
        Dune_RenderCopyF(renderer, pLastTooltip.get(), nullptr, &dest);
    }
}

void BuilderList::resize(uint32_t width, uint32_t height) {
    setWidgetGeometry(&upButton, Point((WIDGET_WIDTH - ARROWBTN_WIDTH) / 2, -2), upButton.getSize());
    setWidgetGeometry(&downButton,
                      Point((WIDGET_WIDTH - ARROWBTN_WIDTH) / 2,
                            getRealHeight(height) - ARROWBTN_HEIGHT - ORDERBTN_HEIGHT - BUILDERBTN_SPACING + 2),
                      downButton.getSize());

    setWidgetGeometry(&orderButton, Point(0, getRealHeight(height) - ORDERBTN_HEIGHT + 2),
                      Point(WIDGET_WIDTH, ORDERBTN_HEIGHT));

    StaticContainer::resize(width, height);

    auto* const currentGame = dune::globals::currentGame.get();
    if (!currentGame)
        return;

    // move list to show currently produced item
    if (const auto* const pBuilder = currentGame->getObjectManager().getObject<BuilderBase>(builderObjectID)) {
        const auto& buildList = pBuilder->getBuildList();

        const auto currentProducedItemIter = std::ranges::find_if(buildList, [pBuilder](const BuildItem& buildItem) {
            return buildItem.itemID == pBuilder->getCurrentProducedItem();
        });

        if (currentProducedItemIter != buildList.end()) {
            constexpr static auto shiftFromTopPos = 1;

            const auto biggestLegalPosition = static_cast<int>(buildList.size()) - getNumButtons(getSize().y);
            const auto currentProducedItemPos =
                static_cast<int>(std::distance(buildList.begin(), currentProducedItemIter));
            currentListPos = std::max(0, std::min(currentProducedItemPos - shiftFromTopPos, biggestLegalPosition));
        }
    }
}

int BuilderList::getRealHeight(int height) {
    int tmp = height;
    tmp -= (ARROWBTN_HEIGHT + BUILDERBTN_SPACING) * 2;
    tmp -= BUILDERBTN_SPACING;
    tmp -= ORDERBTN_HEIGHT;
    tmp -= BUILDERBTN_SPACING;
    const auto numButtons = tmp / (BUILDERBTN_HEIGHT + BUILDERBTN_SPACING);

    return numButtons * (BUILDERBTN_HEIGHT + BUILDERBTN_SPACING) + 3 * BUILDERBTN_SPACING + 2 * ARROWBTN_HEIGHT
         + ORDERBTN_HEIGHT + BUILDERBTN_SPACING;
}

void BuilderList::onUp() {
    if (currentListPos > 0) {
        currentListPos--;
    }
}

void BuilderList::onDown() {
    auto* const currentGame    = dune::globals::currentGame.get();
    const auto* const pBuilder = currentGame->getObjectManager().getObject<BuilderBase>(builderObjectID);

    if (pBuilder && currentListPos < static_cast<int>(pBuilder->getBuildList().size()) - getNumButtons(getSize().y)) {
        currentListPos++;
    }
}

void BuilderList::onOrder() const {
    auto* const currentGame = dune::globals::currentGame.get();
    if (auto* const pStarport = currentGame->getObjectManager().getObject<StarPort>(builderObjectID)) {
        pStarport->handlePlaceOrderClick();
    }
}

int BuilderList::getNumButtons(int height) {
    int tmp = height;
    tmp -= (ARROWBTN_HEIGHT + BUILDERBTN_SPACING) * 2;
    tmp -= BUILDERBTN_SPACING;
    tmp -= ORDERBTN_HEIGHT;
    tmp -= BUILDERBTN_SPACING;
    return tmp / (BUILDERBTN_HEIGHT + BUILDERBTN_SPACING);
}

Point BuilderList::getButtonPosition(int BtnNumber) {
    return {BUILDERBTN_SPACING,
            ARROWBTN_HEIGHT + 2 * BUILDERBTN_SPACING + BtnNumber * (BUILDERBTN_HEIGHT + BUILDERBTN_SPACING)};
}

int BuilderList::getButton(int x, int y) const {
    auto* const currentGame = dune::globals::currentGame.get();

    if (const auto* pBuilder = currentGame->getObjectManager().getObject<BuilderBase>(builderObjectID)) {
        for (int i = 0; i < static_cast<int>(pBuilder->getBuildList().size()); i++) {
            if (i >= currentListPos && i < currentListPos + getNumButtons(getSize().y)) {
                if (x >= getButtonPosition(i - currentListPos).x
                    && x < getButtonPosition(i - currentListPos).x + BUILDERBTN_WIDTH
                    && y >= getButtonPosition(i - currentListPos).y
                    && y < getButtonPosition(i - currentListPos).y + BUILDERBTN_HEIGHT) {

                    return i;
                }
            }
        }
    }

    return -1;
}

ItemID_enum BuilderList::getItemIDFromIndex(int i) const {
    auto* const currentGame = dune::globals::currentGame.get();

    if (i >= 0) {
        const auto* const pBuilder = currentGame->getObjectManager().getObject<BuilderBase>(builderObjectID);

        if (pBuilder != nullptr) {
            const auto buildItemIter = std::next(pBuilder->getBuildList().begin(), i);
            if (buildItemIter != pBuilder->getBuildList().end()) {
                return buildItemIter->itemID;
            }
        }
    }

    return ItemID_Invalid;
}
