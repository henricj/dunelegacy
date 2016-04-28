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

#ifndef CUTSCENE_H
#define CUTSCENE_H

#include <CutScenes/Scene.h>
#include <FileClasses/Palette.h>

#include <SDL.h>
#include <queue>

/// A base class for running Dune 2 Cutscenes.
/**
    This class runs a Dune 2 Cutscene. The cutscene is composed of video elements with aditional audio and text elements. The video
    is managed by subclasses of VideoEvent. The list if VideoEvents determines the length of the video. TextEvents and CutSceneTriggers
    (CutSceneMusicTrigger or CutSceneSoundTrigger) can be linked in at specific framenumbers. The whole video is split up in scenes to
    make it easier to debug the exact timings of TextEvents and CutSceneTriggers. The timings for both are given relative to the start of
    the last scene.
*/
class CutScene
{
public:

    /// Default constructor
    CutScene();

    /// Destructor
    virtual ~CutScene();

    /**
        This method runs the cutscene. This method returns when either the user cancels the cutscene with ESC or when the cutscene is finished.
    */
    void run();

    /**
        Quit this cutscene before the next scene draw.
    */
    void quit() { quiting = true; };

    /**
        This method starts a new scene.
    */
    void startNewScene();

    /**
        This method adds a new video event at the end of this cutscene (to the last started scene)
        IMPORTANT: The video event has to be created by new and will be automatically destroyed when this
        CutScene gets destroyed.
        \param newVideoEvent the new video event to be played at the end of this scene (must be created with new)
    */
    void addVideoEvent(VideoEvent* newVideoEvent);

    /**
        This method adds a new text event to the current this scene.
        IMPORTANT: The text event has to be created by new and will be automatically destroyed when this
        Scene gets destroyed.
        \param newTextEvent the new text event to be played in the current scene (must be created with new)
    */
    void addTextEvent(TextEvent* newTextEvent);

    /**
        This method adds a new trigger to the current this scene.
        IMPORTANT: The trigger event has to be created by new and will be automatically destroyed when this
        Scene gets destroyed.
        \param newTrigger the new trigger to be triggered in the current scene (must be created with new)
    */
    void addTrigger(CutSceneTrigger* newTrigger);

protected:
    /**
        This method draws the current frame. The drawing is deligated to Scene::draw() of the first scene in the scenes queue.
        If the first scene is finished it is dropped from the queue and the next scene is used. If the scenes queue is empty
        the whole cutscene is finished.
        \return the milliseconds until the next frame shall be drawn.
    */
    int draw();

private:
    std::queue<Scene*> scenes;      ///< List of all scenes
    bool quiting;                   ///< Quit the cutscene?
};

#endif // CUTSCENE_H
