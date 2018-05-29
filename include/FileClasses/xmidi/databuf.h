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

// Datasource class for universal methods to retreive data from different sources

#ifndef DATABUF_H
#define DATABUF_H

#include <misc/exceptions.h>
#include <misc/SDL2pp.h>

#include <stdio.h>

typedef char * charptr;

class DataSource
{
public:
    DataSource() = default;
    virtual ~DataSource() = default;

    virtual unsigned int read1() =0;
    virtual unsigned int read2() =0;
    virtual unsigned int read2high() =0;
    virtual unsigned int read4() =0;
    virtual unsigned int read4high() =0;
    virtual void read(char *, size_t) =0;

    virtual void write1(unsigned int) =0;
    virtual void write2(unsigned int) =0;
    virtual void write2high(unsigned int) =0;
    virtual void write4(unsigned int) =0;
    virtual void write4high(unsigned int) =0;

    virtual void seek(unsigned int) =0;
    virtual void skip(int) =0;
    virtual unsigned int getSize() =0;
    virtual unsigned int getPos() =0;
};


class BufferDataSource: public DataSource
{
private:
    unsigned char *buf, *buf_ptr;
    unsigned int size;
public:
    BufferDataSource(char *data, unsigned int len)
    {
        buf = buf_ptr = (unsigned char*)data;
        size = len;
    };

    virtual ~BufferDataSource() = default;

    unsigned int read1() override
    {
        unsigned char b0;
        b0 = (unsigned char)*buf_ptr++;
        return (b0);
    };

    unsigned int read2() override
    {
        unsigned char b0, b1;
        b0 = (unsigned char)*buf_ptr++;
        b1 = (unsigned char)*buf_ptr++;
        return (b0 + (b1 << 8));
    };

    unsigned int read2high() override
    {
        unsigned char b0, b1;
        b1 = (unsigned char)*buf_ptr++;
        b0 = (unsigned char)*buf_ptr++;
        return (b0 + (b1 << 8));
    };

    unsigned int read4() override
    {
        unsigned char b0, b1, b2, b3;
        b0 = (unsigned char)*buf_ptr++;
        b1 = (unsigned char)*buf_ptr++;
        b2 = (unsigned char)*buf_ptr++;
        b3 = (unsigned char)*buf_ptr++;
        return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
    };

    unsigned int read4high() override
    {
        unsigned char b0, b1, b2, b3;
        b3 = (unsigned char)*buf_ptr++;
        b2 = (unsigned char)*buf_ptr++;
        b1 = (unsigned char)*buf_ptr++;
        b0 = (unsigned char)*buf_ptr++;
        return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
    };

    void read(char *b, size_t len) override
    {
        memcpy(b, buf_ptr, len);
        buf_ptr += len;
    };

    void write1(unsigned int val) override
    {
        *buf_ptr++ = val & 0xff;
    };

    void write2(unsigned int val) override
    {
        *buf_ptr++ = val & 0xff;
        *buf_ptr++ = (val>>8) & 0xff;
    };

    void write2high(unsigned int val) override
    {
        *buf_ptr++ = (val>>8) & 0xff;
        *buf_ptr++ = val & 0xff;
    };


    void write4(unsigned int val) override
    {
        *buf_ptr++ = val & 0xff;
        *buf_ptr++ = (val>>8) & 0xff;
        *buf_ptr++ = (val>>16)&0xff;
        *buf_ptr++ = (val>>24)&0xff;
    };

    void write4high(unsigned int val) override
    {
        *buf_ptr++ = (val>>24)&0xff;
        *buf_ptr++ = (val>>16)&0xff;
        *buf_ptr++ = (val>>8) & 0xff;
        *buf_ptr++ = val & 0xff;
    };

    void seek(unsigned int pos) override { buf_ptr = buf+pos; };

    void skip(int pos) override { buf_ptr += pos; };

    unsigned int getSize() override { return size; };

    unsigned int getPos() override { return (buf_ptr-buf); };

    unsigned char *getPtr() { return buf_ptr; };
};

class SDLDataSource : public DataSource
{
private:
    SDL_RWops* rwop;
    int freesrc;
public:
    SDLDataSource(SDL_RWops* rwop, int freesrc = 0) {
        this->rwop = rwop;
        this->freesrc = freesrc;
    };

    virtual ~SDLDataSource() {
        close();
    };

    virtual void close() {
        if(freesrc && rwop != nullptr) {
            SDL_RWclose(rwop);
            rwop = nullptr;
        }
    };

    unsigned int read1() override
    {
        unsigned char b0;
        SDL_RWread(rwop,&b0,sizeof(b0),1);
        return (b0);
    };

    unsigned int read2() override
    {
        unsigned char b0, b1;
        SDL_RWread(rwop,&b0,sizeof(b0),1);
        SDL_RWread(rwop,&b1,sizeof(b1),1);
        return (b0 + (b1 << 8));
    };

    unsigned int read2high() override
    {
        unsigned char b0, b1;
        SDL_RWread(rwop,&b1,sizeof(b1),1);
        SDL_RWread(rwop,&b0,sizeof(b0),1);
        return (b0 + (b1 << 8));
    };

    unsigned int read4() override
    {
        unsigned char b0, b1, b2, b3;
        SDL_RWread(rwop,&b0,sizeof(b0),1);
        SDL_RWread(rwop,&b1,sizeof(b1),1);
        SDL_RWread(rwop,&b2,sizeof(b2),1);
        SDL_RWread(rwop,&b3,sizeof(b3),1);
        return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
    };

    unsigned int read4high() override
    {
        unsigned char b0, b1, b2, b3;
        SDL_RWread(rwop,&b3,sizeof(b3),1);
        SDL_RWread(rwop,&b2,sizeof(b2),1);
        SDL_RWread(rwop,&b1,sizeof(b1),1);
        SDL_RWread(rwop,&b0,sizeof(b0),1);
        return (b0 + (b1<<8) + (b2<<16) + (b3<<24));
    };

    void read(char *b, size_t len) override
    {
        SDL_RWread(rwop,b,1,len);
    };

    void write1(unsigned int val) override
    {
        Uint8 b0 = val & 0xff;
        SDL_RWwrite(rwop,&b0,sizeof(b0),1);
    };

    void write2(unsigned int val) override
    {
        Uint8 b0 = val & 0xff;
        Uint8 b1 = (val>>8) & 0xff;
        SDL_RWwrite(rwop,&b0,sizeof(b0),1);
        SDL_RWwrite(rwop,&b1,sizeof(b1),1);
    };

    void write2high(unsigned int val) override
    {
        Uint8 b0 = val & 0xff;
        Uint8 b1 = (val>>8) & 0xff;
        SDL_RWwrite(rwop,&b1,sizeof(b1),1);
        SDL_RWwrite(rwop,&b0,sizeof(b0),1);
    };


    void write4(unsigned int val) override
    {
        Uint8 b0 = val & 0xff;
        Uint8 b1 = (val>>8) & 0xff;
        Uint8 b2 = (val>>16) & 0xff;
        Uint8 b3 = (val>>24) & 0xff;
        SDL_RWwrite(rwop,&b0,sizeof(b0),1);
        SDL_RWwrite(rwop,&b1,sizeof(b1),1);
        SDL_RWwrite(rwop,&b2,sizeof(b2),1);
        SDL_RWwrite(rwop,&b3,sizeof(b3),1);
    };

    void write4high(unsigned int val) override
    {
        Uint8 b0 = val & 0xff;
        Uint8 b1 = (val>>8) & 0xff;
        Uint8 b2 = (val>>16) & 0xff;
        Uint8 b3 = (val>>24) & 0xff;
        SDL_RWwrite(rwop,&b3,sizeof(b3),1);
        SDL_RWwrite(rwop,&b2,sizeof(b2),1);
        SDL_RWwrite(rwop,&b1,sizeof(b1),1);
        SDL_RWwrite(rwop,&b0,sizeof(b0),1);
    };

    void seek(unsigned int pos) override
    {
        SDL_RWseek(rwop, pos, SEEK_SET);
    };

    void skip(int pos) override
    {
        SDL_RWseek(rwop, pos, SEEK_CUR);
    };

    unsigned int getSize() override
    {
        return static_cast<unsigned int>(SDL_RWsize(rwop));
    };

    unsigned int getPos() override
    {
        return static_cast<unsigned int>(SDL_RWtell(rwop));
    };
};

#endif
