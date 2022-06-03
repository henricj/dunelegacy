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

#ifndef SDLDATASOURCE_H
#define SDLDATASOURCE_H

#include "databuf.h"

#include "misc/BufferedReader.h"

#include <misc/SDL2pp.h>

#include <SDL2/SDL_rwops.h>

inline uint8_t Read1(BufferedReader<>& reader) {
    uint8_t value = 0;
    if (!reader.read_one(&value, 1))
        THROW(std::runtime_error, "Read failed");

    return value;
}

inline void Write1(SDL_RWops* rwop, uint8_t value) {
    if (1 != SDL_RWwrite(rwop, &value, 1, 1))
        THROW(std::runtime_error, "Write failed");
}

class ISDLDataSource final : public IDataSource {
private:
    SDL_RWops* rwop;
    int freesrc;
    BufferedReader<> reader_;

public:
    explicit ISDLDataSource(SDL_RWops* rwop, int freesrc = 0);

    ~ISDLDataSource() override;

    virtual void close();

    uint32_t read1() override { return Read1(reader_); }

    uint16_t read2() override { return Read2(reader_); }

    uint16_t read2high() override { return Read2high(reader_); }

    uint32_t read4() override { return Read4(reader_); }

    uint32_t read4high() override { return Read4high(reader_); }

    void read(void* b, size_t len) override;

    void seek(size_t pos) override;

    void skip(std::streamoff pos) override;

    [[nodiscard]] size_t getSize() const override { return static_cast<unsigned int>(SDL_RWsize(rwop)); }

    [[nodiscard]] size_t getPos() const override { return reader_.position(); }

    uint32_t peek() override;

    std::unique_ptr<IDataSource> makeSource(size_t len) override;

    [[nodiscard]] bool eof() const override { return getPos() < getSize(); }
};

class OSDLDataSource final : public ODataSource {
private:
    SDL_RWops* rwop;
    int freesrc;

public:
    explicit OSDLDataSource(SDL_RWops* rwop, int freesrc = 0);

    ~OSDLDataSource() override;

    virtual void close();

    void write1(uint32_t val) override { Write1(rwop, static_cast<uint8_t>(val)); }

    void write2(uint16_t val) override { Write2(rwop, val); }

    void write2high(uint16_t val) override { Write2high(rwop, val); }

    void write4(uint32_t val) override { Write4(rwop, val); }

    void write4high(uint32_t val) override { Write4high(rwop, val); }

    void write(const void* b, size_t len) override;

    void seek(size_t pos) override;

    void skip(std::streamoff pos) override;

    [[nodiscard]] size_t getSize() const override { return static_cast<unsigned int>(SDL_RWsize(rwop)); }

    [[nodiscard]] size_t getPos() const override { return static_cast<unsigned int>(SDL_RWtell(rwop)); }
};

#endif
