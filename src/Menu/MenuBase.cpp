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

#include <Menu/MenuBase.h>

#include <Network/NetworkManager.h>

#include <misc/string_util.h>
#include <misc/FileSystem.h>

#include <globals.h>

#include <sand.h>


MenuBase::MenuBase() : Window(0,0,0,0) {
	bAllowQuiting = true;
	retVal = MENU_QUIT_DEFAULT;
	bClearScreen = true;
	quiting = false;
}

MenuBase::~MenuBase() {
}

void MenuBase::quit(int returnVal) {
	retVal = returnVal;
	quiting = true;
}

int MenuBase::showMenu() {
	SDL_Event	event;
	// valgrind reports errors in SDL_PollEvent if event is not initialized
	memset(&event, 0, sizeof(event));

	quiting = false;

	while(!quiting) {
	    int frameStart = SDL_GetTicks();

	    update();

	    if(pNetworkManager != NULL) {
            pNetworkManager->update();
	    }

	    if(quiting) {
            return retVal;
	    }

		draw(screen);

		while(SDL_PollEvent(&event)) {
		    //check the events
			if(doInput(event) == false) {
				break;
			}
		}

		int frameTime = SDL_GetTicks() - frameStart;
        if(settings.video.frameLimit == true) {
            if(frameTime < 32) {
                SDL_Delay(32 - frameTime);
            }
        }
	}

	return retVal;
}

void MenuBase::draw(SDL_Surface* screen) {
	if(bClearScreen == true) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
	}

	Window::draw(screen);

	drawSpecificStuff();

	Window::drawOverlay(screen);

	drawCursor();

	SDL_RenderPresent(renderer);
}

void MenuBase::drawSpecificStuff() {
}

bool MenuBase::doInput(SDL_Event &event) {
	switch (event.type) {
		case (SDL_KEYDOWN): {
			// Look for a keypress
			switch(event.key.keysym.sym) {

				case SDLK_ESCAPE: {
					if((pChildWindow == NULL) && (bAllowQuiting == true)) {
						quit();
					}
                } break;

				case SDLK_RETURN: {
					if(SDL_GetModState() & KMOD_ALT) {
						SDL_SetWindowFullscreen(window, (SDL_GetWindowFlags(window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP));
					}
                } break;

                case SDLK_p: {
                    if(SDL_GetModState() & KMOD_CTRL) {
                        // fall through to SDLK_PRINT
                    } else {
                        break;  // do not fall through
                    }
                } // fall through

				case SDLK_PRINTSCREEN:
				case SDLK_SYSREQ: {
                    std::string screenshotFilename;
                    int i = 1;
                    do {
                        screenshotFilename = "Screenshot" + stringify(i) + ".bmp";
                        i++;
                    } while(existsFile(screenshotFilename) == true);

                    SDL_SaveBMP(screen, screenshotFilename.c_str());
                } break;

                case SDLK_TAB: {
                    if(SDL_GetModState() & KMOD_ALT) {
                        SDL_MinimizeWindow(window);
                    }
                } break;

				default: {
				} break;
			}
		} break;

		case SDL_MOUSEMOTION: {
			SDL_MouseMotionEvent* mouse = &event.motion;

			drawnMouseX = mouse->x;
			drawnMouseY = mouse->y;
		} break;

		case SDL_QUIT: {
			if((pChildWindow == NULL) && (bAllowQuiting == true)) {
				quit();
			}
        } break;

		default: {
        } break;
	}

	handleInput(event);

	return !quiting;
}
