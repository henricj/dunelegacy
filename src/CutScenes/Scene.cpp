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

#include "Renderer/DuneRenderer.h"
#include <misc/SDL2pp.h>

Scene::Scene() = default;

Scene::~Scene() = default;

void Scene::addVideoEvent(std::unique_ptr<VideoEvent> newVideoEvent) {
    videoEvents.emplace(std::move(newVideoEvent));
}

void Scene::addTextEvent(std::unique_ptr<TextEvent> newTextEvent) {
    textEvents.emplace_back(std::move(newTextEvent));
}

void Scene::addTrigger(std::unique_ptr<CutSceneTrigger> newTrigger) {
    triggerList.emplace(std::move(newTrigger));
}

bool Scene::isFinished() const {
    if (videoEvents.empty()) {
        return true;
    }
    if (videoEvents.size() == 1) {

        return videoEvents.front()->isFinished();
    }
    return false;
}

int Scene::draw() {
    auto nextFrameTime = 0;

    auto* const renderer = dune::globals::renderer.get();

    // 1.: Clear the whole screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // 2.: Draw everything on the screen
    while (!videoEvents.empty()) {
        if (videoEvents.front()->isFinished()) {
            videoEvents.pop();
            continue;
        }
        nextFrameTime = videoEvents.front()->draw();
        break;
    }

    for (const auto& pTextEvent : textEvents) {
        pTextEvent->draw(currentFrameNumber);
    }

    // 3.: Render everything
    Dune_RenderPresent(renderer);

    // 4.: Process Triggers
    while (!triggerList.empty()) {
        const auto& pTrigger = triggerList.top();

        if (pTrigger->getTriggerFrameNumber() > currentFrameNumber) {
            break;
        }

        if (pTrigger->getTriggerFrameNumber() == currentFrameNumber) {
            pTrigger->trigger(currentFrameNumber);
        }

        triggerList.pop();
    }

    currentFrameNumber++;

    return nextFrameTime;
}
