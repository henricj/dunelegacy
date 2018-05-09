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

#ifndef SCENE_H
#define SCENE_H

#include <queue>
#include <list>
#include <CutScenes/VideoEvent.h>
#include <CutScenes/TextEvent.h>
#include <CutScenes/CutSceneTrigger.h>

/// A class for representing one part of a cutscene.
/**
    Every CutScene consists of multiple Scene objects. This makes debugging the timings easier because all timings are relative to the scene start.
*/
class Scene {
public:

    /// Default constructor
    Scene();

    /// Destructor
    virtual ~Scene();

    /**
        This method adds a new video event at the end of this scene.
        IMPORTANT: The video event has to be created by new and will be automatically destroyed when this
        Scene gets destroyed.
        \param newVideoEvent the new video event to be played at the end of this scene (must be created with new)
    */
    void addVideoEvent(std::unique_ptr<VideoEvent> newVideoEvent);

    /**
        This method adds a new text event to this scene.
        IMPORTANT: The text event has to be created by new and will be automatically destroyed when this
        Scene gets destroyed.
        \param newTextEvent the new text event to be played in this scene (must be created with new)
    */
    void addTextEvent(std::unique_ptr<TextEvent> newTextEvent);

    /**
        This method adds a new trigger to this scene.
        IMPORTANT: The trigger event has to be created by new and will be automatically destroyed when this
        Scene gets destroyed.
        \param newTrigger the new trigger to be triggered in this scene (must be created with new)
    */
    void addTrigger(std::unique_ptr<CutSceneTrigger> newTrigger);

    /**
        This method checks if there is something to draw in the next frame
        \return true, if there are no more VideoEvents in the queue
    */
    bool isFinished() {
        if(videoEvents.empty()) {
            return true;
        } else if(videoEvents.size() == 1) {
            return videoEvents.front()->isFinished();
        } else {
            return false;
        }
    }

    /**
        This method draws the current frame. First the video and then the text is drawn. Afterwards CutSceneTriggers are triggered.
        \return the milliseconds until the next frame shall be drawn.
    */
    int draw();

private:
    int currentFrameNumber;                     ///< current frame number in this frame

    std::queue< std::unique_ptr<VideoEvent> > videoEvents;        ///< queue of all VideoEvents in this scene
    std::list< std::unique_ptr<TextEvent> > textEvents;           ///< list of all TextEvents in this scene
    std::list< std::unique_ptr<CutSceneTrigger> > triggerList;    ///< list of all CutSceneTriggers in this scene
};

#endif // SCENE_H
