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

#include "misc/dune_events.h"
#include <FileClasses/FileManager.h>
#include <FileClasses/Palfile.h>
#include <FileClasses/music/MusicPlayer.h>
#include <misc/SDL2pp.h>

#include <globals.h>
#include <sand.h>

#include "misc/dune_timer_resolution.h"

CutScene::CutScene() : quitting(false) { }

CutScene::~CutScene() {
    // Fixes some flickering
    auto* const renderer = dune::globals::renderer.get();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    Dune_RenderPresent(renderer);
}

void CutScene::run() {
    SDL_Event event;

    dune::DuneTimerResolution timer_handle;

    while (!quitting) {
        const auto frameStart = dune::dune_clock::now();

        const auto nextFrameTime = draw();

        const auto frameDone = std::chrono::milliseconds{nextFrameTime} + frameStart;

        for (;;) {
            const auto now = dune::dune_clock::now();

            if (nextFrameTime > 0) {
                if (now >= frameDone) {
                    // We are done with the current frame, but check for any events before continuing to the
                    // next frame.
                    if (!SDL_PollEvent(&event))
                        break;
                } else {
                    // We keep waiting until the frame is done.  We could wake up before the frame is done,
                    // even without an even to process, so we continue to the top of the loop to recompute
                    // the remaining time.
                    const auto remaining =
                        std::chrono::duration_cast<std::chrono::milliseconds>(frameDone - now).count();

                    if (!dune::Dune_WaitEvent(&event, static_cast<uint32_t>(remaining)))
                        continue;
                }
            } else {
                // The current frame claims it is done, but we delay here in case it is polling for something.
                // This keeps this thread from hogging a CPU and keeps the event pump running.
                if (!dune::Dune_WaitEvent(&event, 1))
                    break;
            }

            // check the events
            switch (event.type) {
                case SDL_KEYDOWN: // Look for a keypress
                {
                    if (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_ESCAPE) {
                        // Fixes some flickering
                        auto* const renderer = dune::globals::renderer.get();

                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        SDL_RenderClear(renderer);
                        Dune_RenderPresent(renderer);
                        quitting = true;
                    }
                }
                default: break;
            }
        }
    }
}

void CutScene::startNewScene() {
    scenes.push(std::make_unique<Scene>());
}

void CutScene::addVideoEvent(std::unique_ptr<VideoEvent> newVideoEvent) {
    if (scenes.empty()) {
        scenes.push(std::make_unique<Scene>());
    }

    scenes.back()->addVideoEvent(std::move(newVideoEvent));
}

void CutScene::addTextEvent(std::unique_ptr<TextEvent> newTextEvent) {
    if (scenes.empty()) {
        scenes.push(std::make_unique<Scene>());
    }

    scenes.back()->addTextEvent(std::move(newTextEvent));
}

void CutScene::addTrigger(std::unique_ptr<CutSceneTrigger> newTrigger) {
    if (scenes.empty()) {
        scenes.push(std::make_unique<Scene>());
    }

    scenes.back()->addTrigger(std::move(newTrigger));
}

int CutScene::draw() {
    int nextFrameTime = 0;

    while (!scenes.empty()) {
        if (scenes.front()->isFinished()) {
            scenes.pop();
            continue;
        }
        nextFrameTime = scenes.front()->draw();
        break;
    }

    if (scenes.empty() && !dune::globals::musicPlayer->isMusicPlaying()) {
        quit();
    }

    return nextFrameTime;
}

std::unique_ptr<Wsafile> CutScene::create_wsafile(const char* name1) {
    return std::make_unique<Wsafile>(dune::globals::pFileManager->openFile(name1).get());
}

std::unique_ptr<Wsafile> CutScene::create_wsafile(const char* name1, const char* name2) {
    const auto* const file_manager = dune::globals::pFileManager.get();

    return std::make_unique<Wsafile>(file_manager->openFile(name1).get(), file_manager->openFile(name2).get());
}

std::unique_ptr<Wsafile> CutScene::create_wsafile(const char* name1, const char* name2, const char* name3) {
    const auto* const file_manager = dune::globals::pFileManager.get();

    return std::make_unique<Wsafile>(file_manager->openFile(name1).get(), file_manager->openFile(name2).get(),
                                     file_manager->openFile(name3).get());
}
