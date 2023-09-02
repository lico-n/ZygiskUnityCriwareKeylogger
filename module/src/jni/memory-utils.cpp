#include "memory-utils.h"

#include <cinttypes>
#include <codecvt>
#include <locale>
#include <string>

static unsigned char *read_byte_array(uint64_t addr, int offset, uint32_t size) {
    auto result = new unsigned char[size];
    auto absOffset = addr + offset;

    memcpy(result, reinterpret_cast<void *>(absOffset), size);

    return result;
}


static std::string utf16_to_utf8(std::u16string const &s) {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10ffff,
        std::codecvt_mode::little_endian>, char16_t> cnv;
    return cnv.to_bytes(s);
}

std::string read_system_string(uint64_t addr) {
    auto charLengthBytes = read_byte_array(addr, 0x10, 4);

    auto charLength = (uint32_t) charLengthBytes[0] << 0 |
                      (uint32_t) charLengthBytes[1] << 8 |
                      (uint32_t) charLengthBytes[2] << 16 |
                      (uint32_t) charLengthBytes[3] << 24;

    auto maxBytes = charLength * 4 + 2;  // max 4 bytes per character + 2 null terminators

    unsigned char stringBytes[maxBytes];
    int offset = 0;

    for (auto i = 0; i < charLength; i++) {
        auto charBytes = read_byte_array(addr, 0x14 + offset, 2);

        // add first byte pair to the string bytes
        stringBytes[offset] = charBytes[0];
        stringBytes[offset + 1] = charBytes[1];
        offset += 2;

        // high surrogates are from range D800 to U+DBFF,
        // they indicate that the char contains 2 more bytes
        if (0xD8 <= charBytes[0] && charBytes[0] < 0xDC) {
            auto lowSurrogate = read_byte_array(addr, 0x14 + offset, 2);
            stringBytes[offset] = lowSurrogate[0];
            stringBytes[offset + 1] = lowSurrogate[1];
            offset += 2;
        }
    }

    // add null terminator
    stringBytes[offset] = 0;
    stringBytes[offset + 1] = 0;

    std::u16string u16_str(reinterpret_cast<const char16_t *>(stringBytes));

    return utf16_to_utf8(u16_str);
}
