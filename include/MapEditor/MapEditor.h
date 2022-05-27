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
#include <MapEditor/MapEditorInterface.h>
#include <MapEditor/MapEditorOperation.h>
#include <MapEditor/MapInfo.h>
#include <MapEditor/MapMirror.h>
#include <MapEditor/ReinforcementInfo.h>

#include <INIMap/INIMapEditorLoader.h>

#include <misc/SDL2pp.h>

#include <AITeamInfo.h>
#include <DataTypes.h>

#include <stack>
#include <string>
#include <utility>
#include <vector>

class MapMirror;

class MapEditor {

public:
    class Player {
    public:
        Player(std::string name, HOUSETYPE house, HOUSETYPE colorOfHouse, bool bActive, bool bAnyHouse,
               std::string brain = "Human", int maxunit = 0)
            : name_(std::move(name)), house_(house), colorOfHouse_(colorOfHouse), bActive_(bActive),
              bAnyHouse_(bAnyHouse), brain_(std::move(brain)), maxunit_(maxunit) { }

        std::string name_;
        HOUSETYPE house_;
        HOUSETYPE colorOfHouse_;
        bool bActive_;
        bool bAnyHouse_;
        int quota_   = 0;
        int credits_ = 2000;
        std::string brain_;
        int maxunit_;
    };

    class EditorMode {
    public:
        EditorMode() = default;

        explicit EditorMode([[maybe_unused]] bool dummy) : mode_(EditorMode_TacticalPos) { }

        EditorMode(TERRAINTYPE terrainType, int pensize)
            : mode_(EditorMode_Terrain), terrainType_(terrainType), pen_size_(pensize) { }

        EditorMode(HOUSETYPE house, ItemID_enum itemID, int health)
            : mode_(EditorMode_Structure), house_(house), itemID_(itemID), health_(health) { }

        EditorMode(HOUSETYPE house, ItemID_enum itemID, int health, ANGLETYPE angle, ATTACKMODE attackmode)
            : mode_(EditorMode_Unit), house_(house), angle_(angle), itemID_(itemID), health_(health),
              attackmode_(attackmode) { }

        enum {
            EditorMode_Selection,
            EditorMode_Terrain,
            EditorMode_Structure,
            EditorMode_Unit,
            EditorMode_TacticalPos
        } mode_{EditorMode_Selection};

        TERRAINTYPE terrainType_ = Terrain_Sand;
        int pen_size_            = 0;
        HOUSETYPE house_         = HOUSETYPE::HOUSE_HARKONNEN;
        ANGLETYPE angle_         = static_cast<ANGLETYPE>(0);
        ItemID_enum itemID_      = ItemID_enum::ItemID_Invalid;
        int health_              = 0;
        ATTACKMODE attackmode_   = ATTACKMODE_INVALID;
    };

    class Structure {
    public:
        Structure(int id, HOUSETYPE house, ItemID_enum itemID, int health, Coord position)
            : id_(id), house_(house), itemID_(itemID), health_(health), position_(position) { }

        int id_;
        HOUSETYPE house_;
        ItemID_enum itemID_;
        int health_;
        Coord position_;
    };

    class Unit {
    public:
        Unit(int id, HOUSETYPE house, ItemID_enum itemID, int health, Coord position, ANGLETYPE angle,
             ATTACKMODE attackmode)
            : id_(id), house_(house), angle_(angle), itemID_(itemID), health_(health), position_(position),
              attack_mode_(attackmode) { }

        int id_;
        HOUSETYPE house_;
        ANGLETYPE angle_;
        ItemID_enum itemID_;
        int health_;
        Coord position_;
        ATTACKMODE attack_mode_;
    };

    MapEditor();
    MapEditor(const MapEditor& o) = delete;
    ~MapEditor();

    friend class INIMapEditorLoader; ///< loading INI Maps is done with a INIMapEditorLoader helper object

    void RunEditor(MenuBase::event_handler_type eventHandler);

    void onQuit() { bQuitEditor_ = true; }

    [[nodiscard]] const std::filesystem::path& getLastSaveName() const { return lastSaveName_; }

    [[nodiscard]] bool hasChangeSinceLastSave() const { return bChangedSinceLastSave_; }

    [[nodiscard]] std::string generateMapname();

    std::vector<Player>& getPlayers() { return players_; }

    void informPlayersChanged() { pInterface_->onHouseChanges(); }

    void setMirrorMode(MirrorMode newMirrorMode);

    const MapMirror* getMapMirror() { return mapMirror_.get(); }

    MapInfo& getMapInfo() { return mapInfo_; }

    void setMapInfo(const MapInfo& newMapInfo) { mapInfo_ = newMapInfo; }

    [[nodiscard]] int getMapVersion() const {
        if (mapInfo_.mapSeed != INVALID) {
            return 1;
        }
        return 2;
    }

    void setMap(const MapData& mapdata, const MapInfo& newMapInfo);

    MapData& getMap() { return map_; }

    std::vector<Coord>& getSpiceBlooms() { return spiceBlooms_; }

    std::vector<Coord>& getSpecialBlooms() { return specialBlooms_; }

    std::vector<Coord>& getSpiceFields() { return spiceFields_; }

    std::map<ItemID_enum, int>& getChoam() { return choam_; }

    std::vector<ReinforcementInfo>& getReinforcements() { return reinforcements_; }

    void setReinforcements(const std::vector<ReinforcementInfo>& newReinforcements) {
        reinforcements_ = newReinforcements;
    }

    std::vector<AITeamInfo>& getAITeams() { return ai_teams_; }

    void setAITeams(const std::vector<AITeamInfo>& newAITeams) { ai_teams_ = newAITeams; }

    std::vector<Structure>& getStructureList() { return structures_; }

    [[nodiscard]] Structure* getStructure(int structureID);

    [[nodiscard]] const Structure* getStructure(int structureID) const;

    [[nodiscard]] std::vector<int> getMirrorStructures(int unitID) const;

    [[nodiscard]] int getSelectedStructureID() const { return selectedStructureID_; }

    Structure* getSelectedStructure() { return getStructure(selectedStructureID_); }
    [[nodiscard]] const Structure* getSelectedStructure() const { return getStructure(selectedStructureID_); }

    std::vector<Unit>& getUnitList() { return units_; }

    [[nodiscard]] const Unit* getUnit(int unitID) const;

    Unit* getUnit(int unitID);

    [[nodiscard]] std::vector<int> getMirrorUnits(int unitID, bool bAddMissingAsInvalid = false) const;

    [[nodiscard]] int getSelectedUnitID() const { return selectedUnitID_; }

    [[nodiscard]] const Unit* getSelectedUnit() const { return getUnit(selectedUnitID_); }

    Unit* getSelectedUnit() { return getUnit(selectedUnitID_); }

    [[nodiscard]] bool isTileBlocked(int x, int y, bool bSlabIsBlocking, bool bUnitsAreBlocking) const;

    void setEditorMode(const EditorMode& newEditorMode);

    void startOperation();

    void addUndoOperation(std::unique_ptr<MapEditorOperation> op) {
        undoOperationStack_.push(std::move(op));
        bChangedSinceLastSave_ = true;
    }

    void undoLastOperation();

    void redoLastOperation();

    void clearRedoOperations();

    void saveMap(const std::filesystem::path& filepath);
    void loadMap(const std::filesystem::path& filepath);

private:
    void performMapEdit(int xpos, int ypos, bool bRepeated);

    void performTerrainChange(int x, int y, TERRAINTYPE terrainType);

    void drawScreen();
    void processInput(MenuBase::event_handler_type handler);
    void drawCursor();
    void drawMap(ScreenBorder* pScreenborder, bool bCompleteMap) const;
    [[nodiscard]] TERRAINTYPE getTerrain(int x, int y) const;
    void saveMapshot();

private:
    std::unique_ptr<MapEditorInterface> pInterface_; ///< This is the whole interface (top bar and side bar)

    SDL_Rect sideBarPos_;
    SDL_Rect topBarPos_;
    SDL_Rect bottomBarPos_;

    bool bQuitEditor_ = false;

    bool scrollDownMode_  = false; ///< currently scrolling the map down?
    bool scrollLeftMode_  = false; ///< currently scrolling the map left?
    bool scrollRightMode_ = false; ///< currently scrolling the map right?
    bool scrollUpMode_    = false; ///< currently scrolling the map up?

    bool shift_ = false;

    bool bChangedSinceLastSave_ = false;

    EditorMode currentEditorMode_;

    MirrorMode currentMirrorMode_;
    std::unique_ptr<MapMirror> mapMirror_;

    bool bLeftMousePressed_  = false;
    int lastTerrainEditPosX_ = -1;
    int lastTerrainEditPosY_ = -1;

    int selectedUnitID_      = INVALID;
    int selectedStructureID_ = INVALID;
    Coord selectedMapItemCoord_; ///< only used for classic maps

    std::filesystem::path lastSaveName_;
    std::unique_ptr<INIFile> loadedINIFile_;

    std::vector<Player> players_;

    MapData map_;
    MapInfo mapInfo_;

    std::vector<Coord> spiceBlooms_;   ///< only for classic maps
    std::vector<Coord> specialBlooms_; ///< only for classic maps
    std::vector<Coord> spiceFields_;   ///< only for classic maps

    std::map<ItemID_enum, int> choam_;

    std::vector<ReinforcementInfo> reinforcements_;

    std::vector<AITeamInfo> ai_teams_;

    std::vector<Unit> units_;
    std::vector<Structure> structures_;

    std::stack<std::unique_ptr<MapEditorOperation>> undoOperationStack_;
    std::stack<std::unique_ptr<MapEditorOperation>> redoOperationStack_;
};

#endif // MAPEDITOR_H
