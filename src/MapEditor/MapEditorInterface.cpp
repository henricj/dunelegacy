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

#include <MapEditor/ChoamWindow.h>
#include <MapEditor/LoadMapWindow.h>
#include <MapEditor/MapEditor.h>
#include <MapEditor/MapEditorInterface.h>
#include <MapEditor/MapInfo.h>
#include <MapEditor/MapMirror.h>
#include <MapEditor/MapSettingsWindow.h>
#include <MapEditor/NewMapWindow.h>
#include <MapEditor/PlayerSettingsWindow.h>
#include <MapEditor/ReinforcementsWindow.h>
#include <MapEditor/TeamsWindow.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <Game.h>
#include <House.h>
#include <ScreenBorder.h>
#include <sand.h>

#include <misc/FileSystem.h>
#include <misc/fnkdat.h>

#include <GUI/ObjectInterfaces/MultiUnitInterface.h>
#include <GUI/ObjectInterfaces/ObjectInterface.h>
#include <GUI/QstBox.h>
#include <GUI/dune/LoadSaveWindow.h>
#include <ObjectBase.h>

#include <misc/SDL2pp.h>
#include <misc/draw_util.h>

MapEditorInterface::MapEditorInterface(MapEditor* pMapEditor)
    : Window(0, 0, 0, 0), pMapEditor_(pMapEditor), radarView(pMapEditor), currentEditStructureID_(INVALID),
      currentEditUnitID_(INVALID), house_(HOUSETYPE::HOUSE_HARKONNEN) {

    color_ = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[static_cast<int>(house_)] + 3]);

    MapEditorInterface::setTransparentBackground(true);

    MapEditorInterface::setCurrentPosition(0, 0, getRendererWidth(), getRendererHeight());

    MapEditorInterface::setWindowWidget(&windowWidget);

    auto* const gfx = dune::globals::pGFXManager.get();

    // top bar
    const auto* const pTopBarTexture = gfx->getUIGraphic(UI_TopBar, HOUSETYPE::HOUSE_HARKONNEN);
    topBar.setTexture(pTopBarTexture);
    windowWidget.addWidget(&topBar, calcAlignedDrawingRect(pTopBarTexture, HAlign::Left, VAlign::Top));

    // side bar
    const auto* const pSideBarTexture = gfx->getUIGraphic(UI_MapEditor_SideBar, HOUSETYPE::HOUSE_HARKONNEN);
    sideBar.setTexture(pSideBarTexture);
    windowWidget.addWidget(&sideBar, calcAlignedDrawingRect(pSideBarTexture, HAlign::Right, VAlign::Top));

    // bottom bar
    const auto* const pBottomBarTexture = gfx->getUIGraphic(UI_MapEditor_BottomBar, HOUSETYPE::HOUSE_HARKONNEN);
    bottomBar.setTexture(pBottomBarTexture);
    windowWidget.addWidget(&bottomBar, calcAlignedDrawingRect(pBottomBarTexture, HAlign::Left, VAlign::Bottom));

    // add radar
    windowWidget.addWidget(&radarView, Point(getRendererWidth() - SIDEBARWIDTH + SIDEBAR_COLUMN_WIDTH, 0),
                           radarView.getMinimumSize());
    radarView.setOnRadarClick(
        [this](auto position, auto mouse_button, auto drag) { return onRadarClick(position, mouse_button, drag); });

    // add buttons
    windowWidget.addWidget(&topBarHBox, Point(0, 3), Point(getRendererWidth() - sideBar.getSize().x, 24));

    topBarHBox.addWidget(Widget::create<HSpacer>(3).release());

    exitButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_ExitIcon));
    exitButton.setTooltipText(_("Leave Mapeditor"));
    exitButton.setOnClick([this] { onQuit(); });
    topBarHBox.addWidget(&exitButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(10).release());

    newButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_NewIcon));
    newButton.setTooltipText(_("New Map"));
    newButton.setOnClick([this] { onNew(); });
    topBarHBox.addWidget(&newButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    loadButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_LoadIcon));
    loadButton.setTooltipText(_("Load Map"));
    loadButton.setOnClick([this] { onLoad(); });
    topBarHBox.addWidget(&loadButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    saveButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_SaveIcon));
    saveButton.setTooltipText(_("Save Map"));
    saveButton.setOnClick([this] { onSave(); });
    topBarHBox.addWidget(&saveButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(10).release());

    undoButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_UndoIcon));
    undoButton.setTooltipText(_("Undo"));
    undoButton.setOnClick([this] { onUndo(); });
    topBarHBox.addWidget(&undoButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    redoButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_RedoIcon));
    redoButton.setTooltipText(_("Redo"));
    redoButton.setOnClick([this] { onRedo(); });
    topBarHBox.addWidget(&redoButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(10).release());

    playersButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_PlayerIcon));
    playersButton.setTooltipText(_("Player Settings"));
    playersButton.setOnClick([this] { onPlayers(); });
    topBarHBox.addWidget(&playersButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    mapSettingsButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_MapSettingsIcon));
    mapSettingsButton.setTooltipText(_("Map Settings"));
    mapSettingsButton.setOnClick([this] { onMapSettings(); });
    topBarHBox.addWidget(&mapSettingsButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(10).release());

    choamButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_ChoamIcon));
    choamButton.setTooltipText(_("Choam"));
    choamButton.setOnClick([this] { onChoam(); });
    topBarHBox.addWidget(&choamButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    reinforcementsButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_ReinforcementsIcon));
    reinforcementsButton.setTooltipText(_("Reinforcements"));
    reinforcementsButton.setOnClick([this] { onReinforcements(); });
    topBarHBox.addWidget(&reinforcementsButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    teamsButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_TeamsIcon));
    teamsButton.setTooltipText(_("Teams"));
    teamsButton.setOnClick([this] { onTeams(); });
    teamsButton.setVisible((pMapEditor->getMapVersion() < 2));
    topBarHBox.addWidget(&teamsButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(10).release());

    mirrorModeNoneButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_MirrorNoneIcon));
    mirrorModeNoneButton.setToggleButton(true);
    mirrorModeNoneButton.setTooltipText(_("Mirror mode") + ": " + _("Off"));
    mirrorModeNoneButton.setOnClick([this] { onMirrorModeButton(MirrorModeNone); });
    mirrorModeNoneButton.setVisible((pMapEditor->getMapVersion() >= 2));
    topBarHBox.addWidget(&mirrorModeNoneButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    mirrorModeHorizontalButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_MirrorHorizontalIcon));
    mirrorModeHorizontalButton.setToggleButton(true);
    mirrorModeHorizontalButton.setTooltipText(_("Mirror mode") + ": " + _("Horizontal"));
    mirrorModeHorizontalButton.setOnClick([this] { onMirrorModeButton(MirrorModeHorizontal); });
    mirrorModeHorizontalButton.setVisible((pMapEditor->getMapVersion() >= 2));
    topBarHBox.addWidget(&mirrorModeHorizontalButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    mirrorModeVerticalButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_MirrorVerticalIcon));
    mirrorModeVerticalButton.setToggleButton(true);
    mirrorModeVerticalButton.setTooltipText(_("Mirror mode") + ": " + _("Vertical"));
    mirrorModeVerticalButton.setOnClick([this] { onMirrorModeButton(MirrorModeVertical); });
    mirrorModeVerticalButton.setVisible((pMapEditor->getMapVersion() >= 2));
    topBarHBox.addWidget(&mirrorModeVerticalButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    mirrorModeBothButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_MirrorBothIcon));
    mirrorModeBothButton.setToggleButton(true);
    mirrorModeBothButton.setTooltipText(_("Mirror mode") + ": " + _("Horizontal and vertical"));
    mirrorModeBothButton.setOnClick([this] { onMirrorModeButton(MirrorModeBoth); });
    mirrorModeBothButton.setVisible((pMapEditor->getMapVersion() >= 2));
    topBarHBox.addWidget(&mirrorModeBothButton, 24);

    topBarHBox.addWidget(Widget::create<HSpacer>(1).release());

    mirrorModePointButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_MirrorPointIcon));
    mirrorModePointButton.setToggleButton(true);
    mirrorModePointButton.setTooltipText(_("Mirror mode") + ": " + _("Inverse"));
    mirrorModePointButton.setOnClick([this] { onMirrorModeButton(MirrorModePoint); });
    mirrorModePointButton.setVisible((pMapEditor->getMapVersion() >= 2));
    topBarHBox.addWidget(&mirrorModePointButton, 24);

    topBarHBox.addWidget(Widget::create<Spacer>().release(), 0.5);

    // add editor mode buttons
    windowWidget.addWidget(&editorModeChooserHBox, Point(getRendererWidth() - sideBar.getSize().x + 14, 148),
                           Point(sideBar.getSize().x - 15, 30));

    terrainButton.setText("T");
    terrainButton.setToggleButton(true);
    terrainButton.setOnClick([this] { onModeButton(1); });
    editorModeChooserHBox.addWidget(&terrainButton);

    editorModeChooserHBox.addWidget(Widget::create<HSpacer>(2).release());

    structuresButton.setText("S");
    structuresButton.setToggleButton(true);
    structuresButton.setOnClick([this] { onModeButton(2); });
    editorModeChooserHBox.addWidget(&structuresButton);

    editorModeChooserHBox.addWidget(Widget::create<HSpacer>(2).release());

    unitsButton.setText("U");
    unitsButton.setToggleButton(true);
    unitsButton.setOnClick([this] { onModeButton(3); });
    editorModeChooserHBox.addWidget(&unitsButton);

    // house choice
    houseDropDownBox.setOnSelectionChange([this](auto interactive) { onHouseDropDownChanged(interactive); });
    windowWidget.addWidget(&houseDropDownBox, Point(getRendererWidth() - sideBar.getSize().x + 14, 179),
                           Point(sideBar.getSize().x - 15, 20));

    // setup terrain mode

    editorModeTerrainVBox.addWidget(&editorModeTerrain_VBox, sideBar.getSize().x - 17);

    editorModeTerrain_VBox.addWidget(&editorModeTerrain_HBox1);

    editorModeTerrain_Sand.setToggleButton(true);
    editorModeTerrain_Sand.setOnClick([this] { onTerrainButton(Terrain_Sand); });
    editorModeTerrain_HBox1.addWidget(&editorModeTerrain_Sand);

    editorModeTerrain_HBox1.addWidget(Widget::create<HSpacer>(2).release());

    editorModeTerrain_Dunes.setToggleButton(true);
    editorModeTerrain_Dunes.setOnClick([this] { onTerrainButton(Terrain_Dunes); });
    editorModeTerrain_HBox1.addWidget(&editorModeTerrain_Dunes);

    editorModeTerrain_HBox1.addWidget(Widget::create<HSpacer>(2).release());

    editorModeTerrain_SpecialBloom.setToggleButton(true);
    editorModeTerrain_SpecialBloom.setOnClick([this] { onTerrainButton(Terrain_SpecialBloom); });
    editorModeTerrain_HBox1.addWidget(&editorModeTerrain_SpecialBloom);

    editorModeTerrain_VBox.addWidget(Widget::create<VSpacer>(2).release());
    editorModeTerrain_VBox.addWidget(&editorModeTerrain_HBox2);

    editorModeTerrain_Spice.setToggleButton(true);
    editorModeTerrain_Spice.setOnClick([this] { onTerrainButton(Terrain_Spice); });
    editorModeTerrain_HBox2.addWidget(&editorModeTerrain_Spice);

    editorModeTerrain_HBox2.addWidget(Widget::create<HSpacer>(2).release());

    editorModeTerrain_ThickSpice.setToggleButton(true);
    editorModeTerrain_ThickSpice.setOnClick([this] { onTerrainButton(Terrain_ThickSpice); });
    editorModeTerrain_HBox2.addWidget(&editorModeTerrain_ThickSpice);

    editorModeTerrain_HBox2.addWidget(Widget::create<HSpacer>(2).release());

    editorModeTerrain_SpiceBloom.setToggleButton(true);
    editorModeTerrain_SpiceBloom.setOnClick([this] { onTerrainButton(Terrain_SpiceBloom); });
    editorModeTerrain_HBox2.addWidget(&editorModeTerrain_SpiceBloom);

    editorModeTerrain_VBox.addWidget(Widget::create<VSpacer>(2).release());
    editorModeTerrain_VBox.addWidget(&editorModeTerrain_HBox3);

    editorModeTerrain_Rock.setToggleButton(true);
    editorModeTerrain_Rock.setOnClick([this] { onTerrainButton(Terrain_Rock); });
    editorModeTerrain_HBox3.addWidget(&editorModeTerrain_Rock);

    editorModeTerrain_HBox3.addWidget(Widget::create<HSpacer>(2).release());

    editorModeTerrain_Mountain.setToggleButton(true);
    editorModeTerrain_Mountain.setOnClick([this] { onTerrainButton(Terrain_Mountain); });
    editorModeTerrain_HBox3.addWidget(&editorModeTerrain_Mountain);

    editorModeTerrain_HBox3.addWidget(Widget::create<Spacer>().release());

    editorModeTerrainVBox.addWidget(Widget::create<VSpacer>(10).release());

    // setup terrain pen size buttons
    editorModeTerrainVBox.addWidget(&editorModeTerrain_PenHBox, 40);

    editorModeTerrain_Pen1x1.setToggleButton(true);
    editorModeTerrain_Pen1x1.setOnClick([this] { onTerrainPenButton(1); });
    editorModeTerrain_PenHBox.addWidget(&editorModeTerrain_Pen1x1);

    editorModeTerrain_PenHBox.addWidget(Widget::create<HSpacer>(2).release());

    editorModeTerrain_Pen3x3.setToggleButton(true);
    editorModeTerrain_Pen3x3.setOnClick([this] { onTerrainPenButton(3); });
    editorModeTerrain_PenHBox.addWidget(&editorModeTerrain_Pen3x3);

    editorModeTerrain_PenHBox.addWidget(Widget::create<HSpacer>(2).release());

    editorModeTerrain_Pen5x5.setToggleButton(true);
    editorModeTerrain_Pen5x5.setOnClick([this] { onTerrainPenButton(5); });
    editorModeTerrain_PenHBox.addWidget(&editorModeTerrain_Pen5x5);

    // setup classic terrain buttons
    editorModeClassicTerrain_MainVBox.addWidget(&editorModeClassicTerrain_VBox, sideBar.getSize().x - 17);

    editorModeClassicTerrain_VBox.addWidget(&editorModeClassicTerrain_HBox1);

    editorModeClassicTerrain_SpiceBloom.setToggleButton(true);
    editorModeClassicTerrain_SpiceBloom.setOnClick([this] { onTerrainButton(Terrain_SpiceBloom); });
    editorModeClassicTerrain_HBox1.addWidget(&editorModeClassicTerrain_SpiceBloom);

    editorModeClassicTerrain_HBox1.addWidget(Widget::create<HSpacer>(2).release());

    editorModeClassicTerrain_SpecialBloom.setToggleButton(true);
    editorModeClassicTerrain_SpecialBloom.setOnClick([this] { onTerrainButton(Terrain_SpecialBloom); });
    editorModeClassicTerrain_HBox1.addWidget(&editorModeClassicTerrain_SpecialBloom);

    editorModeClassicTerrain_HBox1.addWidget(Widget::create<HSpacer>(2).release());

    editorModeClassicTerrain_SpiceField.setToggleButton(true);
    editorModeClassicTerrain_SpiceField.setOnClick([this] { onTerrainButton(Terrain_Spice); });
    editorModeClassicTerrain_HBox1.addWidget(&editorModeClassicTerrain_SpiceField);

    editorModeClassicTerrain_VBox.addWidget(Widget::create<VSpacer>(2).release());
    editorModeClassicTerrain_VBox.addWidget(Widget::create<Spacer>().release());
    editorModeClassicTerrain_VBox.addWidget(Widget::create<VSpacer>(2).release());
    editorModeClassicTerrain_SetTacticalPos.setText(_("Set starting screen"));
    editorModeClassicTerrain_SetTacticalPos.setOnClick([this] { onSetTacticalPosition(); });
    editorModeClassicTerrain_VBox.addWidget(&editorModeClassicTerrain_SetTacticalPos);

    // setup structures mode
    editorModeStructs_MainVBox.addWidget(&editorModeStructs_VBox, 0.01);

    editorModeStructs_VBox.addWidget(&editorModeStructs_HBox1, 2 * D2_TILESIZE + 10);

    editorModeStructs_HBox1.addWidget(&editorModeStructs_SmallStruct_VBox);

    editorModeStructs_SmallStruct_VBox.addWidget(&editorModeStructs_SmallStruct_HBox1);

    editorModeStructs_Slab1.setToggleButton(true);
    editorModeStructs_Slab1.setTooltipText(resolveItemName(Structure_Slab1));
    editorModeStructs_Slab1.setOnClick([this] { onStructButton(Structure_Slab1); });
    editorModeStructs_SmallStruct_HBox1.addWidget(&editorModeStructs_Slab1);

    editorModeStructs_Wall.setToggleButton(true);
    editorModeStructs_Wall.setTooltipText(resolveItemName(Structure_Wall));
    editorModeStructs_Wall.setOnClick([this] { onStructButton(Structure_Wall); });
    editorModeStructs_SmallStruct_HBox1.addWidget(&editorModeStructs_Wall);

    editorModeStructs_SmallStruct_VBox.addWidget(&editorModeStructs_SmallStruct_HBox2);

    editorModeStructs_GunTurret.setToggleButton(true);
    editorModeStructs_GunTurret.setTooltipText(resolveItemName(Structure_GunTurret));
    editorModeStructs_GunTurret.setOnClick([this] { onStructButton(Structure_GunTurret); });
    editorModeStructs_SmallStruct_HBox2.addWidget(&editorModeStructs_GunTurret);

    editorModeStructs_RocketTurret.setToggleButton(true);
    editorModeStructs_RocketTurret.setTooltipText(resolveItemName(Structure_RocketTurret));
    editorModeStructs_RocketTurret.setOnClick([this] { onStructButton(Structure_RocketTurret); });
    editorModeStructs_SmallStruct_HBox2.addWidget(&editorModeStructs_RocketTurret);

    editorModeStructs_HBox1.addWidget(Widget::create<HSpacer>(2).release());

    editorModeStructs_ConstructionYard.setToggleButton(true);
    editorModeStructs_ConstructionYard.setTooltipText(resolveItemName(Structure_ConstructionYard));
    editorModeStructs_ConstructionYard.setOnClick([this] { onStructButton(Structure_ConstructionYard); });
    editorModeStructs_HBox1.addWidget(&editorModeStructs_ConstructionYard);

    editorModeStructs_HBox1.addWidget(Widget::create<HSpacer>(2).release());

    editorModeStructs_Windtrap.setToggleButton(true);
    editorModeStructs_Windtrap.setTooltipText(resolveItemName(Structure_WindTrap));
    editorModeStructs_Windtrap.setOnClick([this] { onStructButton(Structure_WindTrap); });
    editorModeStructs_HBox1.addWidget(&editorModeStructs_Windtrap);

    editorModeStructs_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeStructs_VBox.addWidget(&editorModeStructs_HBox2, 2 * D2_TILESIZE + 10);

    editorModeStructs_Radar.setToggleButton(true);
    editorModeStructs_Radar.setTooltipText(resolveItemName(Structure_Radar));
    editorModeStructs_Radar.setOnClick([this] { onStructButton(Structure_Radar); });
    editorModeStructs_HBox2.addWidget(&editorModeStructs_Radar);

    editorModeStructs_HBox2.addWidget(Widget::create<HSpacer>(2).release());

    editorModeStructs_Silo.setToggleButton(true);
    editorModeStructs_Silo.setTooltipText(resolveItemName(Structure_Silo));
    editorModeStructs_Silo.setOnClick([this] { onStructButton(Structure_Silo); });
    editorModeStructs_HBox2.addWidget(&editorModeStructs_Silo);

    editorModeStructs_HBox2.addWidget(Widget::create<HSpacer>(2).release());

    editorModeStructs_IX.setToggleButton(true);
    editorModeStructs_IX.setTooltipText(resolveItemName(Structure_IX));
    editorModeStructs_IX.setOnClick([this] { onStructButton(Structure_IX); });
    editorModeStructs_HBox2.addWidget(&editorModeStructs_IX);

    editorModeStructs_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeStructs_VBox.addWidget(&editorModeStructs_HBox3, 2 * D2_TILESIZE + 10);

    editorModeStructs_Barracks.setToggleButton(true);
    editorModeStructs_Barracks.setTooltipText(resolveItemName(Structure_Barracks));
    editorModeStructs_Barracks.setOnClick([this] { onStructButton(Structure_Barracks); });
    editorModeStructs_HBox3.addWidget(&editorModeStructs_Barracks);

    editorModeStructs_HBox3.addWidget(Widget::create<HSpacer>(2).release());

    editorModeStructs_WOR.setToggleButton(true);
    editorModeStructs_WOR.setTooltipText(resolveItemName(Structure_WOR));
    editorModeStructs_WOR.setOnClick([this] { onStructButton(Structure_WOR); });
    editorModeStructs_HBox3.addWidget(&editorModeStructs_WOR);

    editorModeStructs_HBox3.addWidget(Widget::create<HSpacer>(2).release());

    editorModeStructs_LightFactory.setToggleButton(true);
    editorModeStructs_LightFactory.setTooltipText(resolveItemName(Structure_LightFactory));
    editorModeStructs_LightFactory.setOnClick([this] { onStructButton(Structure_LightFactory); });
    editorModeStructs_HBox3.addWidget(&editorModeStructs_LightFactory);

    editorModeStructs_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeStructs_VBox.addWidget(&editorModeStructs_HBox4, 2 * D2_TILESIZE + 10);

    editorModeStructs_Refinery.setToggleButton(true);
    editorModeStructs_Refinery.setTooltipText(resolveItemName(Structure_Refinery));
    editorModeStructs_Refinery.setOnClick([this] { onStructButton(Structure_Refinery); });
    editorModeStructs_HBox4.addWidget(&editorModeStructs_Refinery);

    editorModeStructs_HBox4.addWidget(Widget::create<HSpacer>(2).release());

    editorModeStructs_HighTechFactory.setToggleButton(true);
    editorModeStructs_HighTechFactory.setTooltipText(resolveItemName(Structure_HighTechFactory));
    editorModeStructs_HighTechFactory.setOnClick([this] { onStructButton(Structure_HighTechFactory); });
    editorModeStructs_HBox4.addWidget(&editorModeStructs_HighTechFactory);

    editorModeStructs_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeStructs_VBox.addWidget(&editorModeStructs_HBox5, 2 * D2_TILESIZE + 10);

    editorModeStructs_HeavyFactory.setToggleButton(true);
    editorModeStructs_HeavyFactory.setTooltipText(resolveItemName(Structure_HeavyFactory));
    editorModeStructs_HeavyFactory.setOnClick([this] { onStructButton(Structure_HeavyFactory); });
    editorModeStructs_HBox5.addWidget(&editorModeStructs_HeavyFactory);

    editorModeStructs_HBox5.addWidget(Widget::create<HSpacer>(2).release());

    editorModeStructs_RepairYard.setToggleButton(true);
    editorModeStructs_RepairYard.setTooltipText(resolveItemName(Structure_RepairYard));
    editorModeStructs_RepairYard.setOnClick([this] { onStructButton(Structure_RepairYard); });
    editorModeStructs_HBox5.addWidget(&editorModeStructs_RepairYard);

    editorModeStructs_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeStructs_VBox.addWidget(&editorModeStructs_HBox6, 3 * D2_TILESIZE + 10);

    editorModeStructs_Starport.setToggleButton(true);
    editorModeStructs_Starport.setTooltipText(resolveItemName(Structure_StarPort));
    editorModeStructs_Starport.setOnClick([this] { onStructButton(Structure_StarPort); });
    editorModeStructs_HBox6.addWidget(&editorModeStructs_Starport);

    editorModeStructs_HBox6.addWidget(Widget::create<HSpacer>(2).release());

    editorModeStructs_Palace.setToggleButton(true);
    editorModeStructs_Palace.setTooltipText(resolveItemName(Structure_Palace));
    editorModeStructs_Palace.setOnClick([this] { onStructButton(Structure_Palace); });
    editorModeStructs_HBox6.addWidget(&editorModeStructs_Palace);

    editorModeStructs_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeStructs_MainVBox.addWidget(Widget::create<Spacer>().release());

    // setup units mode
    editorModeUnits_MainVBox.addWidget(&editorModeUnits_VBox, 0.01);

    editorModeUnits_VBox.addWidget(&editorModeUnits_HBox1, 2 * D2_TILESIZE);

    editorModeUnits_Soldier.setToggleButton(true);
    editorModeUnits_Soldier.setTooltipText(resolveItemName(Unit_Soldier));
    editorModeUnits_Soldier.setOnClick([this] { onUnitButton(Unit_Soldier); });
    editorModeUnits_HBox1.addWidget(&editorModeUnits_Soldier);

    editorModeUnits_HBox1.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_Trooper.setToggleButton(true);
    editorModeUnits_Trooper.setTooltipText(resolveItemName(Unit_Trooper));
    editorModeUnits_Trooper.setOnClick([this] { onUnitButton(Unit_Trooper); });
    editorModeUnits_HBox1.addWidget(&editorModeUnits_Trooper);

    editorModeUnits_HBox1.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_Harvester.setToggleButton(true);
    editorModeUnits_Harvester.setTooltipText(resolveItemName(Unit_Harvester));
    editorModeUnits_Harvester.setOnClick([this] { onUnitButton(Unit_Harvester); });
    editorModeUnits_HBox1.addWidget(&editorModeUnits_Harvester);

    editorModeUnits_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeUnits_VBox.addWidget(&editorModeUnits_HBox2, 2 * D2_TILESIZE);

    editorModeUnits_Infantry.setToggleButton(true);
    editorModeUnits_Infantry.setTooltipText(resolveItemName(Unit_Infantry));
    editorModeUnits_Infantry.setOnClick([this] { onUnitButton(Unit_Infantry); });
    editorModeUnits_HBox2.addWidget(&editorModeUnits_Infantry);

    editorModeUnits_HBox2.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_Troopers.setToggleButton(true);
    editorModeUnits_Troopers.setTooltipText(resolveItemName(Unit_Troopers));
    editorModeUnits_Troopers.setOnClick([this] { onUnitButton(Unit_Troopers); });
    editorModeUnits_HBox2.addWidget(&editorModeUnits_Troopers);

    editorModeUnits_HBox2.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_MCV.setToggleButton(true);
    editorModeUnits_MCV.setTooltipText(resolveItemName(Unit_MCV));
    editorModeUnits_MCV.setOnClick([this] { onUnitButton(Unit_MCV); });
    editorModeUnits_HBox2.addWidget(&editorModeUnits_MCV);

    editorModeUnits_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeUnits_VBox.addWidget(&editorModeUnits_HBox3, 2 * D2_TILESIZE);

    editorModeUnits_Trike.setToggleButton(true);
    editorModeUnits_Trike.setTooltipText(resolveItemName(Unit_Trike));
    editorModeUnits_Trike.setOnClick([this] { onUnitButton(Unit_Trike); });
    editorModeUnits_HBox3.addWidget(&editorModeUnits_Trike);

    editorModeUnits_HBox3.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_Raider.setToggleButton(true);
    editorModeUnits_Raider.setTooltipText(resolveItemName(Unit_RaiderTrike));
    editorModeUnits_Raider.setOnClick([this] { onUnitButton(Unit_RaiderTrike); });
    editorModeUnits_HBox3.addWidget(&editorModeUnits_Raider);

    editorModeUnits_HBox3.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_Quad.setToggleButton(true);
    editorModeUnits_Quad.setTooltipText(resolveItemName(Unit_Quad));
    editorModeUnits_Quad.setOnClick([this] { onUnitButton(Unit_Quad); });
    editorModeUnits_HBox3.addWidget(&editorModeUnits_Quad);

    editorModeUnits_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeUnits_VBox.addWidget(&editorModeUnits_HBox4, 2 * D2_TILESIZE);

    editorModeUnits_Tank.setToggleButton(true);
    editorModeUnits_Tank.setTooltipText(resolveItemName(Unit_Tank));
    editorModeUnits_Tank.setOnClick([this] { onUnitButton(Unit_Tank); });
    editorModeUnits_HBox4.addWidget(&editorModeUnits_Tank);

    editorModeUnits_HBox4.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_SiegeTank.setToggleButton(true);
    editorModeUnits_SiegeTank.setTooltipText(resolveItemName(Unit_SiegeTank));
    editorModeUnits_SiegeTank.setOnClick([this] { onUnitButton(Unit_SiegeTank); });
    editorModeUnits_HBox4.addWidget(&editorModeUnits_SiegeTank);

    editorModeUnits_HBox4.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_Launcher.setToggleButton(true);
    editorModeUnits_Launcher.setTooltipText(resolveItemName(Unit_Launcher));
    editorModeUnits_Launcher.setOnClick([this] { onUnitButton(Unit_Launcher); });
    editorModeUnits_HBox4.addWidget(&editorModeUnits_Launcher);

    editorModeUnits_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeUnits_VBox.addWidget(&editorModeUnits_HBox5, 2 * D2_TILESIZE);

    editorModeUnits_Devastator.setToggleButton(true);
    editorModeUnits_Devastator.setTooltipText(resolveItemName(Unit_Devastator));
    editorModeUnits_Devastator.setOnClick([this] { onUnitButton(Unit_Devastator); });
    editorModeUnits_HBox5.addWidget(&editorModeUnits_Devastator);

    editorModeUnits_HBox5.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_SonicTank.setToggleButton(true);
    editorModeUnits_SonicTank.setTooltipText(resolveItemName(Unit_SonicTank));
    editorModeUnits_SonicTank.setOnClick([this] { onUnitButton(Unit_SonicTank); });
    editorModeUnits_HBox5.addWidget(&editorModeUnits_SonicTank);

    editorModeUnits_HBox5.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_Deviator.setToggleButton(true);
    editorModeUnits_Deviator.setTooltipText(resolveItemName(Unit_Deviator));
    editorModeUnits_Deviator.setOnClick([this] { onUnitButton(Unit_Deviator); });
    editorModeUnits_HBox5.addWidget(&editorModeUnits_Deviator);

    editorModeUnits_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeUnits_VBox.addWidget(&editorModeUnits_HBox6, 2 * D2_TILESIZE);

    editorModeUnits_Saboteur.setToggleButton(true);
    editorModeUnits_Saboteur.setTooltipText(resolveItemName(Unit_Saboteur));
    editorModeUnits_Saboteur.setOnClick([this] { onUnitButton(Unit_Saboteur); });
    editorModeUnits_HBox6.addWidget(&editorModeUnits_Saboteur);

    editorModeUnits_HBox6.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_Sandworm.setToggleButton(true);
    editorModeUnits_Sandworm.setTooltipText(resolveItemName(Unit_Sandworm));
    editorModeUnits_Sandworm.setOnClick([this] { onUnitButton(Unit_Sandworm); });
    editorModeUnits_HBox6.addWidget(&editorModeUnits_Sandworm);

    editorModeUnits_HBox6.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_SpecialUnit.setToggleButton(true);
    editorModeUnits_SpecialUnit.setTooltipText("Special");
    editorModeUnits_SpecialUnit.setOnClick([this] { onUnitButton(Unit_Special); });
    editorModeUnits_HBox6.addWidget(&editorModeUnits_SpecialUnit);

    editorModeUnits_VBox.addWidget(Widget::create<VSpacer>(2).release());

    editorModeUnits_VBox.addWidget(&editorModeUnits_HBox7, 2 * D2_TILESIZE);

    editorModeUnits_Carryall.setToggleButton(true);
    editorModeUnits_Carryall.setTooltipText(resolveItemName(Unit_Carryall));
    editorModeUnits_Carryall.setOnClick([this] { onUnitButton(Unit_Carryall); });
    editorModeUnits_HBox7.addWidget(&editorModeUnits_Carryall);

    editorModeUnits_HBox7.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_Ornithopter.setToggleButton(true);
    editorModeUnits_Ornithopter.setTooltipText(resolveItemName(Unit_Ornithopter));
    editorModeUnits_Ornithopter.setOnClick([this] { onUnitButton(Unit_Ornithopter); });
    editorModeUnits_HBox7.addWidget(&editorModeUnits_Ornithopter);

    editorModeUnits_HBox7.addWidget(Widget::create<HSpacer>(2).release());

    editorModeUnits_HBox7.addWidget(Widget::create<Spacer>().release());

    editorModeUnits_MainVBox.addWidget(Widget::create<Spacer>().release());

    // bottom bar (structure edit)
    structureDetailsHBox.addWidget(Widget::create<HSpacer>(4).release());
    structureDetailsHealthLabel.setTextFontSize(12);
    structureDetailsHealthLabel.setText("Health:");
    structureDetailsHBox.addWidget(&structureDetailsHealthLabel, 0.1);

    for (int i = 1; i <= 256; i++) {
        structureDetailsHealthDropDownBox.addEntry(fmt::format("{} % ({}/256)", (i * 100) / 256, i), i);
    }

    structureDetailsHealthDropDownBox.setOnSelectionChange(
        [this](auto interactive) { onStructureHealthDropDown(interactive); });

    structureDetailsHBox.addWidget(&structureDetailsHealthDropDownBox, 120);
    structureDetailsHBox.addWidget(Widget::create<Spacer>().release(), 1.0);

    // bottom bar (unit edit)
    unitDetailsHBox.addWidget(Widget::create<HSpacer>(5).release());
    unitDetailsHealthLabel.setTextFontSize(12);
    unitDetailsHealthLabel.setText(_("Health") + ":");
    unitDetailsHBox.addWidget(&unitDetailsHealthLabel, 0.1);

    for (int i = 1; i <= 256; i++) {
        unitDetailsHealthDropDownBox.addEntry(fmt::format("{} % ({}/256)", (i * 100) / 256, i), i);
    }

    unitDetailsHealthDropDownBox.setOnSelectionChange([this](auto interactive) { onUnitHealthDropDown(interactive); });

    unitDetailsHBox.addWidget(&unitDetailsHealthDropDownBox, 115);

    unitDetailsHBox.addWidget(Widget::create<HSpacer>(4).release());
    unitDetailsAttackModeLabel.setTextFontSize(12);
    unitDetailsAttackModeLabel.setText(_("Attack mode") + ":");
    unitDetailsHBox.addWidget(&unitDetailsAttackModeLabel, 0.1);

    unitDetailsAttackModeDropDownBox.addEntry(getAttackModeNameByMode(GUARD), GUARD);
    unitDetailsAttackModeDropDownBox.addEntry(getAttackModeNameByMode(AREAGUARD), AREAGUARD);
    unitDetailsAttackModeDropDownBox.addEntry(getAttackModeNameByMode(AMBUSH), AMBUSH);
    unitDetailsAttackModeDropDownBox.addEntry(getAttackModeNameByMode(HUNT), HUNT);
    unitDetailsAttackModeDropDownBox.addEntry(getAttackModeNameByMode(HARVEST), HARVEST);
    unitDetailsAttackModeDropDownBox.addEntry(getAttackModeNameByMode(SABOTAGE), SABOTAGE);
    unitDetailsAttackModeDropDownBox.addEntry(getAttackModeNameByMode(STOP), STOP);

    unitDetailsAttackModeDropDownBox.setOnSelectionChange(
        [this](auto interactive) { onUnitAttackModeDropDown(interactive); });

    unitDetailsHBox.addWidget(&unitDetailsAttackModeDropDownBox, 90);

    unitDetailsHBox.addWidget(Widget::create<HSpacer>(10).release());

    unitDetailsRotateLeftButton.setTooltipText(_("Rotate left"));
    unitDetailsRotateLeftButton.setOnClick([this] { onSelectedUnitRotateLeft(); });

    unitDetailsHBox.addWidget(&unitDetailsRotateLeftButton, 24);

    unitDetailsHBox.addWidget(Widget::create<HSpacer>(1).release());

    unitDetailsRotateRightButton.setTooltipText(_("Rotate right"));
    unitDetailsRotateRightButton.setOnClick([this] { onSelectedUnitRotateRight(); });

    unitDetailsHBox.addWidget(&unitDetailsRotateRightButton, 24);

    unitDetailsHBox.addWidget(Widget::create<Spacer>().release(), 1.0);

    onHouseChanges();

    onModeButton(1);

    onTerrainPenButton(1);

    onMirrorModeButton(0);
}

MapEditorInterface::~MapEditorInterface() = default;

void MapEditorInterface::onHouseChanges() {

    const int currentSelection = houseDropDownBox.getSelectedEntryIntData();

    houseDropDownBox.clearAllEntries();

    int currentIndex     = 0;
    int currentPlayerNum = 1;
    for (const auto& player : pMapEditor_->getPlayers()) {
        std::string entryName =
            player.bActive_
                ? (player.bAnyHouse_ ? (_("Player") + " " + std::to_string(currentPlayerNum++)) : player.name_)
                : ("(" + player.name_ + ")");

        houseDropDownBox.addEntry(entryName, static_cast<int>(player.house_));

        if (static_cast<int>(player.house_) == currentSelection) {
            houseDropDownBox.setSelectedItem(currentIndex);
        }
        currentIndex++;
    }

    if (currentSelection == -1) {
        houseDropDownBox.setSelectedItem(0);
    }
}

void MapEditorInterface::onNewMap() {
    onModeButton(1);

    onMirrorModeButton(0);

    teamsButton.setVisible((pMapEditor_->getMapVersion() < 2));
    mirrorModeNoneButton.setVisible((pMapEditor_->getMapVersion() >= 2));
    mirrorModeHorizontalButton.setVisible((pMapEditor_->getMapVersion() >= 2));
    mirrorModeVerticalButton.setVisible((pMapEditor_->getMapVersion() >= 2));
    mirrorModeBothButton.setVisible((pMapEditor_->getMapVersion() >= 2));
    mirrorModePointButton.setVisible((pMapEditor_->getMapVersion() >= 2));
    editorModeUnits_SpecialUnit.setVisible((pMapEditor_->getMapVersion() >= 2));
}

void MapEditorInterface::deselectAll() {

    deselectObject();

    onTerrainButton(-1);
    onStructButton(ItemID_enum::ItemID_Invalid);
    onUnitButton(ItemID_enum::ItemID_Invalid);
}

void MapEditorInterface::deselectObject() {
    windowWidget.removeChildWidget(&structureDetailsHBox);
    windowWidget.removeChildWidget(&unitDetailsHBox);
}

void MapEditorInterface::onObjectSelected() {
    windowWidget.removeChildWidget(&structureDetailsHBox);
    windowWidget.removeChildWidget(&unitDetailsHBox);

    const MapEditor::Structure* pStructure = pMapEditor_->getSelectedStructure();

    if (pStructure != nullptr) {
        windowWidget.addWidget(&structureDetailsHBox, Point(0, getRendererHeight() - bottomBar.getSize().y + 14 + 3),
                               Point(getRendererWidth() - sideBar.getSize().x, 24));

        structureDetailsHealthDropDownBox.setSelectedItem(pStructure->health_ - 1);

        changeHouseDropDown(pStructure->house_);
    }

    if (const auto* pUnit = pMapEditor_->getSelectedUnit()) {
        windowWidget.addWidget(&unitDetailsHBox, Point(0, getRendererHeight() - bottomBar.getSize().y + 14 + 3),
                               Point(getRendererWidth() - sideBar.getSize().x, 24));

        unitDetailsHealthDropDownBox.setSelectedItem(pUnit->health_ - 1);
        unitDetailsAttackModeDropDownBox.setSelectedItem(pUnit->attack_mode_);

        changeHouseDropDown(pUnit->house_);
    }
}

void MapEditorInterface::onChildWindowClose(Window* pChildWindow) {
    if (const auto* pNewMapWindow = dynamic_cast<NewMapWindow*>(pChildWindow)) {
        const auto loadMapFilepath = pNewMapWindow->getLoadMapFilepath();

        if (!loadMapFilepath.empty()) {
            pMapEditor_->loadMap(loadMapFilepath);
        } else {
            const MapData& mapdata = pNewMapWindow->getMapData();

            if (mapdata.getSizeX() > 0) {
                pMapEditor_->setMap(mapdata, MapInfo(pNewMapWindow->getMapSeed(), pNewMapWindow->getAuthor(),
                                                     pNewMapWindow->getLicense()));
                onPlayers();
            }
        }
    }

    if (const auto* pLoadMapWindow = dynamic_cast<LoadMapWindow*>(pChildWindow)) {
        const auto loadMapFilepath = pLoadMapWindow->getLoadMapFilepath();

        if (!loadMapFilepath.empty()) {
            pMapEditor_->loadMap(loadMapFilepath);
        }
    }

    const auto* pLoadSaveWindow = dynamic_cast<LoadSaveWindow*>(pChildWindow);
    if (pLoadSaveWindow != nullptr && !pLoadSaveWindow->getFilename().empty()) {
        pMapEditor_->saveMap(pLoadSaveWindow->getFilename());
    }

    const auto* pQstBox = dynamic_cast<QstBox*>(pChildWindow);
    if (pQstBox != nullptr && pQstBox->getPressedButtonID() == QSTBOX_BUTTON1) {
        pMapEditor_->onQuit();
    }
}

void MapEditorInterface::onNew() {
    openWindow(NewMapWindow::create(house_));
}

bool MapEditorInterface::onRadarClick(Coord worldPosition, [[maybe_unused]] bool bRightMouseButton,
                                      [[maybe_unused]] bool bDrag) {
    dune::globals::screenborder->setNewScreenCenter(worldPosition);
    return true;
}

void MapEditorInterface::onQuit() {
    if (pMapEditor_->hasChangeSinceLastSave()) {
        auto* const pQstBox =
            QstBox::create(_("Do you really want to quit and lose unsaved changes to this map?"), _("Yes"), _("No"));
        pQstBox->setTextColor(color_);
        openWindow(pQstBox);
    } else {
        pMapEditor_->onQuit();
    }
}

void MapEditorInterface::onSave() {

    std::vector<std::filesystem::path> mapDirectories;
    std::vector<std::string> directoryTitles;

    auto [ok, tmp] = fnkdat("maps/singleplayer/", FNKDAT_USER | FNKDAT_CREAT);
    mapDirectories.emplace_back(tmp);
    directoryTitles.push_back(_("SP Maps"));

    auto [ok2, tmp2] = fnkdat("maps/multiplayer/", FNKDAT_USER | FNKDAT_CREAT);
    mapDirectories.emplace_back(tmp2);
    directoryTitles.push_back(_("MP Maps"));

    const auto& lastSaveName = pMapEditor_->getLastSaveName();
    std::filesystem::path mapname;
    int lastSaveDirectoryIndex = 0;
    if (lastSaveName.empty()) {
        mapname = pMapEditor_->generateMapname();
    } else {
        mapname = getBasename(lastSaveName, true);

        const auto pathName = getBasename(getDirname(lastSaveName));

        for (int i = 0; i < static_cast<int>(mapDirectories.size()); i++) {
            if (getBasename(mapDirectories[i]) == pathName) {
                lastSaveDirectoryIndex = i;
                break;
            }
        }
    }

    openWindow(LoadSaveWindow::create(true, _("Save Map"), mapDirectories, directoryTitles,
                                      pMapEditor_->getMapVersion() < 2 ? "INI" : "ini", lastSaveDirectoryIndex,
                                      reinterpret_cast<const char*>(mapname.u8string().c_str()), color_)
                   .release());
}

void MapEditorInterface::onLoad() {
    openWindow(LoadMapWindow::create(color_));
}

void MapEditorInterface::onPlayers() {
    openWindow(PlayerSettingsWindow::create(pMapEditor_, house_));
}

void MapEditorInterface::onMapSettings() {
    openWindow(MapSettingsWindow::create(pMapEditor_, house_));
}

void MapEditorInterface::onChoam() {
    openWindow(ChoamWindow::create(pMapEditor_, house_));
}

void MapEditorInterface::onReinforcements() {
    openWindow(ReinforcementsWindow::create(pMapEditor_, house_));
}

void MapEditorInterface::onTeams() {
    openWindow(TeamsWindow::create(pMapEditor_, house_));
}

void MapEditorInterface::onUndo() {
    pMapEditor_->undoLastOperation();

    currentEditUnitID_      = INVALID;
    currentEditStructureID_ = INVALID;

    onObjectSelected();
}

void MapEditorInterface::onRedo() {
    pMapEditor_->redoLastOperation();

    currentEditUnitID_      = INVALID;
    currentEditStructureID_ = INVALID;

    onObjectSelected();
}

void MapEditorInterface::onHouseDropDownChanged(bool bInteractive) {
    int index = houseDropDownBox.getSelectedEntryIntData();
    if (index < 0) {
        return;
    }

    changeInterfaceColor(static_cast<HOUSETYPE>(index));

    if (bInteractive) {
        pMapEditor_->setEditorMode(MapEditor::EditorMode());
        deselectAll();
    }
}

void MapEditorInterface::onModeButton(int button) {
    terrainButton.setToggleState((button == 1));
    structuresButton.setToggleState((button == 2));
    unitsButton.setToggleState((button == 3));

    windowWidget.removeChildWidget(&editorModeTerrainVBox);
    windowWidget.removeChildWidget(&editorModeClassicTerrain_MainVBox);
    windowWidget.removeChildWidget(&editorModeStructs_MainVBox);
    windowWidget.removeChildWidget(&editorModeUnits_MainVBox);

    switch (button) {
        case 1: {
            // add terrain mode
            if (pMapEditor_->getMapVersion() < 2) {
                windowWidget.addWidget(&editorModeClassicTerrain_MainVBox,
                                       Point(getRendererWidth() - sideBar.getSize().x + 14, 200),
                                       Point(sideBar.getSize().x - 14, getRendererHeight() - 200));
            } else {
                windowWidget.addWidget(&editorModeTerrainVBox,
                                       Point(getRendererWidth() - sideBar.getSize().x + 14, 200),
                                       Point(sideBar.getSize().x - 14, getRendererHeight() - 200));
            }
        } break;

        case 2: {
            // add structs mode
            windowWidget.addWidget(&editorModeStructs_MainVBox,
                                   Point(getRendererWidth() - sideBar.getSize().x + 14, 200),
                                   Point(sideBar.getSize().x - 14, getRendererHeight() - 200));
        } break;

        case 3: {
            // add units mode
            windowWidget.addWidget(&editorModeUnits_MainVBox, Point(getRendererWidth() - sideBar.getSize().x + 14, 200),
                                   Point(sideBar.getSize().x - 14, getRendererHeight() - 200));
        } break;

        default: {
            // should never be reached
        } break;
    }

    pMapEditor_->setEditorMode(MapEditor::EditorMode());
    deselectAll();
}

void MapEditorInterface::onTerrainButton(int terrainType) {
    currentTerrainType_ = terrainType;

    editorModeTerrain_Sand.setToggleState((terrainType == Terrain_Sand));
    editorModeTerrain_Dunes.setToggleState((terrainType == Terrain_Dunes));
    editorModeTerrain_SpecialBloom.setToggleState((terrainType == Terrain_SpecialBloom));
    editorModeTerrain_Spice.setToggleState((terrainType == Terrain_Spice));
    editorModeTerrain_ThickSpice.setToggleState((terrainType == Terrain_ThickSpice));
    editorModeTerrain_SpiceBloom.setToggleState((terrainType == Terrain_SpiceBloom));
    editorModeTerrain_Rock.setToggleState((terrainType == Terrain_Rock));
    editorModeTerrain_Mountain.setToggleState((terrainType == Terrain_Mountain));

    editorModeClassicTerrain_SpiceBloom.setToggleState((terrainType == Terrain_SpiceBloom));
    editorModeClassicTerrain_SpecialBloom.setToggleState((terrainType == Terrain_SpecialBloom));
    editorModeClassicTerrain_SpiceField.setToggleState((terrainType == Terrain_Spice));

    if (currentTerrainType_ >= 0) {
        pMapEditor_->setEditorMode(
            MapEditor::EditorMode(static_cast<TERRAINTYPE>(currentTerrainType_), currentTerrainPenSize_));
    }
}

void MapEditorInterface::onTerrainPenButton(int pensize) {
    currentTerrainPenSize_ = pensize;

    editorModeTerrain_Pen1x1.setToggleState((pensize == 1));
    editorModeTerrain_Pen3x3.setToggleState((pensize == 3));
    editorModeTerrain_Pen5x5.setToggleState((pensize == 5));

    if (currentTerrainType_ >= 0) {
        pMapEditor_->setEditorMode(
            MapEditor::EditorMode(static_cast<TERRAINTYPE>(currentTerrainType_), currentTerrainPenSize_));
    }
}

void MapEditorInterface::onSetTacticalPosition() {
    deselectAll();
    pMapEditor_->setEditorMode(MapEditor::EditorMode(true));
}

void MapEditorInterface::onStructButton(ItemID_enum structType) {
    editorModeStructs_Slab1.setToggleState((structType == Structure_Slab1));
    editorModeStructs_Wall.setToggleState((structType == Structure_Wall));
    editorModeStructs_GunTurret.setToggleState((structType == Structure_GunTurret));
    editorModeStructs_RocketTurret.setToggleState((structType == Structure_RocketTurret));
    editorModeStructs_ConstructionYard.setToggleState((structType == Structure_ConstructionYard));
    editorModeStructs_Windtrap.setToggleState((structType == Structure_WindTrap));
    editorModeStructs_Radar.setToggleState((structType == Structure_Radar));
    editorModeStructs_Silo.setToggleState((structType == Structure_Silo));
    editorModeStructs_IX.setToggleState((structType == Structure_IX));
    editorModeStructs_Barracks.setToggleState((structType == Structure_Barracks));
    editorModeStructs_WOR.setToggleState((structType == Structure_WOR));
    editorModeStructs_LightFactory.setToggleState((structType == Structure_LightFactory));
    editorModeStructs_Refinery.setToggleState((structType == Structure_Refinery));
    editorModeStructs_HighTechFactory.setToggleState((structType == Structure_HighTechFactory));
    editorModeStructs_HeavyFactory.setToggleState((structType == Structure_HeavyFactory));
    editorModeStructs_RepairYard.setToggleState((structType == Structure_RepairYard));
    editorModeStructs_Starport.setToggleState((structType == Structure_StarPort));
    editorModeStructs_Palace.setToggleState((structType == Structure_Palace));

    if (structType >= 0) {
        const auto house = static_cast<HOUSETYPE>(houseDropDownBox.getSelectedEntryIntData());
        pMapEditor_->setEditorMode(MapEditor::EditorMode(house, structType, 256));
    }
}

void MapEditorInterface::onUnitButton(ItemID_enum unitType) {
    editorModeUnits_Soldier.setToggleState((unitType == Unit_Soldier));
    editorModeUnits_Trooper.setToggleState((unitType == Unit_Trooper));
    editorModeUnits_Harvester.setToggleState((unitType == Unit_Harvester));
    editorModeUnits_Infantry.setToggleState((unitType == Unit_Infantry));
    editorModeUnits_Troopers.setToggleState((unitType == Unit_Troopers));
    editorModeUnits_MCV.setToggleState((unitType == Unit_MCV));
    editorModeUnits_Trike.setToggleState((unitType == Unit_Trike));
    editorModeUnits_Raider.setToggleState((unitType == Unit_RaiderTrike));
    editorModeUnits_Quad.setToggleState((unitType == Unit_Quad));
    editorModeUnits_Tank.setToggleState((unitType == Unit_Tank));
    editorModeUnits_SiegeTank.setToggleState((unitType == Unit_SiegeTank));
    editorModeUnits_Launcher.setToggleState((unitType == Unit_Launcher));
    editorModeUnits_Devastator.setToggleState((unitType == Unit_Devastator));
    editorModeUnits_SonicTank.setToggleState((unitType == Unit_SonicTank));
    editorModeUnits_Deviator.setToggleState((unitType == Unit_Deviator));
    editorModeUnits_Saboteur.setToggleState((unitType == Unit_Saboteur));
    editorModeUnits_Sandworm.setToggleState((unitType == Unit_Sandworm));
    editorModeUnits_SpecialUnit.setToggleState((unitType == Unit_Special));
    editorModeUnits_Carryall.setToggleState((unitType == Unit_Carryall));
    editorModeUnits_Ornithopter.setToggleState((unitType == Unit_Ornithopter));

    if (unitType >= 0) {
        const auto house = static_cast<HOUSETYPE>(houseDropDownBox.getSelectedEntryIntData());
        pMapEditor_->setEditorMode(MapEditor::EditorMode(house, unitType, 256, static_cast<ANGLETYPE>(0), AREAGUARD));
    }
}

void MapEditorInterface::onStructureHealthDropDown(bool bInteractive) {

    if (bInteractive) {
        currentEditUnitID_ = INVALID;

        if (pMapEditor_->getSelectedStructureID() != currentEditStructureID_) {
            pMapEditor_->startOperation();
            currentEditStructureID_ = pMapEditor_->getSelectedStructureID();
        }

        const std::vector<int> selectedStructures = pMapEditor_->getMirrorStructures(currentEditStructureID_);

        for (const int selectedStructure : selectedStructures) {
            MapEditorEditStructureOperation editStructureOperation(
                selectedStructure, structureDetailsHealthDropDownBox.getSelectedEntryIntData());
            pMapEditor_->addUndoOperation(editStructureOperation.perform(pMapEditor_));
        }
    }
}

void MapEditorInterface::onUnitHealthDropDown(bool bInteractive) {

    if (bInteractive) {
        currentEditStructureID_ = INVALID;

        if (pMapEditor_->getSelectedUnitID() != currentEditUnitID_) {
            pMapEditor_->startOperation();
            currentEditUnitID_ = pMapEditor_->getSelectedUnitID();
        }

        const std::vector<int> selectedUnits = pMapEditor_->getMirrorUnits(currentEditUnitID_);

        for (const int selectedUnit : selectedUnits) {
            const MapEditor::Unit* pUnit = pMapEditor_->getUnit(selectedUnit);
            MapEditorEditUnitOperation editUnitOperation(
                pUnit->id_, unitDetailsHealthDropDownBox.getSelectedEntryIntData(), pUnit->angle_, pUnit->attack_mode_);
            pMapEditor_->addUndoOperation(editUnitOperation.perform(pMapEditor_));
        }
    }
}

void MapEditorInterface::onSelectedUnitRotateLeft() {
    onUnitRotateLeft(pMapEditor_->getSelectedUnitID());
}

void MapEditorInterface::onUnitRotateLeft(int unitID) {
    currentEditStructureID_ = INVALID;

    if (unitID != currentEditUnitID_) {
        pMapEditor_->startOperation();
        currentEditUnitID_ = unitID;
    }

    const std::vector<int> mirrorUnits = pMapEditor_->getMirrorUnits(unitID, true);
    for (int i = 0; i < static_cast<int>(mirrorUnits.size()); i++) {
        if (mirrorUnits[i] == INVALID) {
            continue;
        }

        const MapEditor::Unit* pMirrorUnit = pMapEditor_->getUnit(mirrorUnits[i]);

        auto currentAngle = pMirrorUnit->angle_;
        currentAngle      = pMapEditor_->getMapMirror()->getAngle(currentAngle, i);
        if (pMirrorUnit->itemID_ == Unit_Soldier || pMirrorUnit->itemID_ == Unit_Saboteur
            || pMirrorUnit->itemID_ == Unit_Trooper || pMirrorUnit->itemID_ == Unit_Infantry
            || pMirrorUnit->itemID_ == Unit_Troopers) {
            currentAngle = static_cast<ANGLETYPE>(static_cast<int>(currentAngle) + 2);
        } else {
            currentAngle = static_cast<ANGLETYPE>(static_cast<int>(currentAngle) + 1);
        }

        currentAngle = normalizeAngle(currentAngle);
        currentAngle = pMapEditor_->getMapMirror()->getAngle(currentAngle, i);

        MapEditorEditUnitOperation editUnitOperation(pMirrorUnit->id_, pMirrorUnit->health_, currentAngle,
                                                     pMirrorUnit->attack_mode_);

        pMapEditor_->addUndoOperation(editUnitOperation.perform(pMapEditor_));
    }
}

void MapEditorInterface::onSelectedUnitRotateRight() {
    onUnitRotateRight(pMapEditor_->getSelectedUnitID());
}

void MapEditorInterface::onUnitRotateRight(int unitID) {
    currentEditStructureID_ = INVALID;

    if (unitID != currentEditUnitID_) {
        pMapEditor_->startOperation();
        currentEditUnitID_ = unitID;
    }

    const std::vector<int> mirrorUnits = pMapEditor_->getMirrorUnits(unitID, true);
    for (int i = 0; i < static_cast<int>(mirrorUnits.size()); i++) {
        if (mirrorUnits[i] == INVALID) {
            continue;
        }

        const MapEditor::Unit* pMirrorUnit = pMapEditor_->getUnit(mirrorUnits[i]);

        auto currentAngle = pMirrorUnit->angle_;
        currentAngle      = pMapEditor_->getMapMirror()->getAngle(currentAngle, i);
        if (pMirrorUnit->itemID_ == Unit_Soldier || pMirrorUnit->itemID_ == Unit_Saboteur
            || pMirrorUnit->itemID_ == Unit_Trooper || pMirrorUnit->itemID_ == Unit_Infantry
            || pMirrorUnit->itemID_ == Unit_Troopers) {
            currentAngle = static_cast<ANGLETYPE>(static_cast<int>(currentAngle) - 2);
        } else {
            currentAngle = static_cast<ANGLETYPE>(static_cast<int>(currentAngle) - 1);
        }
        currentAngle = normalizeAngle(currentAngle);
        currentAngle = pMapEditor_->getMapMirror()->getAngle(currentAngle, i);

        MapEditorEditUnitOperation editUnitOperation(pMirrorUnit->id_, pMirrorUnit->health_, currentAngle,
                                                     pMirrorUnit->attack_mode_);

        pMapEditor_->addUndoOperation(editUnitOperation.perform(pMapEditor_));
    }
}

void MapEditorInterface::onUnitAttackModeDropDown(bool bInteractive) {

    if (bInteractive) {
        currentEditStructureID_ = INVALID;

        if (pMapEditor_->getSelectedUnitID() != currentEditUnitID_) {
            pMapEditor_->startOperation();
            currentEditUnitID_ = pMapEditor_->getSelectedUnitID();
        }

        const std::vector<int> selectedUnits = pMapEditor_->getMirrorUnits(currentEditUnitID_);

        for (const int selectedUnit : selectedUnits) {
            const MapEditor::Unit* pUnit = pMapEditor_->getUnit(selectedUnit);
            MapEditorEditUnitOperation editUnitOperation(
                pUnit->id_, pUnit->health_, pUnit->angle_,
                static_cast<ATTACKMODE>(unitDetailsAttackModeDropDownBox.getSelectedEntryIntData()));
            pMapEditor_->addUndoOperation(editUnitOperation.perform(pMapEditor_));
        }
    }
}

void MapEditorInterface::changeHouseDropDown(HOUSETYPE newHouse) {

    for (size_t i = 0; i < pMapEditor_->getPlayers().size(); i++) {
        if (pMapEditor_->getPlayers()[i].house_ == newHouse) {
            houseDropDownBox.setSelectedItem(i);
            break;
        }
    }
}

void MapEditorInterface::changeInterfaceColor(HOUSETYPE newHouse) {
    house_ = newHouse;
    color_ = SDL2RGB(dune::globals::palette[dune::globals::houseToPaletteIndex[static_cast<int>(newHouse)] + 3]);

    terrainButton.setTextColor(color_);
    structuresButton.setTextColor(color_);
    unitsButton.setTextColor(color_);

    houseDropDownBox.setColor(color_);

    auto* const gfx = dune::globals::pGFXManager.get();

    // top bar
    const auto* const pTopBarTexture = gfx->getUIGraphic(UI_TopBar, newHouse);
    topBar.setTexture(pTopBarTexture);
    // side bar
    const auto* const pSideBarTexture = gfx->getUIGraphic(UI_MapEditor_SideBar, newHouse);
    sideBar.setTexture(pSideBarTexture);

    // bottom bar
    const auto* const pBottomBarTexture = gfx->getUIGraphic(UI_MapEditor_BottomBar, newHouse);
    bottomBar.setTexture(pBottomBarTexture);

    editorModeTerrain_Sand.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Sand, newHouse));
    editorModeTerrain_Dunes.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Dunes, newHouse));
    editorModeTerrain_SpecialBloom.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_SpecialBloom, newHouse));
    editorModeTerrain_Spice.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Spice, newHouse));
    editorModeTerrain_ThickSpice.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_ThickSpice, newHouse));
    editorModeTerrain_SpiceBloom.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_SpiceBloom, newHouse));
    editorModeTerrain_Rock.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Rock, newHouse));
    editorModeTerrain_Mountain.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Mountain, newHouse));

    editorModeTerrain_Pen1x1.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Pen1x1, newHouse));
    editorModeTerrain_Pen3x3.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Pen3x3, newHouse));
    editorModeTerrain_Pen5x5.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Pen5x5, newHouse));

    editorModeClassicTerrain_SpiceBloom.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_SpiceBloom, newHouse));
    editorModeClassicTerrain_SpecialBloom.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_SpecialBloom, newHouse));
    editorModeClassicTerrain_SpiceField.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Spice, newHouse));
    editorModeClassicTerrain_SetTacticalPos.setTextColor(color_);

    editorModeStructs_Slab1.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Slab1, newHouse));
    editorModeStructs_Wall.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Wall, newHouse));
    editorModeStructs_GunTurret.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_GunTurret, newHouse));
    editorModeStructs_RocketTurret.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_RocketTurret, newHouse));
    editorModeStructs_ConstructionYard.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_ConstructionYard, newHouse));
    editorModeStructs_Windtrap.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Windtrap, newHouse));
    editorModeStructs_Radar.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Radar, newHouse));
    editorModeStructs_Silo.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Silo, newHouse));
    editorModeStructs_IX.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_IX, newHouse));
    editorModeStructs_Barracks.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Barracks, newHouse));
    editorModeStructs_WOR.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_WOR, newHouse));
    editorModeStructs_LightFactory.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_LightFactory, newHouse));
    editorModeStructs_Refinery.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Refinery, newHouse));
    editorModeStructs_HighTechFactory.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_HighTechFactory, newHouse));
    editorModeStructs_HeavyFactory.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_HeavyFactory, newHouse));
    editorModeStructs_RepairYard.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_RepairYard, newHouse));
    editorModeStructs_Starport.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Starport, newHouse));
    editorModeStructs_Palace.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Palace, newHouse));

    editorModeUnits_Soldier.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Soldier, newHouse));
    editorModeUnits_Trooper.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Trooper, newHouse));
    editorModeUnits_Harvester.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Harvester, newHouse));
    editorModeUnits_Infantry.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Infantry, newHouse));
    editorModeUnits_Troopers.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Troopers, newHouse));
    editorModeUnits_MCV.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_MCV, newHouse));
    editorModeUnits_Trike.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Trike, newHouse));
    editorModeUnits_Raider.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Raider, newHouse));
    editorModeUnits_Quad.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Quad, newHouse));
    editorModeUnits_Tank.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Tank, newHouse));
    editorModeUnits_SiegeTank.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_SiegeTank, newHouse));
    editorModeUnits_Launcher.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Launcher, newHouse));
    editorModeUnits_Devastator.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Devastator, newHouse));
    editorModeUnits_SonicTank.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_SonicTank, newHouse));
    editorModeUnits_Deviator.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Deviator, newHouse));
    editorModeUnits_Saboteur.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Saboteur, newHouse));
    editorModeUnits_Sandworm.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Sandworm, newHouse));
    editorModeUnits_SpecialUnit.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_SpecialUnit, newHouse));
    editorModeUnits_Carryall.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Carryall, newHouse));
    editorModeUnits_Ornithopter.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_Ornithopter, newHouse));

    structureDetailsHealthLabel.setTextColor(color_);
    structureDetailsHealthDropDownBox.setColor(color_);

    unitDetailsHealthLabel.setTextColor(color_);
    unitDetailsHealthDropDownBox.setColor(color_);

    unitDetailsAttackModeLabel.setTextColor(color_);
    unitDetailsAttackModeDropDownBox.setColor(color_);

    unitDetailsRotateLeftButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_RotateLeftIcon, newHouse),
                                          gfx->getUIGraphicSurface(UI_MapEditor_RotateLeftHighlightIcon, newHouse));
    unitDetailsRotateRightButton.setSymbol(gfx->getUIGraphicSurface(UI_MapEditor_RotateRightIcon, newHouse),
                                           gfx->getUIGraphicSurface(UI_MapEditor_RotateRightHighlightIcon, newHouse));
}

void MapEditorInterface::onMirrorModeButton(int mode) {
    pMapEditor_->setMirrorMode(static_cast<MirrorMode>(mode));

    mirrorModeNoneButton.setToggleState((mode == MirrorModeNone));
    mirrorModeHorizontalButton.setToggleState((mode == MirrorModeHorizontal));
    mirrorModeVerticalButton.setToggleState((mode == MirrorModeVertical));
    mirrorModeBothButton.setToggleState((mode == MirrorModeBoth));
    mirrorModePointButton.setToggleState((mode == MirrorModePoint));
}
