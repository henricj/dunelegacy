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

#ifndef STARPORT_H
#define STARPORT_H

#include <structures/BuilderBase.h>

class StarPort : public BuilderBase
{
public:
    StarPort(House* newOwner);
    StarPort(InputStream& stream);
    void init();
    virtual ~StarPort();

    virtual void save(OutputStream& stream) const;

    virtual void handleProduceItemClick(Uint32 itemID, bool multipleMode = false);

    virtual void handlePlaceOrderClick();
    virtual void handleCancelOrderClick();


    /**
        Start production of the specified item.
        \param  itemID          the item to produce
        \param  multipleMode    false = 1 item, true = 5 items
    */
    virtual void doProduceItem(Uint32 itemID, bool multipleMode = false);

    /**
        Cancel ordering of the specified item.
        \param  itemID          the item to cancel
        \param  multipleMode    false = 1 item, true = 5 items
    */
    virtual void doCancelItem(Uint32 itemID, bool multipleMode = false);

    /**
        Send order and wait for delivery.
    */
    void doPlaceOrder();

    /**
        Cancel the whole order
    */
    void doCancelOrder();

    /**
        Start adding a random item to the order list.
    */
    virtual void doBuildRandom();


    void updateBuildList();

    /**
        Begin with the deploying of the delivered units.
    */
    void startDeploying() {
        deploying = true;
        firstAnimFrame = 8;
        lastAnimFrame = 9;
    };

    /**
      Inform the starport that the frigate was destroyed and new orders can be given.
    */
    void informFrigateDestroyed();

    inline bool okToOrder() const { return (arrivalTimer < 0); }
    inline int getArrivalTimer() const { return arrivalTimer; }

protected:
    /**
        Used for updating things that are specific to that particular structure. Is called from
        StructureBase::update() before the check if this structure is still alive.
    */
    virtual void updateStructureSpecificStuff();

private:
    Sint32  arrivalTimer;       ///< When will the frigate arrive?
    bool    deploying;          ///< Currently deploying units
};

#endif // STARPORT_H
