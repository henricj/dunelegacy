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

#ifndef CONSTRUCTIONYARD_H
#define CONSTRUCTIONYARD_H

#include <structures/BuilderBase.h>

class ConstructionYard final : public BuilderBase
{
public:
    explicit ConstructionYard(House* newOwner);
    explicit ConstructionYard(InputStream& stream);
    void init();
    virtual ~ConstructionYard();

    /**
        Places the just produced structure at x,y.
        \param  x           the x coordinate (in tile coordinates)
        \param  y           the y coordinate (in tile coordinates)
        \return true if placement was successful, false otherwise
    */
    bool doPlaceStructure(int x, int y);
};


#endif // CONSTRUCTIONYARD_H
