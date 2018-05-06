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

#ifndef TRIGGERMANAGER_H
#define TRIGGERMANAGER_H

#include <Trigger/Trigger.h>
#include <misc/InputStream.h>
#include <misc/OutputStream.h>

#include <memory>
#include <list>

/**
    This class manages triggers for the game play. A trigger is triggered at a specific game cycle.
*/
class TriggerManager {
public:

    /// default constructor
    TriggerManager();

    /// destructor
    virtual ~TriggerManager();

    /**
        Save all triggers to stream
        \param  stream  the stream to save to
    */
    void save(OutputStream& stream) const;

    /**
        Load triggers from stream.
        \param  stream  the stream to load from
    */
    void load(InputStream& stream);

    /**
        Triggers all triggers at CycleNumber
        \param  CycleNumber the current game cycle
    */
    void trigger(Uint32 CycleNumber);

    /**
        Add a trigger to the trigger manager.
        \param newTrigger   shared pointer to the new trigger
    */
    void addTrigger(std::unique_ptr<Trigger> newTrigger);

    /**
        This method returns a list of all the managed triggers.
        \return a list of all the triggers
    */
    const std::list<std::unique_ptr<Trigger> >& getTriggers() const { return triggers; }

private:
    std::list<std::unique_ptr<Trigger> > triggers;  ///< list of all triggers. sorted by the time when they shall be triggered.

    typedef enum {
        Type_ReinforcementTrigger = 1,      ///< the trigger is of type ReinforcementTrigger
        Type_TimeoutTrigger = 2             ///< the trigger is of type TimeoutTrigger
    } TriggerType;

    /**
        Helper method for saving one trigger.
        \param  stream      the stream to save to
        \param  t           pointer to the trigger to save
    */
    void saveTrigger(OutputStream& stream, const Trigger* t) const;

    /**
        Helper method for loading one trigger
        \param  stream  stream to load from
        \return a shared pointer to the loaded trigger
    */
    std::unique_ptr<Trigger> loadTrigger(InputStream& stream);
};

#endif // TRIGGERMANAGER_H
