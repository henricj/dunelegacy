#include "misc/InputStream.h"

InputStream::InputStream()  = default;
InputStream::~InputStream() = default;

int8_t InputStream::readSint8() {
    uint8_t tmp = readUint8();
    return *reinterpret_cast<int8_t*>(&tmp);
}

int16_t InputStream::readSint16() {
    uint16_t tmp = readUint16();
    return *reinterpret_cast<int16_t*>(&tmp);
}

int32_t InputStream::readSint32() {
    uint32_t tmp = readUint32();
    return *reinterpret_cast<int32_t*>(&tmp);
}

int64_t InputStream::readSint64() {
    uint64_t tmp = readUint64();
    return *reinterpret_cast<int64_t*>(&tmp);
}

void InputStream::readBools(bool* pVal1, bool* pVal2, bool* pVal3, bool* pVal4, bool* pVal5, bool* pVal6, bool* pVal7,
                            bool* pVal8) {
    uint8_t val = readUint8();

    if (pVal1 != nullptr)
        *pVal1 = ((val & 0x01) != 0);
    if (pVal2 != nullptr)
        *pVal2 = ((val & 0x02) != 0);
    if (pVal3 != nullptr)
        *pVal3 = ((val & 0x04) != 0);
    if (pVal4 != nullptr)
        *pVal4 = ((val & 0x08) != 0);
    if (pVal5 != nullptr)
        *pVal5 = ((val & 0x10) != 0);
    if (pVal6 != nullptr)
        *pVal6 = ((val & 0x20) != 0);
    if (pVal7 != nullptr)
        *pVal7 = ((val & 0x40) != 0);
    if (pVal8 != nullptr)
        *pVal8 = ((val & 0x80) != 0);
}

std::vector<uint8_t> InputStream::readUint8Vector() {
    std::vector<uint8_t> vec;
    readUint8Vector(vec);

    return vec;
}

void InputStream::readUint8Vector(std::vector<uint8_t>& vec) {
    vec.clear();
    const auto size = readUint32();
    vec.reserve(size);
    for (unsigned int i = 0; i < size; i++) {
        vec.push_back(readUint8());
    }
}

std::list<uint32_t> InputStream::readUint32List() {
    std::list<uint32_t> List;
    const auto size = readUint32();
    for (unsigned int i = 0; i < size; i++) {
        List.push_back(readUint32());
    }
    return List;
}

void InputStream::readUint32Vector(std::vector<uint32_t>& vec) {
    vec.clear();
    const auto size = readUint32();
    vec.reserve(size);
    for (unsigned int i = 0; i < size; i++) {
        vec.push_back(readUint32());
    }
}

std::vector<uint32_t> InputStream::readUint32Vector() {
    std::vector<uint32_t> vec;
    readUint32Vector(vec);

    return vec;
}

Dune::selected_set_type InputStream::readUint32Set() {
    Dune::selected_set_type retSet;
    const auto size = readUint32();
    for (auto i = decltype(size) {0}; i < size; i++) {
        retSet.insert(readUint32());
    }
    return retSet;
}
