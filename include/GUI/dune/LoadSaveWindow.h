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

#ifndef LOADSAVEWINDOW_H
#define LOADSAVEWINDOW_H

#include <GUI/Window.h>
#include <GUI/HBox.h>
#include <GUI/VBox.h>
#include <GUI/TextButton.h>
#include <GUI/TextBox.h>
#include <GUI/ListBox.h>
#include <GUI/Label.h>
#include <Definitions.h>

#include <string>
#include <vector>

class LoadSaveWindow : public Window
{
public:
    LoadSaveWindow(bool bSave, const std::string& caption, const std::vector<std::string>& directories, const std::vector<std::string>& directoryTitles, const std::string& extension, int preselectedDirectoryIndex = 0, const std::string& preselectedFile = "", Uint32 color = COLOR_DEFAULT);
    virtual ~LoadSaveWindow();

    void updateEntries();
    std::string getFilename() { return filename; };

    bool isSaveWindow() { return bSaveWindow; };

    const std::string& getDirectory() const { return directories[currentDirectoryIndex]; };

    int getCurrentDirectoryIndex() const { return currentDirectoryIndex; };

    std::string getExtension() { return extension; };

    bool handleKeyPress(SDL_KeyboardEvent& key) override;

    /**
        This method is called, when the child window is about to be closed.
        This child window will be closed after this method returns.
        \param  pChildWindow    The child window that will be closed
    */
    void onChildWindowClose(Window* pChildWindow) override;

    void onSaveNameCancel();
    void onSaveNameConfirm();

    /**
        This static method creates a dynamic load/save window.
        The idea behind this method is to simply create a new dialog on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  bSave       true = Save window, false = Load window
        \param  caption     the caption of the window
        \param  directory   the directory to save/load from
        \param  extension   the file extension
        \param  preselectedFile the name of a file (without extension) that shall be selected upon opening this dialog
        \param  color       the color of the new dialog
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static LoadSaveWindow* create(bool bSave, const std::string& caption, const std::string& directory, const std::string& extension, const std::string& preselectedFile = "", Uint32 color = COLOR_DEFAULT) {
        std::vector<std::string> directories;
        directories.push_back(directory);
        std::vector<std::string> directoryTitles;
        directoryTitles.emplace_back("");

        LoadSaveWindow* dlg = new LoadSaveWindow(bSave, caption, directories, directoryTitles, extension, 0, preselectedFile, color);
        dlg->pAllocated = true;
        return dlg;
    }

    /**
        This static method creates a dynamic load/save window.
        The idea behind this method is to simply create a new dialog on the fly and
        add it as a child window of some other window. If the window gets closed it will be freed.
        \param  bSave       true = Save window, false = Load window
        \param  caption     the caption of the window
        \param  directories a list of directories to save/load from
        \param  directoryTitles a list of button titles for the listed directories
        \param  extension   the file extension
        \param  preselectedDirectoryIndex   which directory to show on opening this dialog?
        \param  preselectedFile the name of a file (without extension) that shall be selected upon opening this dialog
        \param  color       the color of the new dialog
        \return The new dialog box (will be automatically destroyed when it's closed)
    */
    static LoadSaveWindow* create(bool bSave, const std::string& caption, const std::vector<std::string>& directories, const std::vector<std::string>& directoryTitles, const std::string& extension, int preselectedDirectoryIndex = 0, const std::string& preselectedFile = "", Uint32 color = COLOR_DEFAULT) {
        LoadSaveWindow* dlg = new LoadSaveWindow(bSave, caption, directories, directoryTitles, extension, preselectedDirectoryIndex, preselectedFile, color);
        dlg->pAllocated = true;
        return dlg;
    }

private:
    void onOK();
    void onCancel();

    void onDirectoryChange(int i);

    void onSelectionChange(bool bInteractive);

    HBox    mainHBox;
    VBox    mainVBox;

    HBox    directoryHBox;
    HBox    fileListHBox;

    HBox    buttonHBox;
    std::vector<TextButton> directoryButtons;

    Label       titleLabel;
    ListBox     fileList;
    TextButton  okButton;
    TextButton  cancelButton;
    TextBox     saveName;

    bool        bSaveWindow;
    std::string filename;
    std::vector<std::string> directories;
    std::vector<std::string> directoryTitles;
    std::string extension;
    int currentDirectoryIndex;
    std::string preselectedFile;
    Uint32      color;
};

#endif //LOADSAVEWINDOW_H
