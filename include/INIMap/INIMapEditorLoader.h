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

#ifndef INIMAPEDITORLOADER_H
#define INIMAPEDITORLOADER_H

#include <DataTypes.h>

#include <INIMap/INIMap.h>
#include <misc/SDL2pp.h>

#include <string>
#include <map>


// forward declarations
class MapEditor;

class INIMapEditorLoader : public INIMap {
public:
    INIMapEditorLoader(MapEditor* pMapEditor, INIMap::inifile_ptr pINIFile);
    ~INIMapEditorLoader();

private:
    void load();
    void loadMap();
    void loadHouses();
    void loadChoam();
    void loadUnits();
    void loadStructures();
    void loadReinforcements();
    void loadAITeams();

    HOUSETYPE getHouseID(const std::string& name);

    MapEditor* pMapEditor;
    std::map<std::string, HOUSETYPE> housename2house;
};

#endif //INIMAPEDITORLOADER_H
