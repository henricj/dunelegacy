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

#include <MapEditor/MapEditor.h>

#include <MapEditor/MapMirror.h>

#include <GUI/Window.h>
#include <Menu/MenuBase.h>

#include <misc/Fullscreen.h>

#include <ScreenBorder.h>
#include <globals.h>

void MapEditor::processInput(MenuBase::event_handler_type handler) {
    SDL_Event event;

    auto* const screenborder = dune::globals::screenborder.get();

    while (SDL_PollEvent(&event)) {

        // first of all update mouse
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            const auto& mouse = event.motion;

            const auto& video = dune::globals::settings.video;

            dune::globals::drawnMouseX = std::max(0, std::min(static_cast<int>(mouse.x), video.width - 1));
            dune::globals::drawnMouseY = std::max(0, std::min(static_cast<int>(mouse.y), video.height - 1));
        }

        if (pInterface_->hasChildWindow()) {
            pInterface_->handleInput(event);
        } else {
            switch (event.type) {
                case SDL_EVENT_KEY_DOWN: {
                    switch (event.key.key) {

                        case SDLK_RETURN: {
                            if (SDL_GetModState() & SDL_KMOD_ALT) {
                                toggleFullscreen();
                            }
                        } break;

                        case SDLK_TAB: {
                            if (SDL_GetModState() & SDL_KMOD_ALT) {
                                SDL_MinimizeWindow(dune::globals::window.get());
                            }
                        } break;

                        case SDLK_F1: {
                            Coord oldCenterCoord            = screenborder->getCurrentCenter();
                            dune::globals::currentZoomlevel = 0;
                            screenborder->adjustScreenBorderToMapsize(map_.getSizeX(), map_.getSizeY());
                            screenborder->setNewScreenCenter(oldCenterCoord);
                        } break;

                        case SDLK_F2: {
                            Coord oldCenterCoord            = screenborder->getCurrentCenter();
                            dune::globals::currentZoomlevel = 1;
                            screenborder->adjustScreenBorderToMapsize(map_.getSizeX(), map_.getSizeY());
                            screenborder->setNewScreenCenter(oldCenterCoord);
                        } break;

                        case SDLK_F3: {
                            Coord oldCenterCoord            = screenborder->getCurrentCenter();
                            dune::globals::currentZoomlevel = 2;
                            screenborder->adjustScreenBorderToMapsize(map_.getSizeX(), map_.getSizeY());
                            screenborder->setNewScreenCenter(oldCenterCoord);
                        } break;

                        case SDLK_P: {
                            if (SDL_GetModState() & SDL_KMOD_CTRL) {
                                saveMapshot();
                            }
                        } break;

                        case SDLK_PRINTSCREEN:
                        case SDLK_SYSREQ: {
                            saveMapshot();
                        } break;

                        case SDLK_Z: {
                            if (SDL_GetModState() & SDL_KMOD_CTRL) {
                                pInterface_->onUndo();
                            }
                        } break;

                        case SDLK_Y: {
                            if (SDL_GetModState() & SDL_KMOD_CTRL) {
                                pInterface_->onRedo();
                            }
                        } break;

                        default: break;
                    }

                } break;

                case SDL_EVENT_KEY_UP: {
                    switch (event.key.key) {
                        case SDLK_ESCAPE: {
                            // quitting
                            pInterface_->onQuit();
                        } break;

                        case SDLK_DELETE:
                        case SDLK_BACKSPACE: {

                            // check units first
                            if (selectedUnitID_ != INVALID) {
                                clearRedoOperations();
                                startOperation();

                                std::vector<int> selectedUnits = getMirrorUnits(selectedUnitID_);

                                for (const int selectedUnit : selectedUnits) {
                                    MapEditorRemoveUnitOperation removeOperation(selectedUnit);
                                    addUndoOperation(removeOperation.perform(this));
                                }
                                selectedUnitID_ = INVALID;

                                pInterface_->deselectAll();
                            } else if (selectedStructureID_ != INVALID) {
                                // We only try deleting structures_ if we had not yet deleted a unit (e.g. a unit on
                                // concrete)
                                clearRedoOperations();
                                startOperation();

                                std::vector<int> selectedStructures = getMirrorStructures(selectedStructureID_);

                                for (const int selectedStructure : selectedStructures) {
                                    MapEditorRemoveStructureOperation removeOperation(selectedStructure);
                                    addUndoOperation(removeOperation.perform(this));
                                }
                                selectedStructureID_ = INVALID;

                                pInterface_->deselectAll();
                            } else if (selectedMapItemCoord_.isValid()) {
                                auto iter = std::ranges::find(specialBlooms_, selectedMapItemCoord_);

                                if (iter != specialBlooms_.end()) {
                                    clearRedoOperations();
                                    startOperation();
                                    MapEditorTerrainRemoveSpecialBloomOperation removeOperation(iter->x, iter->y);
                                    addUndoOperation(removeOperation.perform(this));

                                    selectedMapItemCoord_.invalidate();
                                } else {
                                    iter = std::ranges::find(spiceBlooms_, selectedMapItemCoord_);

                                    if (iter != spiceBlooms_.end()) {
                                        clearRedoOperations();
                                        startOperation();
                                        MapEditorTerrainRemoveSpiceBloomOperation removeOperation(iter->x, iter->y);
                                        addUndoOperation(removeOperation.perform(this));

                                        selectedMapItemCoord_.invalidate();
                                    } else {
                                        iter = std::ranges::find(spiceFields_, selectedMapItemCoord_);

                                        if (iter != spiceFields_.end()) {
                                            clearRedoOperations();
                                            startOperation();
                                            MapEditorTerrainRemoveSpiceFieldOperation removeOperation(iter->x, iter->y);
                                            addUndoOperation(removeOperation.perform(this));

                                            selectedMapItemCoord_.invalidate();
                                        }
                                    }
                                }
                            }

                        } break;

                        default: break;
                    }
                } break;

                case SDL_EVENT_MOUSE_MOTION: {
                    pInterface_->handleMouseMovement(dune::globals::drawnMouseX, dune::globals::drawnMouseY);

                    if (bLeftMousePressed_) {
                        if (screenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX,
                                                                 dune::globals::drawnMouseY)) {
                            // if mouse is not over side bar

                            const int xpos = screenborder->screen2MapX(dune::globals::drawnMouseX);
                            const int ypos = screenborder->screen2MapY(dune::globals::drawnMouseY);

                            if ((xpos != lastTerrainEditPosX_) || (ypos != lastTerrainEditPosY_)) {
                                performMapEdit(xpos, ypos, true);
                            }
                        }
                    }
                } break;

                case SDL_EVENT_MOUSE_WHEEL: {
                    if (event.wheel.y != 0) {
                        if (screenborder->isScreenCoordInsideMap(dune::globals::drawnMouseX,
                                                                 dune::globals::drawnMouseY)) {
                            // if mouse is not over side bar
                            const int xpos = screenborder->screen2MapX(dune::globals::drawnMouseX);
                            const int ypos = screenborder->screen2MapY(dune::globals::drawnMouseY);

                            for (const Unit& unit : units_) {
                                const Coord position = unit.position_;
                                if ((position.x == xpos) && (position.y == ypos)) {
                                    if (event.wheel.y > 0) {
                                        pInterface_->onUnitRotateLeft(unit.id_);
                                    } else {
                                        pInterface_->onUnitRotateRight(unit.id_);
                                    }
                                    break;
                                }
                            }
                        }

                        pInterface_->handleMouseWheel(
                            dune::globals::drawnMouseX, dune::globals::drawnMouseY, (event.wheel.y > 0));
                    }
                } break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                    const SDL_MouseButtonEvent* mouse = &event.button;

                    switch (mouse->button) {
                        case SDL_BUTTON_LEFT: {
                            if (!pInterface_->handleMouseLeft(
                                    static_cast<int>(mouse->x), static_cast<int>(mouse->y), true)) {

                                bLeftMousePressed_ = true;

                                if (screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                    // if mouse is not over side bar

                                    const int xpos = screenborder->screen2MapX(mouse->x);
                                    const int ypos = screenborder->screen2MapY(mouse->y);

                                    performMapEdit(xpos, ypos, false);
                                }
                            }

                        } break;

                        case SDL_BUTTON_RIGHT: {
                            if (!pInterface_->handleMouseRight(
                                    static_cast<int>(mouse->x), static_cast<int>(mouse->y), true)) {

                                if (screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                    // if mouse is not over side bar
                                    setEditorMode(EditorMode());
                                    pInterface_->deselectAll();
                                }
                            }
                        } break;
                    }
                } break;

                case SDL_EVENT_MOUSE_BUTTON_UP: {
                    const SDL_MouseButtonEvent* mouse = &event.button;

                    switch (mouse->button) {
                        case SDL_BUTTON_LEFT: {

                            pInterface_->handleMouseLeft(static_cast<int>(mouse->x), static_cast<int>(mouse->y), false);

                            if (bLeftMousePressed_) {

                                bLeftMousePressed_   = false;
                                lastTerrainEditPosX_ = -1;
                                lastTerrainEditPosY_ = -1;

                                if (currentEditorMode_.mode_ == EditorMode::EditorMode_Selection) {
                                    if (screenborder->isScreenCoordInsideMap(mouse->x, mouse->y)) {
                                        // if mouse is not over side bar

                                        const int xpos = screenborder->screen2MapX(mouse->x);
                                        const int ypos = screenborder->screen2MapY(mouse->y);

                                        selectedUnitID_      = INVALID;
                                        selectedStructureID_ = INVALID;
                                        selectedMapItemCoord_.invalidate();

                                        bool bUnitSelected = false;

                                        for (const Unit& unit : units_) {
                                            const Coord position = unit.position_;

                                            if ((position.x == xpos) && (position.y == ypos)) {
                                                selectedUnitID_ = unit.id_;
                                                bUnitSelected   = true;
                                                pInterface_->onObjectSelected();
                                                mapInfo_.cursorPos = position;
                                                break;
                                            }
                                        }

                                        bool bStructureSelected = false;

                                        for (const Structure& structure : structures_) {
                                            const Coord& position     = structure.position_;
                                            const Coord structureSize = getStructureSize(structure.itemID_);

                                            if (!bUnitSelected && (xpos >= position.x)
                                                && (xpos < position.x + structureSize.x) && (ypos >= position.y)
                                                && (ypos < position.y + structureSize.y)) {
                                                selectedStructureID_ = structure.id_;
                                                bStructureSelected   = true;
                                                pInterface_->onObjectSelected();
                                                mapInfo_.cursorPos = position;
                                                break;
                                            }
                                        }

                                        if (!bUnitSelected && !bStructureSelected) {
                                            pInterface_->deselectAll();

                                            // find map items (spice bloom, special bloom or spice field)
                                            if ((std::ranges::find(spiceBlooms_, Coord(xpos, ypos))
                                                 != spiceBlooms_.end())
                                                || (std::ranges::find(specialBlooms_, Coord(xpos, ypos))
                                                    != specialBlooms_.end())
                                                || (std::ranges::find(spiceFields_, Coord(xpos, ypos))
                                                    != spiceFields_.end())) {
                                                selectedMapItemCoord_ = Coord(xpos, ypos);
                                            }
                                        }
                                    }
                                }
                            }
                        } break;

                        case SDL_BUTTON_RIGHT: {
                            pInterface_->handleMouseRight(
                                static_cast<int>(mouse->x), static_cast<int>(mouse->y), false);
                        } break;
                    }
                } break;

                case SDL_EVENT_QUIT: {
                    bQuitEditor_ = true;
                } break;
            }
        }

        if (handler && Window::isBroadcastEvent(event))
            handler(event);
    }

    if ((!pInterface_->hasChildWindow())
        && (SDL_GetWindowFlags(dune::globals::window.get()) & SDL_WINDOW_INPUT_FOCUS)) {
        const auto* const keystate = SDL_GetKeyboardState(nullptr);
        scrollDownMode_ =
            (dune::globals::drawnMouseY >= getRendererHeight() - 1 - SCROLLBORDER) || keystate[SDL_SCANCODE_DOWN];
        scrollLeftMode_ = (dune::globals::drawnMouseX <= SCROLLBORDER) || keystate[SDL_SCANCODE_LEFT];
        scrollRightMode_ =
            (dune::globals::drawnMouseX >= getRendererWidth() - 1 - SCROLLBORDER) || keystate[SDL_SCANCODE_RIGHT];
        scrollUpMode_ = (dune::globals::drawnMouseY <= SCROLLBORDER) || keystate[SDL_SCANCODE_UP];

        if (scrollLeftMode_ && scrollRightMode_) {
            // do nothing
        } else if (scrollLeftMode_) {
            scrollLeftMode_ = screenborder->scrollLeft();
        } else if (scrollRightMode_) {
            scrollRightMode_ = screenborder->scrollRight();
        }

        if (scrollDownMode_ && scrollUpMode_) {
            // do nothing
        } else if (scrollDownMode_) {
            scrollDownMode_ = screenborder->scrollDown();
        } else if (scrollUpMode_) {
            scrollUpMode_ = screenborder->scrollUp();
        }
    } else {
        scrollDownMode_  = false;
        scrollLeftMode_  = false;
        scrollRightMode_ = false;
        scrollUpMode_    = false;
    }
}
