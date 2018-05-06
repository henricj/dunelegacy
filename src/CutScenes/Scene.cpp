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

#include <CutScenes/Scene.h>

#include <globals.h>

#include <misc/SDL2pp.h>

Scene::Scene()
{
    currentFrameNumber = 0;
}

Scene::~Scene()
{
    while(videoEvents.empty() == false) {
        VideoEvent* pVideoEvent = videoEvents.front();
        videoEvents.pop();
        delete pVideoEvent;
    }

    for(TextEvent* pTextEvent : textEvents) {
        delete pTextEvent;
    }
    textEvents.clear();

    for(CutSceneTrigger* pCutSceneTrigger : triggerList) {
        delete pCutSceneTrigger;
    }
    triggerList.clear();

}

void Scene::addVideoEvent(VideoEvent* newVideoEvent)
{
    videoEvents.push(newVideoEvent);
}

void Scene::addTextEvent(TextEvent* newTextEvent)
{
    textEvents.push_back(newTextEvent);
}

void Scene::addTrigger(CutSceneTrigger* newTrigger)
{
    std::list<CutSceneTrigger*>::iterator iter = triggerList.begin();

    while(iter != triggerList.end()) {
        if((*iter)->getTriggerFrameNumber() > newTrigger->getTriggerFrameNumber()) {
            break;
        }

        ++iter;
    }

    triggerList.insert(iter, newTrigger);
}

int Scene::draw()
{
    int nextFrameTime = 0;

    // 1.: Clear the whole screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 2.: Draw everything on the screen
    while(videoEvents.empty() == false) {
        VideoEvent* pVideoEvent = videoEvents.front();

        if(pVideoEvent->isFinished() == true) {
            delete pVideoEvent;
            videoEvents.pop();
            continue;
        } else {
            nextFrameTime = pVideoEvent->draw();
            break;
        }
    }

    for(TextEvent* pTextEvent : textEvents) {
        pTextEvent->draw(currentFrameNumber);
    }

    // 3.: Render everything
    SDL_RenderPresent(renderer);

    // 4.: Process Triggers
    while(triggerList.empty() == false) {
        CutSceneTrigger* pTrigger = triggerList.front();

        if(pTrigger->getTriggerFrameNumber() > currentFrameNumber) {
            break;
        }

        if(pTrigger->getTriggerFrameNumber() == currentFrameNumber) {
            pTrigger->trigger(currentFrameNumber);
        }

        triggerList.pop_front();
        delete pTrigger;
    }

    currentFrameNumber++;

    return nextFrameTime;
}
