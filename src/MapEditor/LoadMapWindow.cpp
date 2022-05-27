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

#include <MapEditor/LoadMapWindow.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/INIFile.h>
#include <FileClasses/TextManager.h>

#include <GUI/GUIStyle.h>
#include <GUI/QstBox.h>
#include <GUI/Spacer.h>
#include <GUI/dune/DuneStyle.h>

#include <fmt/printf.h>
#include <misc/FileSystem.h>
#include <misc/draw_util.h>
#include <misc/fnkdat.h>

#include <INIMap/INIMapPreviewCreator.h>

#include <globals.h>

LoadMapWindow::LoadMapWindow(uint32_t color) : Window(0, 0, 0, 0), color(color) {

    // set up window
    const auto* const pBackground = dune::globals::pGFXManager->getUIGraphic(UI_NewMapWindow);
    setBackground(pBackground);

    LoadMapWindow::setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    LoadMapWindow::setWindowWidget(&mainHBox);

    mainHBox.addWidget(Widget::create<HSpacer>(16).release());
    mainHBox.addWidget(&mainVBox);
    mainHBox.addWidget(Widget::create<HSpacer>(16).release());

    titleLabel.setTextColor(COLOR_LIGHTYELLOW, COLOR_TRANSPARENT);
    titleLabel.setAlignment(static_cast<Alignment_Enum>(Alignment_HCenter | Alignment_VCenter));
    titleLabel.setText(_("Load Map"));
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(Widget::create<VSpacer>(22).release());

    mainVBox.addWidget(&centralHBox, 346);

    centralHBox.addWidget(&leftVBox, 0.8);

    leftVBox.addWidget(&mapTypeButtonsHBox, 24);

    singleplayerUserMapsButton.setText(_("SP User Maps"));
    singleplayerUserMapsButton.setTextColor(color);
    singleplayerUserMapsButton.setToggleButton(true);
    singleplayerUserMapsButton.setOnClick([this] { onMapTypeChange(0); });
    mapTypeButtonsHBox.addWidget(&singleplayerUserMapsButton);

    multiplayerUserMapsButton.setText(_("MP User Maps"));
    multiplayerUserMapsButton.setTextColor(color);
    multiplayerUserMapsButton.setToggleButton(true);
    multiplayerUserMapsButton.setOnClick([this] { onMapTypeChange(1); });
    mapTypeButtonsHBox.addWidget(&multiplayerUserMapsButton);

    mapTypeButtonsHBox.addWidget(Widget::create<Spacer>().release(), 5.0);
    mapList.setColor(color);
    mapList.setAutohideScrollbar(false);
    mapList.setOnSelectionChange([this](auto flag) { onMapListSelectionChange(flag); });
    mapList.setOnDoubleClick([this] { onLoad(); });
    leftVBox.addWidget(&mapList, 0.95);

    leftVBox.addWidget(Widget::create<VSpacer>(10).release());

    centralHBox.addWidget(Widget::create<HSpacer>(8).release());
    centralHBox.addWidget(Widget::create<Spacer>().release(), 0.05);

    centralHBox.addWidget(&rightVBox, 180);
    minimap.setSurface(GUIStyle::getInstance().createButtonSurface(130, 130, _("Choose map"), true, false));
    rightVBox.addWidget(&minimap);

    rightVBox.addWidget(Widget::create<VSpacer>(10).release());
    rightVBox.addWidget(&mapPropertiesHBox, 0.01);
    mapPropertiesHBox.addWidget(&mapPropertyNamesVBox, 75);
    mapPropertiesHBox.addWidget(&mapPropertyValuesVBox, 105);
    mapPropertyNamesVBox.addWidget(Label::create(_("Size") + ":", color).release());
    mapPropertySize.setTextColor(color);
    mapPropertyValuesVBox.addWidget(&mapPropertySize);
    mapPropertyNamesVBox.addWidget(Label::create(_("Players") + ":", color).release());
    mapPropertyPlayers.setTextColor(color);
    mapPropertyValuesVBox.addWidget(&mapPropertyPlayers);
    mapPropertyNamesVBox.addWidget(Label::create(_("Author") + ":", color).release());
    mapPropertyAuthors.setTextColor(color);
    mapPropertyValuesVBox.addWidget(&mapPropertyAuthors);
    mapPropertyNamesVBox.addWidget(Label::create(_("License") + ":", color).release());
    mapPropertyLicense.setTextColor(color);
    mapPropertyValuesVBox.addWidget(&mapPropertyLicense);
    rightVBox.addWidget(Widget::create<Spacer>().release());

    mainVBox.addWidget(Widget::create<VSpacer>(5).release());

    mainVBox.addWidget(&buttonHBox);

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick([this] { onCancel(); });

    buttonHBox.addWidget(&cancelButton);

    buttonHBox.addWidget(Widget::create<HSpacer>(8).release());

    buttonHBox.addWidget(Widget::create<Spacer>().release());

    buttonHBox.addWidget(Widget::create<HSpacer>(8).release());

    loadButton.setText(_("Load"));
    loadButton.setTextColor(color);
    loadButton.setOnClick([this] { onLoad(); });

    buttonHBox.addWidget(&loadButton);

    mainVBox.addWidget(Widget::create<VSpacer>(10).release());

    onMapTypeChange(0);
}

bool LoadMapWindow::handleKeyPress(const SDL_KeyboardEvent& key) {
    if (pChildWindow_ != nullptr) {
        const bool ret = pChildWindow_->handleKeyPress(key);
        return ret;
    }

    if (isEnabled() && (pWindowWidget_ != nullptr)) {
        if (key.keysym.sym == SDLK_RETURN) {
            onLoad();
            return true;
        }
        if (key.keysym.sym == SDLK_DELETE) {

            const int index = mapList.getSelectedIndex();

            if (index >= 0) {

                QstBox* pQstBox = QstBox::create(
                    fmt::sprintf(_("Do you really want to delete '%s' ?"), mapList.getEntry(index).c_str()),

                    _("Yes"),

                    _("No"),

                    QSTBOX_BUTTON1);

                pQstBox->setTextColor(color);

                openWindow(pQstBox);
            }

            return true;
        }
        return pWindowWidget_->handleKeyPress(key);
    }
    return false;
}

void LoadMapWindow::onChildWindowClose(Window* pChildWindow) {
    const auto* pQstBox = dynamic_cast<QstBox*>(pChildWindow);
    if (pQstBox == nullptr)
        return;

    if (pQstBox->getPressedButtonID() != QSTBOX_BUTTON1)
        return;

    const int index = mapList.getSelectedIndex();
    if (index < 0)
        return;

    const auto file2delete = currentMapDirectory / (mapList.getSelectedEntry() + ".ini");

    std::error_code ec;
    if (!std::filesystem::remove(file2delete, ec))
        return;

    // remove was successful => delete from list
    mapList.removeEntry(index);
    if (mapList.getNumEntries() > 0) {
        if (index >= mapList.getNumEntries()) {
            mapList.setSelectedItem(mapList.getNumEntries() - 1);
        } else {
            mapList.setSelectedItem(index);
        }
    }
}

void LoadMapWindow::onCancel() {
    auto* pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void LoadMapWindow::onLoad() {
    if (mapList.getSelectedIndex() < 0) {
        return;
    }

    loadMapname         = mapList.getSelectedEntry();
    loadMapFilepath     = currentMapDirectory / (loadMapname + ".ini");
    loadMapSingleplayer = singleplayerUserMapsButton.getToggleState();
    getCaseInsensitiveFilename(loadMapFilepath);

    auto* pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void LoadMapWindow::onMapTypeChange(int buttonID) {
    singleplayerUserMapsButton.setToggleState(buttonID == 0);
    multiplayerUserMapsButton.setToggleState(buttonID == 1);

    switch (buttonID) {
        case 0: {
            auto [ok, tmp]      = fnkdat("maps/singleplayer/", FNKDAT_USER | FNKDAT_CREAT);
            currentMapDirectory = tmp;
        } break;
        case 1: {
            auto [ok, tmp]      = fnkdat("maps/multiplayer/", FNKDAT_USER | FNKDAT_CREAT);
            currentMapDirectory = tmp;
        } break;
    }

    mapList.clearAllEntries();

    for (const auto& filename :
         getFileNamesList(currentMapDirectory, "ini", true, FileListOrder_Name_CaseInsensitive_Asc)) {
        mapList.addEntry(reinterpret_cast<const char*>(filename.stem().u8string().c_str()));
    }

    if (mapList.getNumEntries() > 0) {
        mapList.setSelectedItem(0);
    } else {
        minimap.setSurface(GUIStyle::getInstance().createButtonSurface(130, 130, _("No map available"), true, false));
        mapPropertySize.setText("");
        mapPropertyPlayers.setText("");
        mapPropertyAuthors.setText("");
        mapPropertyLicense.setText("");
    }
}

void LoadMapWindow::onMapListSelectionChange(bool bInteractive) {
    loadButton.setEnabled(true);

    if (mapList.getSelectedIndex() < 0) {
        return;
    }

    auto mapFilename = currentMapDirectory / (mapList.getSelectedEntry() + ".ini");
    getCaseInsensitiveFilename(mapFilename);

    INIFile inimap(mapFilename);

    int sizeX = 0;
    int sizeY = 0;

    if (inimap.hasKey("MAP", "Seed")) {
        // old map format with seed value
        const int mapscale = inimap.getIntValue("BASIC", "MapScale", -1);

        switch (mapscale) {
            case 0: {
                sizeX = 62;
                sizeY = 62;
            } break;

            case 1: {
                sizeX = 32;
                sizeY = 32;
            } break;

            case 2: {
                sizeX = 21;
                sizeY = 21;
            } break;

            default: {
                sizeX = 64;
                sizeY = 64;
            }
        }
    } else {
        // new map format with saved map
        sizeX = inimap.getIntValue("MAP", "SizeX", 0);
        sizeY = inimap.getIntValue("MAP", "SizeY", 0);
    }

    mapPropertySize.setText(std::to_string(sizeX) + " x " + std::to_string(sizeY));

    sdl2::surface_ptr pMapSurface;
    try {
        INIMapPreviewCreator mapPreviewCreator(&inimap);
        pMapSurface = mapPreviewCreator.createMinimapImageOfMap(1, DuneStyle::buttonBorderColor);
    } catch (...) {
        pMapSurface = GUIStyle::getInstance().createButtonSurface(130, 130, "Error", true, false);
        loadButton.setEnabled(false);
    }
    minimap.setSurface(std::move(pMapSurface));

    int numPlayers = 0;
    if (inimap.hasSection("Atreides"))
        numPlayers++;
    if (inimap.hasSection("Ordos"))
        numPlayers++;
    if (inimap.hasSection("Harkonnen"))
        numPlayers++;
    if (inimap.hasSection("Fremen"))
        numPlayers++;
    if (inimap.hasSection("Mercenary"))
        numPlayers++;
    if (inimap.hasSection("Sardaukar"))
        numPlayers++;
    if (inimap.hasSection("Player1"))
        numPlayers++;
    if (inimap.hasSection("Player2"))
        numPlayers++;
    if (inimap.hasSection("Player3"))
        numPlayers++;
    if (inimap.hasSection("Player4"))
        numPlayers++;
    if (inimap.hasSection("Player5"))
        numPlayers++;
    if (inimap.hasSection("Player6"))
        numPlayers++;

    mapPropertyPlayers.setText(std::to_string(numPlayers));

    auto authors = inimap.getStringValue("BASIC", "Author", "-");
    if (authors.size() > 11) {
        authors = authors.substr(0, 9) + "...";
    }
    mapPropertyAuthors.setText(authors);

    mapPropertyLicense.setText(inimap.getStringValue("BASIC", "License", "-"));
}
