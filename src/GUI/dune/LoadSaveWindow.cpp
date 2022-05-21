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

#include <GUI/MsgBox.h>
#include <GUI/QstBox.h>
#include <GUI/Spacer.h>

#include <fmt/printf.h>

#include <filesystem>
#include <string>
#include <utility>

namespace {
inline constexpr auto invalid_chars = "?*:|<>/\\\"'`";
}

LoadSaveWindow::LoadSaveWindow(bool bSave, const std::string& caption,
                               const std::vector<std::filesystem::path>& directories,
                               const std::vector<std::string>& directoryTitles, std::string extension,
                               int preselectedDirectoryIndex, const std::string& preselectedFile, uint32_t color)
    : Window(0, 0, 0, 0), bSaveWindow(bSave), directories(directories), directoryTitles(directoryTitles),
      extension(std::move(extension)), currentDirectoryIndex(preselectedDirectoryIndex),
      preselectedFile(preselectedFile), color(color) {

    // set up window
    const auto* pBackground = dune::globals::pGFXManager->getUIGraphic(UI_LoadSaveWindow);
    setBackground(pBackground);

    LoadSaveWindow::setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    LoadSaveWindow::setWindowWidget(&mainHBox);

    mainHBox.addWidget(HSpacer::create(16));
    mainHBox.addWidget(&mainVBox);
    mainHBox.addWidget(HSpacer::create(16));

    titleLabel.setTextColor(COLOR_LIGHTYELLOW, COLOR_TRANSPARENT);
    titleLabel.setAlignment(static_cast<Alignment_Enum>(Alignment_HCenter | Alignment_VCenter));
    titleLabel.setText(caption);
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(VSpacer::create(8));

    if (directories.size() > 1) {
        directoryButtons.reserve(directories.size());

        for (const auto& title : directoryTitles) {
            auto& button = directoryButtons.emplace_back();

            button.setText(title);
            button.setTextColor(color);
            button.setToggleButton(true);
            button.setOnClick([this, i = static_cast<int>(directoryButtons.size() - 1)] { onDirectoryChange(i); });

            directoryHBox.addWidget(&button);
        }

        mainVBox.addWidget(&directoryHBox, 20);
    }

    mainVBox.addWidget(&fileListHBox, (bSave ? 121 : 151) - (directories.size() > 1 ? 20 : 0));
    fileList.setColor(color);
    fileList.setOnSelectionChange([this](bool bInteractive) { onSelectionChange(bInteractive); });
    fileList.setOnDoubleClick([this] { onOK(); });
    fileListHBox.addWidget(&fileList);

    mainVBox.addWidget(VSpacer::create(5));

    if (bSave) {
        saveName.setTextColor(color);
        mainVBox.addWidget(&saveName);
        saveName.setMaximumTextLength(64);
        saveName.setForbiddenChars(invalid_chars);
        mainVBox.addWidget(VSpacer::create(5));
    }

    mainVBox.addWidget(&buttonHBox);

    okButton.setText(_(bSave ? "Save" : "Load"));
    okButton.setTextColor(color);
    okButton.setOnClick([this] { onOK(); });

    buttonHBox.addWidget(&okButton);

    buttonHBox.addWidget(HSpacer::create(8));

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick([this] { onCancel(); });

    buttonHBox.addWidget(&cancelButton);

    mainVBox.addWidget(VSpacer::create(10));

    if (directories.size() > 1) {
        onDirectoryChange(currentDirectoryIndex);
    } else {
        updateEntries();
    }

    if (bSaveWindow && (fileList.getSelectedIndex() < 0)) {
        saveName.setText(preselectedFile);
        saveName.setActive();
    }

    this->preselectedFile.clear();
}

LoadSaveWindow::~LoadSaveWindow() {
    fileList.setOnSelectionChange({});
}

void LoadSaveWindow::updateEntries() {
    fileList.clearAllEntries();

    int preselectedFileIndex = -1;
    for (const auto& fileName :
         getFileNamesList(directories[currentDirectoryIndex], extension, true, FileListOrder_ModifyDate_Dsc)) {
        const std::string entryName{reinterpret_cast<const char*>(fileName.stem().u8string().c_str())};

        fileList.addEntry(entryName);

        if (entryName == preselectedFile) {
            preselectedFileIndex = fileList.getNumEntries() - 1;
        }
    }

    if (preselectedFileIndex >= 0) {
        fileList.setSelectedItem(preselectedFileIndex);
    }
}

bool LoadSaveWindow::handleKeyPress(const SDL_KeyboardEvent& key) {
    if (pChildWindow != nullptr) {
        const auto ret = pChildWindow->handleKeyPress(key);
        return ret;
    }

    if (isEnabled() && (pWindowWidget != nullptr)) {
        if (key.keysym.sym == SDLK_RETURN) {
            onOK();
            return true;
        }
        if (key.keysym.sym == SDLK_DELETE) {

            const auto index = fileList.getSelectedIndex();

            if (index >= 0) {

                auto* pQstBox = QstBox::create(
                    fmt::sprintf(_("Do you really want to delete '%s' ?"), fileList.getEntry(index).c_str()), _("Yes"),
                    _("No"), QSTBOX_BUTTON1);

                pQstBox->setTextColor(color);

                openWindow(pQstBox);
            }

            return true;
        }
        return pWindowWidget->handleKeyPress(key);
    }
    return false;
}

void LoadSaveWindow::onChildWindowClose(Window* pChildWindow) {
    const auto* pQstBox = dynamic_cast<QstBox*>(pChildWindow);
    if (pQstBox == nullptr || pQstBox->getPressedButtonID() != QSTBOX_BUTTON1)
        return;

    const int index = fileList.getSelectedIndex();
    if (index < 0)
        return;

    const auto file2delete = directories[currentDirectoryIndex] / (fileList.getEntry(index) + "." + extension);

    if (std::filesystem::remove(file2delete) != 0)
        return;

    // remove was successful => delete from list
    fileList.removeEntry(index);
    if (fileList.getNumEntries() > 0) {
        if (index >= fileList.getNumEntries()) {
            fileList.setSelectedItem(fileList.getNumEntries() - 1);
        } else {
            fileList.setSelectedItem(index);
        }
    }
}

std::unique_ptr<LoadSaveWindow>
LoadSaveWindow::create(bool bSave, const std::string& caption, const std::filesystem::path& directory,
                       const std::string& extension, const std::string& preselectedFile, Uint32 color) {
    std::vector<std::filesystem::path> directories;
    directories.push_back(directory);
    std::vector<std::string> directoryTitles;
    directoryTitles.emplace_back();

    return create(bSave, caption, directories, directoryTitles, extension, 0, preselectedFile, color);
}

std::unique_ptr<LoadSaveWindow>
LoadSaveWindow::create(bool bSave, const std::string& caption, const std::vector<std::filesystem::path>& directories,
                       const std::vector<std::string>& directoryTitles, const std::string& extension,
                       int preselectedDirectoryIndex, const std::string& preselectedFile, Uint32 color) {
    std::unique_ptr<LoadSaveWindow> dlg{new LoadSaveWindow(bSave, caption, directories, directoryTitles, extension,
                                                           preselectedDirectoryIndex, preselectedFile, color)};
    dlg->pAllocated = true;
    return dlg;
}

void LoadSaveWindow::onOK() {
    if (!bSaveWindow) {
        const auto index = fileList.getSelectedIndex();
        if (index >= 0) {
            filename = directories[currentDirectoryIndex] / (fileList.getEntry(index) + "." + extension);
            filename = filename.lexically_normal().native();

            auto* const pParentWindow = dynamic_cast<Window*>(getParent());
            if (pParentWindow != nullptr) {
                pParentWindow->closeChildWindow();
            }
        }
    } else {
        const auto savename = saveName.getText();

        if (!savename.empty() && savename.find_first_of(invalid_chars) == std::string::npos) {
            filename = directories[currentDirectoryIndex] / fmt::format("{}.{}", saveName.getText(), extension);

            auto* const pParentWindow = dynamic_cast<Window*>(getParent());
            if (pParentWindow != nullptr) {
                pParentWindow->closeChildWindow();
            }
        } else {
            openWindow(
                MsgBox::create(_("Invalid file name! File names must not contain \\ or / and must not be empty!")));
        }
    }
}

void LoadSaveWindow::onCancel() {
    auto* const pParentWindow = dynamic_cast<Window*>(getParent());
    if (pParentWindow != nullptr) {
        pParentWindow->closeChildWindow();
    }
}

void LoadSaveWindow::onDirectoryChange(int i) {
    currentDirectoryIndex = i;
    for (auto j = 0; j < directoryButtons.size(); j++) {
        directoryButtons[j].setToggleState(i == j);
    }

    updateEntries();
}

void LoadSaveWindow::onSelectionChange(bool bInteractive) {
    if (!bSaveWindow)
        return;

    const int index = fileList.getSelectedIndex();
    if (index >= 0) {
        saveName.setText(fileList.getEntry(index));
    }
}
