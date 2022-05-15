/*
Copyright (C) 2000, 2001  The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "FileClasses/xmidi/SDLDataSource.h"

ISDLDataSource::ISDLDataSource(SDL_RWops* rwop, int freesrc) : rwop(rwop), freesrc(freesrc), reader_{rwop} { }

ISDLDataSource::~ISDLDataSource() {
    close();
}

void ISDLDataSource::close() {
    if (freesrc && rwop != nullptr) {
        SDL_RWclose(rwop);
        rwop = nullptr;
    }
}

void ISDLDataSource::read(void* b, size_t len) {
    if (1 != reader_.read(b, len, 1))
        THROW(std::runtime_error, "Unable to read file.");
}

void ISDLDataSource::seek(size_t pos) {
    reader_.seek(pos);
}

void ISDLDataSource::skip(std::streamoff pos) {
    reader_.skip(pos);
}

uint32_t ISDLDataSource::peek() {
    THROW(std::runtime_error, "Peek is not supported");
}

std::unique_ptr<IDataSource> ISDLDataSource::makeSource(size_t len) {
    return std::make_unique<IBufferDataSource>(readN(len), len);
}

OSDLDataSource::OSDLDataSource(SDL_RWops* rwop, int freesrc) : rwop(rwop), freesrc(freesrc) { }

OSDLDataSource::~OSDLDataSource() {
    close();
}

void OSDLDataSource::close() {
    if (freesrc && rwop != nullptr) {
        SDL_RWclose(rwop);
        rwop = nullptr;
    }
}

void OSDLDataSource::write(const void* b, size_t len) {
    if (len != SDL_RWwrite(rwop, b, 1, len))
        THROW(std::runtime_error, "Unable to write file.");
}

void OSDLDataSource::seek(size_t pos) {
    if (-1 == SDL_RWseek(rwop, pos, SEEK_SET))
        THROW(std::runtime_error, "Unable to seek file.");
}

void OSDLDataSource::skip(std::streamoff pos) {
    if (-1 == SDL_RWseek(rwop, pos, SEEK_CUR))
        THROW(std::runtime_error, "Unable to skip file.");
}
