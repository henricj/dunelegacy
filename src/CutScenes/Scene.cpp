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

Scene::~Scene() = default;

void Scene::addVideoEvent(std::unique_ptr<VideoEvent> newVideoEvent)
{
    videoEvents.push(std::move(newVideoEvent));
}

void Scene::addTextEvent(std::unique_ptr<TextEvent> newTextEvent)
{
    textEvents.push_back(std::move(newTextEvent));
}

void Scene::addTrigger(std::unique_ptr<CutSceneTrigger> newTrigger)
{
    auto iter = triggerList.begin();
    while(iter != triggerList.end()) {
        if((*iter)->getTriggerFrameNumber() > newTrigger->getTriggerFrameNumber()) {
            break;
        }

        ++iter;
    }

    triggerList.insert(iter, std::move(newTrigger));
}

int Scene::draw()
{
    int nextFrameTime = 0;

    // 1.: Clear the whole screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 2.: Draw everything on the screen
    while(videoEvents.empty() == false) {
        if(videoEvents.front()->isFinished() == true) {
            videoEvents.pop();
            continue;
        } else {
            nextFrameTime = videoEvents.front()->draw();
            break;
        }
    }

    for(auto& pTextEvent : textEvents) {
        pTextEvent->draw(currentFrameNumber);
    }

    // 3.: Render everything
    SDL_RenderPresent(renderer);

    // 4.: Process Triggers
    while(triggerList.empty() == false) {
        auto& pTrigger = triggerList.front();

        if(pTrigger->getTriggerFrameNumber() > currentFrameNumber) {
            break;
        }

        if(pTrigger->getTriggerFrameNumber() == currentFrameNumber) {
            pTrigger->trigger(currentFrameNumber);
        }

        triggerList.pop_front();
    }

    currentFrameNumber++;

    return nextFrameTime;
}
