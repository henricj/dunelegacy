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

#ifndef MAPEDITOR_H
#define MAPEDITOR_H

#include <data.h>

#include <MapEditor/MapData.h>
#include <MapEditor/ReinforcementInfo.h>
#include <MapEditor/TeamInfo.h>
#include <MapEditor/MapInfo.h>
#include <MapEditor/MapMirror.h>
#include <MapEditor/MapEditorInterface.h>
#include <MapEditor/MapEditorOperation.h>


#include <INIMap/INIMapEditorLoader.h>

#include <ScreenBorder.h>

#include <DataTypes.h>

#include <SDL.h>
#include <string>
#include <vector>
#include <stack>

class MapMirror;

class MapEditor {

public:

    class Player {
    public:

        Player(std::string name, HOUSETYPE house, HOUSETYPE colorOfHouse, bool bActive, bool bAnyHouse, std::string brain = "Human", int maxunit = 0)
         : name(name), house(house), colorOfHouse(colorOfHouse), bActive(bActive), bAnyHouse(bAnyHouse), brain(brain), maxunit(maxunit) {
            quota = 0;
            credits = 2000;
        }

        std::string name;
        HOUSETYPE house;
        HOUSETYPE colorOfHouse;
        bool bActive;
        bool bAnyHouse;
        int quota;
        int credits;
        std::string brain;
        int maxunit;
    };

    class EditorMode {
    public:

        EditorMode()
         : mode(EditorMode_Selection) {
        }

        EditorMode(bool dummy)
         : mode(EditorMode_TacticalPos) {
        }

        EditorMode(TERRAINTYPE terrainType, int pensize)
         : mode(EditorMode_Terrain), terrainType(terrainType), pensize(pensize) {
        }

        EditorMode(HOUSETYPE house, int itemID, int health)
         : mode(EditorMode_Structure), house(house), itemID(itemID), health(health) {
        }

        EditorMode(HOUSETYPE house, int itemID, int health, unsigned char angle, ATTACKMODE attackmode)
         : mode(EditorMode_Unit), house(house), itemID(itemID), health(health), angle(angle), attackmode(attackmode) {
        }

        enum {
            EditorMode_Selection,
            EditorMode_Terrain,
            EditorMode_Structure,
            EditorMode_Unit,
            EditorMode_TacticalPos
        } mode;

        TERRAINTYPE     terrainType;
        int             pensize;
        HOUSETYPE       house;
        int             itemID;
        int             health;
        unsigned char   angle;
        ATTACKMODE      attackmode;
    };

    class Structure {
    public:
        Structure(int id, HOUSETYPE house, int itemID, int health, Coord position)
         : id(id), house(house), itemID(itemID), health(health), position(position) {

        }

        int             id;
        HOUSETYPE       house;
        int             itemID;
        int             health;
        Coord           position;
    };

    class Unit {
    public:
        Unit(int id, HOUSETYPE house, int itemID, int health, Coord position, unsigned char angle, ATTACKMODE attackmode)
         : id(id), house(house), itemID(itemID), health(health), position(position), angle(angle), attackmode(attackmode) {

        }

        int             id;
        HOUSETYPE       house;
        int             itemID;
        int             health;
        Coord           position;
        unsigned char   angle;
        ATTACKMODE      attackmode;
    };

    MapEditor();
    ~MapEditor();

    friend class INIMapEditorLoader; ///< loading INI Maps is done with a INIMapEditorLoader helper object

    void RunEditor();

    void onQuit() {
        bQuitEditor = true;
    }

    const std::string& getLastSaveName() const { return lastSaveName; };

    bool hasChangeSinceLastSave() const { return bChangedSinceLastSave; };

    std::string generateMapname() const;

    std::vector<Player>& getPlayers() {
        return players;
    }

    void informPlayersChanged() { pInterface->onHouseChanges(); };

    void setMirrorMode(MirrorMode newMirrorMode);

    std::shared_ptr<MapMirror> getMapMirror() { return mapMirror; }

    MapInfo& getMapInfo() { return mapInfo; };

    void setMapInfo(MapInfo newMapInfo) { mapInfo = newMapInfo; };

    int getMapVersion() const {
        if(mapInfo.mapSeed != INVALID) {
            return 1;
        } else {
            return 2;
        }
    }

    void setMap(const MapData& mapdata, const MapInfo& newMapInfo);

    MapData& getMap() {
        return map;
    }

    std::vector<Coord>& getSpiceBlooms() { return spiceBlooms; };

    std::vector<Coord>& getSpecialBlooms() { return specialBlooms; };

    std::vector<Coord>& getSpiceFields() { return spiceFields; };


    std::map<int,int>& getChoam() { return choam; };

    std::vector<ReinforcementInfo>& getReinforcements() { return reinforcements; };

    void setReinforcements(std::vector<ReinforcementInfo>& newReinforcements) {
        reinforcements = newReinforcements;
    }

    std::vector<TeamInfo>& getTeams() { return teams; };

    void setTeams(std::vector<TeamInfo>& newTeams) {
        teams = newTeams;
    }

    std::vector<Structure>& getStructureList() {
        return structures;
    }

    Structure* getStructure(int structureID) {
        std::vector<Structure>::iterator iter;
        for(iter = structures.begin(); iter != structures.end(); ++iter) {
            if(iter->id == structureID) {
                return &(*iter);
            }
        }

        return nullptr;
    }

    std::vector<int> getMirrorStructures(int unitID);

    int getSelectedStructureID() const {
        return selectedStructureID;
    }

    Structure* getSelectedStructure() {
        return getStructure(selectedStructureID);
    }

    std::vector<Unit>& getUnitList() {
        return units;
    }

    Unit* getUnit(int unitID) {
        std::vector<Unit>::iterator iter;
        for(iter = units.begin(); iter != units.end(); ++iter) {
            if(iter->id == unitID) {
                return &(*iter);
            }
        }

        return nullptr;
    }


    std::vector<int> getMirrorUnits(int unitID, bool bAddMissingAsInvalid = false);

    int getSelectedUnitID() const {
        return selectedUnitID;
    }

    Unit* getSelectedUnit() {
        return getUnit(selectedUnitID);
    }

    bool isTileBlocked(int x, int y, bool bSlabIsBlocking, bool bUnitsAreBlocking) const;

    void setEditorMode(const EditorMode& newEditorMode);

    void startOperation();

    void addUndoOperation(std::shared_ptr<MapEditorOperation> op) {
        undoOperationStack.push(op);
        bChangedSinceLastSave = true;
    }

    void undoLastOperation();

    void redoLastOperation();

    inline void clearRedoOperations() {
        while(!redoOperationStack.empty()) {
            redoOperationStack.pop();
        }
    }

    void saveMap(const std::string& filepath);
    void loadMap(const std::string& filepath);

private:
    void performMapEdit(int xpos, int ypos, bool bRepeated);

    void performTerrainChange(int x, int y, TERRAINTYPE terrainType);

    void drawScreen();
    void processInput();
    void drawCursor();
    void drawMap(ScreenBorder* pScreenborder, bool bCompleteMap);
    TERRAINTYPE getTerrain(int x, int y);
    void saveMapshot();

private:
    MapEditorInterface*             pInterface;     ///< This is the whole interface (top bar and side bar)


    SDL_Rect                        sideBarPos;
    SDL_Rect                        topBarPos;
    SDL_Rect                        bottomBarPos;

    bool                            bQuitEditor;

    bool                            scrollDownMode;     ///< currently scrolling the map down?
    bool                            scrollLeftMode;     ///< currently scrolling the map left?
    bool                            scrollRightMode;    ///< currently scrolling the map right?
    bool                            scrollUpMode;       ///< currently scrolling the map up?

    bool                            shift;

    bool                            bChangedSinceLastSave;

    EditorMode                      currentEditorMode;

    MirrorMode                      currentMirrorMode;
    std::shared_ptr<MapMirror>      mapMirror;

    bool                            bLeftMousePressed;
    int                             lastTerrainEditPosX;
    int                             lastTerrainEditPosY;

    int                             selectedUnitID;
    int                             selectedStructureID;
    Coord                           selectedMapItemCoord;   ///< only used for classic maps



    std::string                     lastSaveName;
    std::shared_ptr<INIFile>        loadedINIFile;

    std::vector<Player>             players;


    MapData                         map;
    MapInfo                         mapInfo;

    std::vector<Coord>              spiceBlooms;    ///< only for classic maps
    std::vector<Coord>              specialBlooms;  ///< only for classic maps
    std::vector<Coord>              spiceFields;    ///< only for classic maps

    std::map<int,int>               choam;

    std::vector<ReinforcementInfo>  reinforcements;

    std::vector<TeamInfo>           teams;

    std::vector<Unit>               units;
    std::vector<Structure>          structures;


    std::stack<std::shared_ptr<MapEditorOperation> > undoOperationStack;
    std::stack<std::shared_ptr<MapEditorOperation> > redoOperationStack;
};

#endif // MAPEDITOR_H
