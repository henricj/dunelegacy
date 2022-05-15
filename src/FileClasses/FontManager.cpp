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

#include <FileClasses/FontManager.h>

#include <globals.h>

#include "misc/DrawingRectHelper.h"
#include <misc/FileSystem.h>
#include <misc/draw_util.h>

#include <FileClasses/FileManager.h>
#include <FileClasses/LoadSavePNG.h>
#include <FileClasses/TTFFont.h>

FontManager::FontManager(std::filesystem::path font_path) : font_{loadImage(std::move(font_path))} { }

FontManager::~FontManager() = default;

Font* FontManager::getFont(uint32_t fontSize) {
    auto& font = fonts[fontSize];

    if (!font)
        font = loadFont(fontSize);

    return font.get();
}

std::unique_ptr<Font> FontManager::loadFont(unsigned int fontSize) const {
    sdl2::RWops_ptr file{SDL_RWFromConstMem(font_.data(), static_cast<int>(font_.size()))};

    return std::make_unique<TTFFont>(std::move(file), fontSize);
}

std::vector<char> FontManager::loadImage(std::filesystem::path font_path) {
    const auto file = dune::globals::pFileManager->openFile(std::move(font_path));
    if (!file)
        THROW(std::runtime_error, "Unable to open font because %s!", SDL_GetError());

    const auto size = SDL_RWsize(file.get());

    std::vector<char> buffer(size);

    if (1 != SDL_RWread(file.get(), buffer.data(), buffer.size(), 1))
        THROW(std::runtime_error, "Unable to load font because %s!", SDL_GetError());

    return buffer;
}
