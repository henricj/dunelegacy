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
        THROW(std::runtime_error, "Unable to open font because {}!", SDL_GetError());

    const auto size = SDL_SizeIO(file.get());

    std::vector<char> buffer(size);

    // SDL3: SDL_ReadIO returns number of bytes read and may return less than requested
    // We need to loop until all bytes are read or an error occurs
    size_t total_read = 0;
    while (total_read < buffer.size()) {
        const auto bytes_read = SDL_ReadIO(file.get(), buffer.data() + total_read, buffer.size() - total_read);
        if (bytes_read == 0) {
            // EOF or error - check SDL_GetIOStatus for details
            const auto status = SDL_GetIOStatus(file.get());
            if (status == SDL_IO_STATUS_EOF) {
                THROW(std::runtime_error, "Unable to load font: unexpected end of file!");
            } else if (status == SDL_IO_STATUS_ERROR) {
                THROW(std::runtime_error, "Unable to load font because {}!", SDL_GetError());
            }
            // SDL_IO_STATUS_READY with 0 bytes is unusual, but break to avoid infinite loop
            break;
        }
        total_read += bytes_read;
    }

    if (total_read != buffer.size())
        THROW(std::runtime_error, "Unable to load font: read {} of {} bytes!", total_read, buffer.size());

    return buffer;
}
