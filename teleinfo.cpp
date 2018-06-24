#include "teleinfo.hpp"

namespace teleinfo
{

PGM_P get_P(Error err)
{
    switch (err)
    {
    case Error::FRAME_OK:
        return PSTR("FRAME_OK");
    case Error::FRAME_END:
        return PSTR("FRAME_END");
    case Error::FRAME_TOO_SMALL:
        return PSTR("FRAME_TOO_SMALL");
    case Error::FRAME_INVALID:
        return PSTR("FRAME_INVALID");
    case Error::CHECKSUM_INVALID:
        return PSTR("CHECKSUM_INVALID");
    case Error::TIMEOUT:
        return PSTR("TIMEOUT");
    case Error::DATA_OVERFLOW:
        return PSTR("DATA_OVERFLOW");
    default:
        return PSTR("UNKNOWN");
    }
}

/*
 * class Checksum
 */

bool Checksum::_guess(const char *data, size_t size)
{
    if (_v1(data, size))
    {
        TELEINFO_PRINT_DEBUG("checksum v1\r\n");
        _method = &Checksum::_v1;
    }
    else if (_v2(data, size))
    {
        TELEINFO_PRINT_DEBUG("checksum v2\r\n");
        _method = &Checksum::_v2;
    }
    else
    {
        return false;
    }
    return true;
}

bool Checksum::_v1(const char *data, size_t size)
{
    return _base(data, size - 2, data[size - 1]);
}

bool Checksum::_v2(const char *data, size_t size)
{
    return _base(data, size - 1, data[size - 1]);
}

inline bool Checksum::_base(const char *data, size_t size, uint8_t sum)
{
    uint8_t computedSum = 0;
    for (size_t i = size; i--;)
    {
        computedSum += data[i];
    }
    computedSum = (computedSum & 0x3f) + 0x20;
#if 0
    TELEINFO_PRINT_DEBUG("sum: '%c'(%02X) == '%c'(%02X)\r\n",
                         static_cast<char>(sum), static_cast<int>(sum),
                         static_cast<char>(computedSum), static_cast<char>(computedSum));
#endif
    return computedSum == sum;
}

/*
 * class Reader
 */

bool Reader::waitForFrame()
{
    _stream->flush();
    for (;;)
    {
        switch (_read())
        {
        case '\0':
            return false;
        case Constants::START_FRAME:
            return true;
        default:
            continue;
        }
    }
}

int Reader::fillBuffer(char *buf, size_t bufSize)
{
    // start frame
    for (bool loop = true; loop;)
    {
        switch (_read())
        {
        case Constants::END_FRAME:
            return -Error::FRAME_END;
        case '\0':
            return -Error::TIMEOUT;
        case Constants::START_INFO:
            loop = false;
        default:
            break;
        }
    }
    size_t size = 0;
    for (char chr; (chr = _read()) != Constants::END_INFO;)
    {
        if (chr == '\0')
        {
            return -Error::TIMEOUT;
        }
        buf[size] = chr;
        ++size;
        if (size == bufSize)
        {
            return -Error::DATA_OVERFLOW;
        }
    }
    buf[size] = '\0';
    return size;
}

char Reader::_read()
{
    unsigned long refMillis = millis();
    while (!_stream->available())
    {
        delay(10);
        if (millis() - refMillis > 1000)
        {
            TELEINFO_PRINT_DEBUG("Read timeout\r\n");
            return '\0';
        }
    }

    return _stream->read() & 0x7f;
}

/*
 * class TeleInfoBase
 */

template <>
bool TeleInfoBase<Historical>::_splitKeyValue(Historical &data, size_t size)
{
    const char *sep = static_cast<const char *>(memchr(data.buf, Historical::SEPARATOR, size - 2));
    if (sep == nullptr)
    {
        return false;
    }
    data.key.buf = data.buf;
    data.key.size = sep - data.buf;

    data.value.buf = sep + 1;
    data.value.size = (size - 2) - (data.value.buf - data.buf);
    return true;
}

template <>
bool TeleInfoBase<Standard>::_splitKeyValue(Standard &data, size_t size)
{
    const char *sep = static_cast<const char *>(memchr(data.buf, Standard::SEPARATOR, size - 2));
    if (sep == nullptr)
    {
        return false;
    }
    data.key.buf = data.buf;
    data.key.size = sep - data.buf;

    const char *sep2 = static_cast<const char *>(memchr(sep + 1, Standard::SEPARATOR, size - (sep - data.buf) - 3));
    if (sep2 != nullptr)
    {
        data.date.buf = sep + 1;
        data.date.size = sep2 - data.date.buf;

        data.value.buf = sep2 + 1;
    }
    else
    {
        data.value.buf = sep + 1;
    }
    data.value.size = (size - 2) - (data.value.buf - data.buf);
    return true;
}

} // namespace teleinfo
