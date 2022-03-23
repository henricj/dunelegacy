#include "misc/OutputStream.h"

OutputStream::OutputStream()  = default;
OutputStream::~OutputStream() = default;

void OutputStream::writeSint8(int8_t x) {
    const uint8_t tmp = *reinterpret_cast<uint8_t*>(&x);
    writeUint8(tmp);
}

void OutputStream::writeSint16(int16_t x) {
    const uint16_t tmp = *reinterpret_cast<uint16_t*>(&x);
    writeUint16(tmp);
}

void OutputStream::writeSint32(int32_t x) {
    const uint32_t tmp = *reinterpret_cast<uint32_t*>(&x);
    writeUint32(tmp);
}

void OutputStream::writeSint64(int64_t x) {
    const uint64_t tmp = *reinterpret_cast<uint64_t*>(&x);
    writeUint64(tmp);
}

void OutputStream::writeFixPoint(FixPoint x) {
    writeSint64(x.getRawValue());
}

void OutputStream::writeBools(bool val1, bool val2, bool val3, bool val4, bool val5, bool val6, bool val7, bool val8) {
    const uint8_t val =
        static_cast<uint8_t>(val1) | val2 << 1 | val3 << 2 | val4 << 3 | val5 << 4 | val6 << 5 | val7 << 6 | val8 << 7;
    writeUint8(val);
}

void OutputStream::writeUint8Vector(gsl::span<const uint8_t> dataVector) {
    writeUint32(static_cast<uint32_t>(dataVector.size()));
    for (const auto data : dataVector) {
        writeUint8(data);
    }
}

void OutputStream::writeUint32List(const std::list<uint32_t>& dataList) {
    writeUint32(static_cast<uint32_t>(dataList.size()));
    for (const auto data : dataList) {
        writeUint32(data);
    }
}

void OutputStream::writeUint32Vector(gsl::span<const uint32_t> dataVector) {
    writeUint32(static_cast<uint32_t>(dataVector.size()));
    for (const auto data : dataVector) {
        writeUint32(data);
    }
}

void OutputStream::writeUint32Set(const Dune::selected_set_type& dataSet) {
    writeUint32(static_cast<uint32_t>(dataSet.size()));
    for (const auto data : dataSet) {
        writeUint32(data);
    }
}
