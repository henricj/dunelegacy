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

#include <GUI/dune/LoadSaveWindow.h>

#include <globals.h>

#include <FileClasses/GFXManager.h>
#include <FileClasses/TextManager.h>
#include <misc/FileSystem.h>

#include <GUI/Spacer.h>
#include <GUI/QstBox.h>
#include <GUI/MsgBox.h>

#include <misc/format.h>

#include <stdio.h>

LoadSaveWindow::LoadSaveWindow(bool bSave, const std::string& caption, const std::vector<std::string>& directories, const std::vector<std::string>& directoryTitles, const std::string& extension, int preselectedDirectoryIndex, const std::string& preselectedFile, Uint32 color)
 : Window(0,0,0,0), bSaveWindow(bSave), directories(directories), directoryTitles(directoryTitles), extension(extension), currentDirectoryIndex(preselectedDirectoryIndex), preselectedFile(preselectedFile), color(color) {

    // set up window
    SDL_Texture *pBackground = pGFXManager->getUIGraphic(UI_LoadSaveWindow);
    setBackground(pBackground);

    setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    setWindowWidget(&mainHBox);

    mainHBox.addWidget(HSpacer::create(16));
    mainHBox.addWidget(&mainVBox);
    mainHBox.addWidget(HSpacer::create(16));

    titleLabel.setTextColor(COLOR_LIGHTYELLOW, COLOR_TRANSPARENT);
    titleLabel.setAlignment((Alignment_Enum) (Alignment_HCenter | Alignment_VCenter));
    titleLabel.setText(caption);
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(VSpacer::create(8));

    if(directories.size() > 1) {
        directoryButtons.resize(directories.size());

        for(size_t i=0;i<directories.size(); i++) {
            directoryButtons[i].setText(directoryTitles[i]);
            directoryButtons[i].setTextColor(color);
            directoryButtons[i].setToggleButton(true);
            directoryButtons[i].setOnClick(std::bind(&LoadSaveWindow::onDirectoryChange, this, i));

            directoryHBox.addWidget(&directoryButtons[i]);
        }

        mainVBox.addWidget(&directoryHBox, 20);
    }

    mainVBox.addWidget(&fileListHBox, (bSave ? 120 : 150) - (directories.size() > 1 ? 20 : 0));
    fileList.setColor(color);
    fileList.setOnSelectionChange(std::bind(&LoadSaveWindow::onSelectionChange, this, std::placeholders::_1));
    fileList.setOnDoubleClick(std::bind(&LoadSaveWindow::onOK, this));
    fileListHBox.addWidget(&fileList);

    mainVBox.addWidget(VSpacer::create(5));

    if(bSave == true) {
        saveName.setTextColor(color);
        mainVBox.addWidget(&saveName);
        saveName.setMaximumTextLength(64);
        saveName.setForbiddenChars("?*:|<>/\\\"");
        mainVBox.addWidget(VSpacer::create(5));
    }

    mainVBox.addWidget(&buttonHBox);

    okButton.setText(_(bSave ? "Save" : "Load"));
    okButton.setTextColor(color);
    okButton.setOnClick(std::bind(&LoadSaveWindow::onOK, this));

    buttonHBox.addWidget(&okButton);

    buttonHBox.addWidget(HSpacer::create(8));

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick(std::bind(&LoadSaveWindow::onCancel, this));

    buttonHBox.addWidget(&cancelButton);

    mainVBox.addWidget(VSpacer::create(10));

    if(directories.size() > 1) {
        onDirectoryChange(currentDirectoryIndex);
    } else {
        updateEntries();
    }

    if(bSaveWindow && (fileList.getSelectedIndex() < 0)) {
        saveName.setText(preselectedFile);
        saveName.setActive();
    }

    this->preselectedFile = "";
}

LoadSaveWindow::~LoadSaveWindow() {
    fileList.setOnSelectionChange(std::function<void (bool)>());
}

void LoadSaveWindow::updateEntries() {
    fileList.clearAllEntries();

    int preselectedFileIndex = -1;
    for(const std::string fileName : getFileNamesList(directories[currentDirectoryIndex],extension, true, FileListOrder_ModifyDate_Dsc)) {
        std::string entryName = fileName.substr(0, fileName.length() - extension.length() - 1);
        fileList.addEntry(entryName);

        if(entryName == preselectedFile) {
            preselectedFileIndex = fileList.getNumEntries()-1;
        }
    }

    if(preselectedFileIndex >= 0) {
        fileList.setSelectedItem(preselectedFileIndex);
    }
}

bool LoadSaveWindow::handleKeyPress(SDL_KeyboardEvent& key) {
    if(pChildWindow != nullptr) {
        bool ret = pChildWindow->handleKeyPress(key);
        return ret;
    }

    if(isEnabled() && (pWindowWidget != nullptr)) {
        if(key.keysym.sym == SDLK_RETURN) {
            onOK();
            return true;
        } else if(key.keysym.sym == SDLK_DELETE) {
            int index = fileList.getSelectedIndex();
            if(index >= 0) {
                QstBox* pQstBox = QstBox::create(   fmt::sprintf(_("Do you really want to delete '%s' ?"), fileList.getEntry(index).c_str()),
                                                    _("Yes"),
                                                    _("No"),
                                                    QSTBOX_BUTTON1);

                pQstBox->setTextColor(color);

                openWindow(pQstBox);
            }

            return true;
        } else {
            return pWindowWidget->handleKeyPress(key);
        }
    } else {
        return false;
    }
}


void LoadSaveWindow::onChildWindowClose(Window* pChildWindow) {
    QstBox* pQstBox = dynamic_cast<QstBox*>(pChildWindow);
    if(pQstBox == nullptr || pQstBox->getPressedButtonID() != QSTBOX_BUTTON1) return;

    int index = fileList.getSelectedIndex();
    if(index < 0) return;

    const auto file2delete = directories[currentDirectoryIndex] + fileList.getEntry(index) + "." + extension;

    if(remove(file2delete.c_str()) != 0) return;

    // remove was successful => delete from list
    fileList.removeEntry(index);
    if(fileList.getNumEntries() > 0) {
        if(index >= fileList.getNumEntries()) {
            fileList.setSelectedItem(fileList.getNumEntries() - 1);
        } else {
            fileList.setSelectedItem(index);
        }
    }
}


void LoadSaveWindow::onOK() {
    if(bSaveWindow == false) {
        int index = fileList.getSelectedIndex();
        if(index >= 0) {
            filename = directories[currentDirectoryIndex] + fileList.getEntry(index) + "." + extension;

            Window* pParentWindow = dynamic_cast<Window*>(getParent());
            if(pParentWindow != nullptr) {
                pParentWindow->closeChildWindow();
            }
        }
    } else {
        std::string savename = saveName.getText();

        if(savename != "" && savename.find_first_of("\\/") == std::string::npos) {
            filename = directories[currentDirectoryIndex] + saveName.getText() + "." + extension;

            Window* pParentWindow = dynamic_cast<Window*>(getParent());
            if(pParentWindow != nullptr) {
                pParentWindow->closeChildWindow();
            }
        } else {
            openWindow(MsgBox::create(_("Invalid file name! File names must not contain \\ or / and must not be empty!")));
        }
    }
}

void LoadSaveWindow::onCancel() {
    Window* pParentWindow = dynamic_cast<Window*>(getParent());
    if(pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void LoadSaveWindow::onDirectoryChange(int i) {
    currentDirectoryIndex = i;
    for(int j = 0; j < (int) directoryButtons.size(); j++) {
        directoryButtons[j].setToggleState( (i==j) );
    }

    updateEntries();
}

void LoadSaveWindow::onSelectionChange(bool bInteractive) {
    if(bSaveWindow != true) return;

    int index = fileList.getSelectedIndex();
    if(index >= 0) {
        saveName.setText(fileList.getEntry(index));
    }
}

