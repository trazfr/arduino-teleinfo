#if TELEINFO_DEBUG
#define TELEINFO_PRINT_DEBUG(...)   \
    do                              \
    {                               \
        Serial.printf(__VA_ARGS__); \
    } while (0)
#else
#define TELEINFO_PRINT_DEBUG(...)
#endif // TELEINFO_DEBUG

namespace teleinfo
{

/*
 * class Checksum
 */

inline Checksum::Checksum()
    : _method(&Checksum::_guess)
{
}

inline bool Checksum::operator()(const char *data, size_t size)
{
    return (this->*_method)(data, size);
}

/*
 * class Reader
 */

inline Reader::Reader(Stream &stream)
    : _stream(&stream)
{
}

/*
 * class TeleInfo
 */
template <typename TInfoData>
inline TeleInfoBase<TInfoData>::TeleInfoBase(Stream &stream)
    : Reader(stream)
{
}

template <typename TInfoData>
Error TeleInfoBase<TInfoData>::read(TInfoData &data)
{
    _clear(data);
    int size = fillBuffer(data.buf, sizeof(data.buf));
    if (size < 0)
    {
        TELEINFO_PRINT_DEBUG("Error %d\r\n", -size);
        return static_cast<Error>(-size);
    }
    if (size <= 2)
    {
        TELEINFO_PRINT_DEBUG("Frame too small\r\n");
        return Error::FRAME_TOO_SMALL;
    }

    if (_checksum(data.buf, size) == false)
    {
        TELEINFO_PRINT_DEBUG("checksum\r\n");
        return Error::CHECKSUM_INVALID;
    }

    if (_splitKeyValue(data, size) == false)
    {
        TELEINFO_PRINT_DEBUG("Frame invalid\r\n");
        return Error::FRAME_INVALID;
    }
    return Error::FRAME_OK;
}

template <>
inline void TeleInfoBase<Historical>::_clear(Historical &data)
{
    data.key.size = 0;
    data.value.size = 0;
}

template <>
inline void TeleInfoBase<Standard>::_clear(Standard &data)
{
    data.key.size = 0;
    data.date.size = 0;
    data.value.size = 0;
}

template <typename TSerial, typename TInfoData>
inline TeleInfo<TSerial, TInfoData>::TeleInfo(Stream &serial)
    : TeleInfoBase<TInfoData>(serial)
{
}

template <typename TSerial, typename TInfoData>
inline TeleInfo<TSerial, TInfoData>::~TeleInfo()
{
    end();
}

template <typename TSerial, typename TInfoData>
inline void TeleInfo<TSerial, TInfoData>::begin()
{
    getSerial().begin(TInfoData::RX_SPEED);
}

template <typename TSerial, typename TInfoData>
inline void TeleInfo<TSerial, TInfoData>::end()
{
    getSerial().end();
}

template <typename TSerial, typename TInfoData>
inline TSerial &TeleInfo<TSerial, TInfoData>::getSerial()
{
    return *static_cast<TSerial *>(Reader::_stream);
}

} // namespace teleinfo
