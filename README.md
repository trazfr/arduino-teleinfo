# TeleInfo

Bufferless implementation of the Teleinfo protocol for Arduino.  
It is used to stream checksum-validated data without any big static buffer.

It supports both Historical and Standard modes.

## Usage

### Prerequisites

You may choose:

 - a Serial library ([Serial](https://www.arduino.cc/reference/en/language/functions/communication/serial/),
                     [SoftwareSerial](https://www.arduino.cc/en/Reference/SoftwareSerial),
                     [AltSoftSerial](https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html)...)
 - which protocol you use (`Historical` or `Standard`)

### Usage

Using `SoftwareSerial` and `Historical` teleinfo protocol:

```cpp
#include "teleinfo.hpp"

SoftwareSerial softSerial(RX_PIN, TX_PIN);
auto tinfo = teleinfo::create<teleinfo::Historical>(softSerial);

void setup()
{
    tinfo.begin();
}

void loop()
{
    if (!tinfo.waitForFrame())
    {
        Serial.println("Failed ");
        // Error
    }
    teleinfo::Historical info;
    Teleinfo::Error err;
    for (; (err = tinfo.read(info)) == teleinfo::FRAME_OK;)
    {
        // info contains a valid frame with right checksum
        // info.buf contains the full frame (NULL-terminated)
        // info.key.buf contains the key (ADCO, HPHP...). Its size is info.key.size
        // info.value.buf contains the value. Its size is info.value.size
        // Attention: they are not NULL-terminated
        Serial.write(info.key.buf, info.key.size);
        Serial.write(" = ");
        Serial.write(info.value.buf, info.value.size);
        Serial.write("\r\n");
    }
    // Get the error:
    //PGM_P error_P = teleinfo.get_P(err);
}
```

Usage with `Serial` and `Standard` protocol. The serial object must stay valid:

```cpp
auto tinfo = teleinfo::create<teleinfo::Standard>(Serial);
// ...
void loop()
{
    // ...
    teleinfo::Standard info;
    for (; (err = tinfo.read(info)) == teleinfo::FRAME_OK;)
    {

    }
}
```
