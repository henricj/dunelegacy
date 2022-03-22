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
#include <MapEditor/MapInfo.h>
#include <MapEditor/MapMirror.h>
#include <MapEditor/MapEditorInterface.h>
#include <MapEditor/MapEditorOperation.h>

#include <INIMap/INIMapEditorLoader.h>

#include <ScreenBorder.h>
#include <misc/SDL2pp.h>

#include <DataTypes.h>
#include <AITeamInfo.h>

#include <string>
#include <utility>
#include <vector>
#include <stack>

class MapMirror;

class MapEditor {

public:

    class Player {
    public:

        Player(std::string  name, HOUSETYPE house, HOUSETYPE colorOfHouse, bool bActive, bool bAnyHouse, std::string  brain = "Human", int maxunit = 0)
         : name(std::move(name)), house(house), colorOfHouse(colorOfHouse), bActive(bActive), bAnyHouse(bAnyHouse), brain(std::move(brain)), maxunit(maxunit) {
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
          = default;

        explicit EditorMode(bool dummy)
         : mode(EditorMode_TacticalPos) {
        }

        EditorMode(TERRAINTYPE terrainType, int pensize)
         : mode(EditorMode_Terrain), terrainType(terrainType), pensize(pensize) {
        }

        EditorMode(HOUSETYPE house, ItemID_enum itemID, int health)
         : mode(EditorMode_Structure), house(house), itemID(itemID), health(health) {
        }

        EditorMode(HOUSETYPE house, ItemID_enum itemID, int health, ANGLETYPE angle, ATTACKMODE attackmode)
         : mode(EditorMode_Unit), house(house), itemID(itemID), health(health), angle(angle), attackmode(attackmode) {
        }

        enum {
            EditorMode_Selection,
            EditorMode_Terrain,
            EditorMode_Structure,
            EditorMode_Unit,
            EditorMode_TacticalPos
        } mode{EditorMode_Selection};

        TERRAINTYPE     terrainType = Terrain_Sand;
        int             pensize = 0;
        HOUSETYPE       house = HOUSETYPE::HOUSE_HARKONNEN;
        ANGLETYPE       angle       = static_cast<ANGLETYPE>(0);
        ItemID_enum     itemID      = ItemID_enum::ItemID_Invalid;
        int             health = 0;
        ATTACKMODE      attackmode = ATTACKMODE_INVALID;
    };

    class Structure {
    public:
        Structure(int id, HOUSETYPE house, ItemID_enum itemID, int health, Coord position)
         : id(id), house(house), itemID(itemID), health(health), position(position) {

        }

        int             id;
        HOUSETYPE       house;
        ItemID_enum     itemID;
        int             health;
        Coord           position;
    };

    class Unit {
    public:
        Unit(int id, HOUSETYPE house, ItemID_enum itemID, int health, Coord position, ANGLETYPE angle,
             ATTACKMODE attackmode)
         : id(id), house(house), itemID(itemID), health(health), position(position), angle(angle), attackmode(attackmode) {

        }

        int             id;
        HOUSETYPE       house;
        ANGLETYPE       angle;
        ItemID_enum     itemID;
        int             health;
        Coord           position;
        ATTACKMODE      attackmode;
    };

    MapEditor();
    MapEditor(const MapEditor& o) = delete;
    ~MapEditor();

    friend class INIMapEditorLoader; ///< loading INI Maps is done with a INIMapEditorLoader helper object

    void RunEditor();

    void onQuit() {
        bQuitEditor = true;
    }

    [[nodiscard]] const std::filesystem::path& getLastSaveName() const { return lastSaveName; }

    [[nodiscard]] bool hasChangeSinceLastSave() const { return bChangedSinceLastSave; }

    [[nodiscard]] std::string generateMapname() ;

    std::vector<Player>& getPlayers() {
        return players;
    }

    void informPlayersChanged() { pInterface->onHouseChanges(); }

    void setMirrorMode(MirrorMode newMirrorMode);

    const MapMirror* getMapMirror() { return mapMirror.get(); }

    MapInfo& getMapInfo() { return mapInfo; }

    void setMapInfo(const MapInfo& newMapInfo) { mapInfo = newMapInfo; }

    [[nodiscard]] int getMapVersion() const {
        if(mapInfo.mapSeed != INVALID) {
            return 1;
        }             return 2;

       
    }

    void setMap(const MapData& mapdata, const MapInfo& newMapInfo);

    MapData& getMap() {
        return map;
    }

    std::vector<Coord>& getSpiceBlooms() { return spiceBlooms; }

    std::vector<Coord>& getSpecialBlooms() { return specialBlooms; }

    std::vector<Coord>& getSpiceFields() { return spiceFields; }


    std::map<ItemID_enum,int>& getChoam() { return choam; }

    std::vector<ReinforcementInfo>& getReinforcements() { return reinforcements; }

    void setReinforcements(const std::vector<ReinforcementInfo>& newReinforcements) {
        reinforcements = newReinforcements;
    }

    std::vector<AITeamInfo>& getAITeams() { return aiteams; }

    void setAITeams(const std::vector<AITeamInfo>& newAITeams) {
        aiteams = newAITeams;
    }

    std::vector<Structure>& getStructureList() {
        return structures;
    }

    [[nodiscard]] Structure* getStructure(int structureID) {
        const auto it =
            std::find_if(structures.begin(), structures.end(), [=](const auto& s) { return s.id == structureID; });

        return it == structures.end() ? nullptr : &(*it);
    }

    [[nodiscard]] const Structure* getStructure(int structureID) const {
        const auto it =
            std::find_if(structures.begin(), structures.end(), [=](const auto& s) { return s.id == structureID; });

        return it == structures.end() ? nullptr : &(*it);
    }

    [[nodiscard]] std::vector<int> getMirrorStructures(int unitID) const;

    [[nodiscard]] int getSelectedStructureID() const { return selectedStructureID; }

    Structure* getSelectedStructure() { return getStructure(selectedStructureID); }
    [[nodiscard]] const Structure* getSelectedStructure() const { return getStructure(selectedStructureID); }

    std::vector<Unit>& getUnitList() {
        return units;
    }

    [[nodiscard]] const Unit* getUnit(int unitID) const {
        for(const auto& unit : units) {
            if(unit.id == unitID) { return &unit; }
        }

        return nullptr;
    }

    Unit* getUnit(int unitID) {
        for(auto& unit : units) {
            if(unit.id == unitID) { return &unit; }
        }

        return nullptr;
    }


    [[nodiscard]] std::vector<int> getMirrorUnits(int unitID, bool bAddMissingAsInvalid = false) const;

    [[nodiscard]] int getSelectedUnitID() const {
        return selectedUnitID;
    }

    [[nodiscard]] const Unit* getSelectedUnit() const { return getUnit(selectedUnitID); }

    Unit* getSelectedUnit() { return getUnit(selectedUnitID); }

    [[nodiscard]] bool isTileBlocked(int x, int y, bool bSlabIsBlocking, bool bUnitsAreBlocking) const;

    void setEditorMode(const EditorMode& newEditorMode);

    void startOperation();

    void addUndoOperation(std::unique_ptr<MapEditorOperation> op) {
        undoOperationStack.push(std::move(op));
        bChangedSinceLastSave = true;
    }

    void undoLastOperation();

    void redoLastOperation();

    void clearRedoOperations() {
        while(!redoOperationStack.empty()) {
            redoOperationStack.pop();
        }
    }

    void saveMap(const std::filesystem::path& filepath);
    void loadMap(const std::filesystem::path& filepath);

private:
    void performMapEdit(int xpos, int ypos, bool bRepeated);

    void performTerrainChange(int x, int y, TERRAINTYPE terrainType);

    void drawScreen();
    void processInput();
    void drawCursor();
    void drawMap(ScreenBorder* pScreenborder, bool bCompleteMap) const;
    [[nodiscard]] TERRAINTYPE getTerrain(int x, int y) const;
    void saveMapshot();

private:
    std::unique_ptr<MapEditorInterface> pInterface;     ///< This is the whole interface (top bar and side bar)


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
    std::unique_ptr<MapMirror>      mapMirror;

    bool                            bLeftMousePressed;
    int                             lastTerrainEditPosX;
    int                             lastTerrainEditPosY;

    int                             selectedUnitID;
    int                             selectedStructureID;
    Coord                           selectedMapItemCoord;   ///< only used for classic maps



    std::filesystem::path           lastSaveName;
    std::unique_ptr<INIFile>        loadedINIFile;

    std::vector<Player>             players;


    MapData                         map;
    MapInfo                         mapInfo;

    std::vector<Coord>              spiceBlooms;    ///< only for classic maps
    std::vector<Coord>              specialBlooms;  ///< only for classic maps
    std::vector<Coord>              spiceFields;    ///< only for classic maps

    std::map<ItemID_enum, int>      choam;

    std::vector<ReinforcementInfo>  reinforcements;

    std::vector<AITeamInfo>         aiteams;

    std::vector<Unit>               units;
    std::vector<Structure>          structures;


    std::stack<std::unique_ptr<MapEditorOperation> > undoOperationStack;
    std::stack<std::unique_ptr<MapEditorOperation> > redoOperationStack;
};

#endif // MAPEDITOR_H
