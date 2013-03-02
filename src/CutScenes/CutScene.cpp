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

#include <CutScenes/CutScene.h>

#include <FileClasses/Palfile.h>
#include <FileClasses/FileManager.h>

#include <globals.h>
#include <sand.h>

#include <SDL.h>

CutScene::CutScene()
{
    quiting = false;

    oldPalette = Palette(screen->format->palette);

    palette = LoadPalette_RW(pFileManager->openFile("INTRO.PAL"), true);

	// Set index 255 to black to avoid flickering
	palette[palette.getNumColors()-1].r = 0;
	palette[palette.getNumColors()-1].g = 0;
	palette[palette.getNumColors()-1].b = 0;

	palette.applyToSurface(screen);
}

CutScene::~CutScene()
{
    // Fixes some flickering
    SDL_FillRect(screen, NULL, 0);
    SDL_Flip(screen);

    while(scenes.empty() == false) {
        Scene* pScene = scenes.front();
        scenes.pop();
        delete pScene;
    }

    oldPalette.applyToSurface(screen);
    palette = oldPalette;
}

void CutScene::run()
{
    SDL_Event event;

	while (!quiting)
	{
	    int frameStart = SDL_GetTicks();

		int nextFrameTime = draw();

		while(SDL_PollEvent(&event)) {

		    //check the events
            switch (event.type)
            {
                case (SDL_KEYDOWN):	// Look for a keypress
                {
                    // Fixes some flickering
                    SDL_FillRect(screen, NULL, 0);
                    SDL_Flip(screen);
                    quiting = true;
                }
            }
		}

		int frameTime = SDL_GetTicks() - frameStart;
        if(frameTime < nextFrameTime) {
            SDL_Delay(nextFrameTime - frameTime);
        }
	}
}

void CutScene::startNewScene() {
    scenes.push(new Scene());
}

void CutScene::addVideoEvent(VideoEvent* newVideoEvent)
{
    if(scenes.empty()) {
        scenes.push(new Scene());
    }

    scenes.back()->addVideoEvent(newVideoEvent);
}

void CutScene::addTextEvent(TextEvent* newTextEvent)
{
    if(scenes.empty()) {
        scenes.push(new Scene());
    }

    scenes.back()->addTextEvent(newTextEvent);
}

void CutScene::addTrigger(CutSceneTrigger* newTrigger)
{
    if(scenes.empty()) {
        scenes.push(new Scene());
    }

    scenes.back()->addTrigger(newTrigger);
}

int CutScene::draw()
{
    int nextFrameTime = 0;

    while(scenes.empty() == false) {

        Scene* pScene = scenes.front();

        if(pScene->isFinished() == true) {
            delete pScene;
            scenes.pop();
            continue;
        } else {
            nextFrameTime = pScene->draw(screen);
            break;
        }
    }

    if(scenes.empty() == true) {
        quit();
    }

    return nextFrameTime;
}
