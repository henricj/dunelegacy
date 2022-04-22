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

#ifndef ENETPACKETOSTREAM_H
#define ENETPACKETOSTREAM_H

#include <misc/OutputStream.h>

#include <enet/enet.h>

#include <string>

class ENetPacketOStream final : public OutputStream {
public:
    explicit ENetPacketOStream(enet_uint32 flags) : currentPos(0), packet(enet_packet_create(nullptr, 16, flags)) {

        if (packet == nullptr) {
            THROW(OutputStream::error, "ENetPacketOStream: enet_packet_create() failed!");
        }
    }

    ENetPacketOStream(const ENetPacketOStream& p) : currentPos(0), packet(nullptr) { *this = p; }

    ~ENetPacketOStream() override {
        if (packet != nullptr) {
            enet_packet_destroy(packet);
        }
    }

    ENetPacketOStream& operator=(const ENetPacketOStream& p) {
        if (this != &p) {
            ENetPacket* packetCopy = enet_packet_create(p.packet->data, p.packet->dataLength, p.packet->flags);
            if (packetCopy == nullptr) {
                THROW(InputStream::error, "ENetPacketOStream::operator=(): enet_packet_create() failed!");
            }

            if (packet != nullptr) {
                enet_packet_destroy(packet);
            }

            packet     = packetCopy;
            currentPos = p.currentPos;
        }

        return *this;
    }

    ENetPacket* getPacket() {
        if (enet_packet_resize(packet, currentPos) < 0) {
            THROW(OutputStream::error, "ENetPacketOStream::getPacket(): enet_packet_resize() failed!");
        }

        ENetPacket* pPacket = packet;

        packet = nullptr;

        return pPacket;
    }

    void flush() override { }

    // write operations

    void writeString(std::string_view str) override {
        ensureBufferSize(currentPos + str.length() + sizeof(uint32_t));

        writeUint32(str.length());

        if (!str.empty()) {
            memcpy(packet->data + currentPos, str.data(), str.length());
            currentPos += str.length();
        }
    }

    void writeUint8(uint8_t x) override {
        ensureBufferSize(currentPos + sizeof(uint8_t));
        packet->data[currentPos] = x;
        currentPos += sizeof(uint8_t);
    }

    void writeUint16(uint16_t x) override {
        ensureBufferSize(currentPos + sizeof(uint16_t));
        *reinterpret_cast<uint16_t*>(packet->data + currentPos) = SDL_SwapLE16(x);
        currentPos += sizeof(uint16_t);
    }

    void writeUint32(uint32_t x) override {
        ensureBufferSize(currentPos + sizeof(uint32_t));
        *reinterpret_cast<uint32_t*>(packet->data + currentPos) = SDL_SwapLE32(x);
        currentPos += sizeof(uint32_t);
    }

    void writeUint64(uint64_t x) override {
        ensureBufferSize(currentPos + sizeof(uint64_t));
        *reinterpret_cast<uint64_t*>(packet->data + currentPos) = SDL_SwapLE64(x);
        currentPos += sizeof(uint64_t);
    }

    void writeBool(bool x) override { writeUint8(x ? 1 : 0); }

    void writeFloat(float x) override {
        uint32_t tmp = 0;
        memcpy(&tmp, &x, sizeof(uint32_t)); // workaround for a strange optimization in gcc 4.1
        writeUint32(tmp);
    }

    void ensureBufferSize(size_t minBufferSize) {
        if (minBufferSize < packet->dataLength) {
            return;
        }

        size_t newBufferSize = packet->dataLength * 3 / 2;
        if (newBufferSize < minBufferSize) {
            newBufferSize = minBufferSize;
        }

        if (enet_packet_resize(packet, newBufferSize) < 0) {
            THROW(OutputStream::error, "ENetPacketOStream::ensureBufferSize(): enet_packet_resize() failed!");
        }
    }

private:
    size_t currentPos;
    ENetPacket* packet;
};

#endif // ENETPACKETOSTREAM_H
