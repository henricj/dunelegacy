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

#ifndef ENETPACKETISTREAM_H
#define ENETPACKETISTREAM_H

#include <misc/InputStream.h>
#include <misc/exceptions.h>

#include <enet/enet.h>

#include <string>

class ENetPacketIStream : public InputStream
{
public:
    explicit ENetPacketIStream(ENetPacket* pPacket)
     : currentPos(0), packet(pPacket) {
        ;
    }

    explicit ENetPacketIStream(const ENetPacketIStream& p)
     : currentPos(0), packet(nullptr) {
        *this = p;
    }

    ~ENetPacketIStream() {
        if(packet != nullptr) {
            enet_packet_destroy(packet);
        }
    }

    ENetPacketIStream& operator=(const ENetPacketIStream& p) {
        if(this != &p) {
            ENetPacket* packetCopy = enet_packet_create(p.packet->data,p.packet->dataLength,p.packet->flags);
            if(packetCopy == nullptr) {
                THROW(InputStream::error, "ENetPacketIStream::operator=(): enet_packet_create() failed!");
            }

            if(packet != nullptr) {
                enet_packet_destroy(packet);
            }

            packet = packetCopy;
            currentPos = p.currentPos;
        }

        return *this;
    }

    std::string readString() override
    {
        Uint32 length = readUint32();

        if(currentPos + length > packet->dataLength) {
            THROW(InputStream::eof, "ENetPacketIStream::readString(): End-of-File reached!");
        }

        std::string resultString((char*) (packet->data + currentPos), length);
        currentPos += length;
        return resultString;
    }

    Uint8 readUint8() override
    {
        if(currentPos + sizeof(Uint8) > packet->dataLength) {
            THROW(InputStream::eof, "ENetPacketIStream::readUint8(): End-of-File reached!");
        }

        Uint8 tmp = *((Uint8*) (packet->data + currentPos));
        currentPos += sizeof(Uint8);
        return tmp;
    }

    Uint16 readUint16() override
    {
        if(currentPos + sizeof(Uint16) > packet->dataLength) {
            THROW(InputStream::eof, "ENetPacketIStream::readUint16(): End-of-File reached!");
        }

        Uint16 tmp = *((Uint16*) (packet->data + currentPos));
        currentPos += sizeof(Uint16);
        return SDL_SwapLE16(tmp);
    }

    Uint32 readUint32() override
    {
        if(currentPos + sizeof(Uint32) > packet->dataLength) {
            THROW(InputStream::eof, "ENetPacketIStream::readUint32(): End-of-File reached!");
        }

        Uint32 tmp = *((Uint32*) (packet->data + currentPos));
        currentPos += sizeof(Uint32);
        return SDL_SwapLE32(tmp);
    }

    Uint64 readUint64() override
    {
        if(currentPos + sizeof(Uint64) > packet->dataLength) {
            THROW(InputStream::eof, "ENetPacketIStream::readUint64(): End-of-File reached!");
        }

        Uint64 tmp = *((Uint64*) (packet->data + currentPos));
        currentPos += sizeof(Uint64);
        return SDL_SwapLE64(tmp);
    }

    bool readBool() override
    {
        return (readUint8() == 1 ? true : false);
    }

    float readFloat() override
    {
        Uint32 tmp = readUint32();
        float tmp2;
        memcpy(&tmp2,&tmp,sizeof(Uint32)); // workaround for a strange optimization in gcc 4.1
        return tmp2;
    }

private:
    size_t  currentPos;
    ENetPacket* packet;
};

#endif // ENETPACKETISTREAM_H
