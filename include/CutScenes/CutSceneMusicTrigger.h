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

#ifndef CUTSCENEMUSICTRIGGER_H
#define CUTSCENEMUSICTRIGGER_H

#include <FileClasses/music/MusicPlayer.h>


/**
    This class is used for triggering a music change
*/
class CutSceneMusicTrigger : public CutSceneTrigger {
public:

    /**
        Constructor
        \param  frameNumber     the frame number relative to the scene start where the music shall be changed
        \param  musicType       the type of the new music
    */
    CutSceneMusicTrigger(int frameNumber, MUSICTYPE musicType)
     : CutSceneTrigger(frameNumber), musicType(musicType)
    {
    }

    /// destructor
    ~CutSceneMusicTrigger() = default;

    /**
        Trigger this trigger. This method is only called if currentFrameNumber == getTriggerFrameNumber()
        \param  currentFrameNumber  the current frame number relative to the beginning of the current scene
    */
    void trigger(int currentFrameNumber) override
    {
        musicPlayer->changeMusic(musicType);
    }

private:
    MUSICTYPE musicType;    ///< the type of the new music
};

#endif // CUTSCENEMUSICTRIGGER_H
