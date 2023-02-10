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

#include <gsl/gsl>

#include <filesystem>
#include <string>
#include <utility>

namespace {
inline constexpr auto invalid_chars = "?*:|<>/\\\"'`";
}

LoadSaveWindow::LoadSaveWindow(bool bSave, std::string caption, std::vector<std::filesystem::path> directories,
                               std::vector<std::string> directoryTitles, std::string extension,
                               int preselectedDirectoryIndex, std::string preselectedFile, uint32_t color)
    : Window(0, 0, 0, 0), bSaveWindow_(bSave), directories_(std::move(directories)),
      directoryTitles_(std::move(directoryTitles)), extension_(std::move(extension)),
      currentDirectoryIndex_(preselectedDirectoryIndex), preselectedFile_(preselectedFile), color_(color) {

    // set up window
    const auto* pBackground = dune::globals::pGFXManager->getUIGraphic(UI_LoadSaveWindow);
    setBackground(pBackground);

    LoadSaveWindow::setCurrentPosition(calcAlignedDrawingRect(pBackground, HAlign::Center, VAlign::Center));

    LoadSaveWindow::setWindowWidget(&mainHBox);

    mainHBox.addWidget(Widget::create<HSpacer>(16).release());
    mainHBox.addWidget(&mainVBox);
    mainHBox.addWidget(Widget::create<HSpacer>(16).release());

    titleLabel.setTextColor(COLOR_LIGHTYELLOW, COLOR_TRANSPARENT);
    titleLabel.setAlignment(static_cast<Alignment_Enum>(Alignment_HCenter | Alignment_VCenter));
    titleLabel.setText(std::move(caption));
    mainVBox.addWidget(&titleLabel);

    mainVBox.addWidget(Widget::create<VSpacer>(8).release());

    if (directories_.size() > 1) {
        directoryButtons.reserve(directories_.size());

        for (const auto& title : directoryTitles_) {
            auto& button = directoryButtons.emplace_back();

            button.setText(title);
            button.setTextColor(color);
            button.setToggleButton(true);
            button.setOnClick([this, i = static_cast<int>(directoryButtons.size() - 1)] { onDirectoryChange(i); });

            directoryHBox.addWidget(&button);
        }

        mainVBox.addWidget(&directoryHBox, 20);
    }

    mainVBox.addWidget(&fileListHBox, (bSave ? 121 : 151) - (directories_.size() > 1 ? 20 : 0));
    fileList.setColor(color);
    fileList.setOnSelectionChange([this](bool bInteractive) { onSelectionChange(bInteractive); });
    fileList.setOnDoubleClick([this] { onOK(); });
    fileListHBox.addWidget(&fileList);

    mainVBox.addWidget(Widget::create<VSpacer>(5).release());

    if (bSave) {
        saveName.setTextColor(color);
        mainVBox.addWidget(&saveName);
        saveName.setMaximumTextLength(64);
        saveName.setForbiddenChars(invalid_chars);
        mainVBox.addWidget(Widget::create<VSpacer>(5).release());
    }

    mainVBox.addWidget(&buttonHBox);

    okButton.setText(_(bSave ? "Save" : "Load"));
    okButton.setTextColor(color);
    okButton.setOnClick([this] { onOK(); });

    buttonHBox.addWidget(&okButton);

    buttonHBox.addWidget(Widget::create<HSpacer>(8).release());

    cancelButton.setText(_("Cancel"));
    cancelButton.setTextColor(color);
    cancelButton.setOnClick([this] { onCancel(); });

    buttonHBox.addWidget(&cancelButton);

    mainVBox.addWidget(Widget::create<VSpacer>(10).release());

    if (directories_.size() > 1) {
        onDirectoryChange(currentDirectoryIndex_);
    } else {
        updateEntries();
    }

    if (bSaveWindow_ && !fileList.isSelected()) {
        saveName.setText(preselectedFile);
        saveName.setActive();
    }

    this->preselectedFile_.clear();
}

LoadSaveWindow::~LoadSaveWindow() {
    fileList.setOnSelectionChange({});
}

void LoadSaveWindow::updateEntries() {
    fileList.clearAllEntries();

    auto preselectedFileIndex = ListBox::invalid_index;
    for (const auto& fileName :
         getFileNamesList(directories_[currentDirectoryIndex_], extension_, true, FileListOrder_ModifyDate_Dsc)) {
        const auto entryName{(fileName.stem().string())};

        fileList.addEntry(entryName);

        if (entryName == preselectedFile_) {
            preselectedFileIndex = fileList.getNumEntries() - 1;
        }
    }

    if (preselectedFileIndex != ListBox::invalid_index) {
        fileList.setSelectedItem(preselectedFileIndex);
    }
}

bool LoadSaveWindow::handleKeyPress(const SDL_KeyboardEvent& key) {
    if (pChildWindow_ != nullptr) {
        const auto ret = pChildWindow_->handleKeyPress(key);
        return ret;
    }

    if (isEnabled() && (pWindowWidget_ != nullptr)) {
        if (key.keysym.sym == SDLK_RETURN) {
            onOK();
            return true;
        }
        if (key.keysym.sym == SDLK_DELETE) {
            if (!fileList.isSelected())
                return true;

            auto* const pQstBox =
                QstBox::create(fmt::sprintf(_("Do you really want to delete '%s' ?"), fileList.getSelectedEntry()),
                               _("Yes"), _("No"), QSTBOX_BUTTON1);

            pQstBox->setTextColor(color_);

            openWindow(pQstBox);

            return true;
        }
        return pWindowWidget_->handleKeyPress(key);
    }
    return false;
}

void LoadSaveWindow::onChildWindowClose(Window* pChildWindow) {
    const auto* pQstBox = dynamic_cast<QstBox*>(pChildWindow);
    if (pQstBox == nullptr || pQstBox->getPressedButtonID() != QSTBOX_BUTTON1)
        return;

    const auto index = fileList.getSelectedIndex();
    if (!fileList.isValid(index))
        return;

    const auto file2delete = directories_[currentDirectoryIndex_] / (fileList.getEntry(index) + "." + extension_);

    if (std::filesystem::remove(file2delete) != 0)
        return;

    // remove was successful => delete from list
    fileList.removeEntry(index);
    const auto num_entries = fileList.getNumEntries();
    if (num_entries > 0) {
        if (index >= num_entries) {
            fileList.setSelectedItem(num_entries - 1);
        } else {
            fileList.setSelectedItem(index);
        }
    }
}

std::unique_ptr<LoadSaveWindow>
LoadSaveWindow::create(bool bSave, std::string caption, std::filesystem::path directory, std::string extension,
                       std::string preselectedFile, Uint32 color) {
    std::vector<std::filesystem::path> directories;
    directories.push_back(std::move(directory));
    std::vector<std::string> directoryTitles;
    directoryTitles.emplace_back();

    return create(bSave, std::move(caption), std::move(directories), std::move(directoryTitles), std::move(extension),
                  0, std::move(preselectedFile), color);
}

std::unique_ptr<LoadSaveWindow>
LoadSaveWindow::create(bool bSave, std::string caption, std::vector<std::filesystem::path> directories,
                       std::vector<std::string> directoryTitles, std::string extension, int preselectedDirectoryIndex,
                       std::string preselectedFile, Uint32 color) {
    std::unique_ptr<LoadSaveWindow> dlg{
        new LoadSaveWindow(bSave, std::move(caption), std::move(directories), std::move(directoryTitles),
                           std::move(extension), preselectedDirectoryIndex, std::move(preselectedFile), color)};
    dlg->pAllocated_ = true;
    return dlg;
}

void LoadSaveWindow::onOK() {
    if (!bSaveWindow_) {
        const auto index = fileList.getSelectedIndex();
        if (!fileList.isValid(index))
            return;

        filename_ = directories_[currentDirectoryIndex_] / (fileList.getEntry(index) + "." + extension_);
        filename_ = filename_.lexically_normal().native();

        auto* const pParentWindow = dynamic_cast<Window*>(getParent());
        if (pParentWindow != nullptr)
            pParentWindow->closeChildWindow();
    } else {
        const auto savename = saveName.getText();

        if (!savename.empty() && savename.find_first_of(invalid_chars) == std::string::npos) {
            filename_ = directories_[currentDirectoryIndex_] / fmt::format("{}.{}", saveName.getText(), extension_);

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
    currentDirectoryIndex_ = i;
    for (auto j = 0U; j < directoryButtons.size(); ++j) {
        directoryButtons[j].setToggleState(std::cmp_equal(i, j));
    }

    updateEntries();
}

void LoadSaveWindow::onSelectionChange([[maybe_unused]] bool bInteractive) {
    if (!bSaveWindow_)
        return;

    const auto index = fileList.getSelectedIndex();
    if (!fileList.isValid(index))
        return;

    saveName.setText(fileList.getEntry(index));
}
