# Industrial Shield — Wi-Fi and ThingSpeak contribution

This folder contains Dolima's IoT/cloud-connectivity contribution for the
Industrial Shield project. It merges ThingSpeak publishing into the team's
existing ESP32 sensor and buzzer firmware. The local sensor/alarm loop remains
on the ESP32; the Wi-Fi upload runs as a separate task.

## Data path

`MQ-2 + DS18B20 + INA219 -> ESP32 -> phone hotspot/Wi-Fi -> ThingSpeak`

The dashboard reads the same ThingSpeak channel. This code does not upload
anything merely by being stored on GitHub: the sketch must be merged, compiled,
configured locally, and uploaded to the ESP32.

## ThingSpeak channel contract

- Channel ID: `3591124` (identifier, not a secret)
- Field 1: Gas ADC — raw ADC count, not ppm
- Field 2: Temperature C — `-127` means DS18B20 unavailable
- Field 3: Current mA — `-1` means INA219 unavailable
- Field 4: Risk Code — `-1` sensor fault, `0` safe, `1` warning, `2` critical

Risk code mapping: one active alarm flag is WARNING; two or more simultaneous
hazard flags are CRITICAL; if there is no active hazard but temperature or
current sensor is unavailable, the status is SENSOR FAULT. This mapping is
derived from the supplied firmware and should be kept consistent with the
dashboard.

## Local setup (keep credentials out of Git)

1. Copy `firmware/secrets.example.h` to `firmware/secrets.h`.
2. In the local `secrets.h`, enter the phone hotspot name/password and the
   ThingSpeak Write API Key. Keep `secrets.h` untracked; `.gitignore` excludes it.
3. Connect the laptop containing the ESP32 to the phone hotspot. The ESP32
   joins the same hotspot using the values in `secrets.h`.
4. Install Arduino libraries: ThingSpeak, OneWire, DallasTemperature, and
   Adafruit INA219. Select the team's ESP32 board and COM port.
5. Compile, upload, then check Serial Monitor at 115200 baud for
   `ThingSpeak upload OK`.

ThingSpeak's free write interval is at least 15 seconds; this sketch sends
every 16 seconds. The channel is private, so the dashboard needs its **Read**
API Key. Never put the Write API Key in dashboard code or a public repository.

## Hardware pins and preserved thresholds

- DS18B20 data: GPIO 32
- MQ-2 analog output: GPIO 34 (use the team's verified voltage divider)
- Buzzer: GPIO 25
- Button: GPIO 35
- INA219 I2C: pins 13 and 27; firmware probes both SDA/SCL orders
- Existing thresholds retained: 33/31 C, gas ADC 2100/1900, current 500/400 mA,
  current jump 150 mA. These are classroom prototype values, not certified
  industrial safety limits.

## Team integration

Agrima/dashboard owner: read fields 1–4 from channel `3591124`, map them using
the contract above, and use recent feeds for chart/history. The current
firmware sends no relay or buzzer status fields, so don't present those as live
ThingSpeak values until the channel and firmware are extended together.

This contribution has compiled in Arduino IDE but still needs an actual-board
upload and ThingSpeak entry check before it can be described as hardware-tested.
