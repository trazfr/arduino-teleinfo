#ifndef TELEINFO_HPP
#define TELEINFO_HPP

#include <Arduino.h>

namespace teleinfo
{

enum Error
{
    FRAME_OK = 0,
    FRAME_END,
    FRAME_TOO_SMALL,
    FRAME_INVALID,
    CHECKSUM_INVALID,
    TIMEOUT,
    DATA_OVERFLOW,
};

enum Constants
{
    MAX_INFO_SIZE = 32,
    START_FRAME = 0x02,
    END_FRAME = 0x03,
    START_INFO = 0x0A, // CR
    END_INFO = 0x0D,   // LF
};

struct BufRef
{
    const char *buf;
    size_t size;
};

struct Historical
{
    enum ConstantsGen
    {
        SEPARATOR = 0x20,
        RX_SPEED = 1200,
    };
    BufRef key;
    BufRef value;
    char buf[Constants::MAX_INFO_SIZE];
};

struct Standard
{
    enum ConstantsGen
    {
        SEPARATOR = 0x09,
        RX_SPEED = 9600,
    };
    BufRef key;
    BufRef date;
    BufRef value;
    char buf[Constants::MAX_INFO_SIZE];
};

/*
 * class Checksum
 */

class Checksum
{
  public:
    Checksum();

    bool operator()(const char *data, size_t size);

  private:
    bool _guess(const char *data, size_t size);
    bool _v1(const char *data, size_t size);
    bool _v2(const char *data, size_t size);
    static bool _base(const char *data, size_t size, uint8_t sum);

    bool (Checksum::*_method)(const char *data, size_t size);
};

/*
 * class Reader
 */

class Reader
{
  public:
    Reader(Stream &stream);

    bool waitForFrame();
    int fillBuffer(char *buf, size_t bufSize);

  protected:
    Stream *_stream;

  private:
    char _read();
};

/*
 * class TeleInfoBase
 */

template <typename TInfoData>
class TeleInfoBase : public Reader
{
  public:
    TeleInfoBase(Stream &stream);

    Error read(TInfoData &data);

  private:
    static void _clear(TInfoData &data);
    static bool _splitKeyValue(TInfoData &data, size_t size);

    Checksum _checksum;
};

/*
 * class TeleInfo
 */

template <typename TSerial, typename TInfoData>
class TeleInfo : public TeleInfoBase<TInfoData>
{
  public:
    TeleInfo(Stream &serial);
    ~TeleInfo();

    void begin();
    void end();

  private:
    TSerial &getSerial();
};

/*
 * Functions
 */

PGM_P get_P(Error err);

template <typename TInfoData, typename TSerial>
inline TeleInfo<TSerial, TInfoData> create(TSerial &serial)
{
    return TeleInfo<TSerial, TInfoData>(serial);
}

} // namespace teleinfo

#include "teleinfo.ipp"

#endif // TELEINFO_HPP
