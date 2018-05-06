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

#ifndef TIMEOUTTRIGGER_H
#define TIMEOUTTRIGGER_H

#include <Trigger/Trigger.h>

#include <vector>

/**
    This trigger is used for ending the game when the game has run for the specified time. What player has won is determined by the Lose-Flags.
*/
class TimeoutTrigger final : public Trigger {
public:

    /**
        Constructor
        \param  triggerCycleNumber  the game cycle this trigger shall be triggered
    */
    explicit TimeoutTrigger(Uint32 triggerCycleNumber);

    /**
        This constructor constructs the trigger from a stream.
        \param  stream  the stream to read from
    */
    explicit TimeoutTrigger(InputStream& stream);

    /// destructor
    ~TimeoutTrigger();

    /**
        This method saves this trigger to a stream.
        \param  stream  the stream to save to
    */
    void save(OutputStream& stream) const override;

    /**
        Trigger this trigger. Shall only be called when getCycleNumber() is equal to the current game cycle
    */
    void trigger() override;
};

#endif // TIMEOUTTRIGGER_H
