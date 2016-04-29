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

#ifndef CUTSCENETRIGGER_H
#define CUTSCENETRIGGER_H

/**
    This class is the base class for triggers that are triggered at a certain frame.
*/
class CutSceneTrigger {
public:

    /**
        Constructor
        \param  frameNumber the frame number relative to the start of the current scene where this trigger should be triggered.
    */
    explicit CutSceneTrigger(int frameNumber);

    /// destructor
    virtual ~CutSceneTrigger();

    /**
        This method returns the frame number where this trigger shall be triggered.
        \return the frame number where this trigger shall be triggered
    */
    inline int getTriggerFrameNumber() const {
        return frameNumber;
    }

    /**
        Trigger this trigger. This method is only called if currentFrameNumber == getTriggerFrameNumber()
        \param  currentFrameNumber  the current frame number relative to the beginning of the current scene
    */
    virtual void trigger(int currentFrameNumber) = 0;

private:
    int frameNumber;    ///< The frame number where this trigger shall be triggered.
};

#endif // CUTSCENETRIGGER_H
