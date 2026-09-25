# 433 MHz RF Remote Transmitter (4 Channels)

Arduino firmware for a four-button 433 MHz ASK remote control. Each button press or release is debounced, encoded into a small text frame with a sequence number, and transmitted several times to compensate for the unreliability of low-cost RF links.

This is the transmitter half of a pair. The matching receiver, which drives four relays, lives in [recepteur_433mhz_rf-interrupteur_4_pos_arduino_code](https://github.com/GUELORD-MWENDERWA/recepteur_433mhz_rf-interrupteur_4_pos_arduino_code).

## Features

- Four independent inputs with per-button software debouncing (50 ms)
- Non-blocking design driven by `millis()` and a three-state machine (`IDLE`, `PREPARE_MSG`, `SENDING`)
- Burst retransmission: every frame is sent 3 times at 100 ms intervals
- Monotonic sequence counter so the receiver can discard duplicate frames
- Transmitter identifier embedded in each frame for basic addressing

## Hardware

| Component | Notes |
| --- | --- |
| Arduino Uno / Nano (ATmega328P) | Any board supported by RadioHead works |
| 433 MHz ASK transmitter module (FS1000A or equivalent) | Data pin on D12 (RadioHead `RH_ASK` default) |
| 4 push buttons | D2, D3, D4, D5, wired to GND (internal pull-ups enabled) |

## Frame format

```
<TX_ID>|B<n>|<ON|OFF>|<seq>
```

Example: `TX1|B3|ON|42` means button 3 on transmitter `TX1` was pressed, sequence number 42. A pressed button (pin pulled `LOW`) is reported as `ON`, a released button as `OFF`.

## Configuration

All tunables are constants at the top of `EMETTEUR_cmd_relais.ino`:

| Constant | Default | Purpose |
| --- | --- | --- |
| `boutonPins` | `{2, 3, 4, 5}` | Input pins |
| `DEBOUNCE_DELAY` | `50` ms | Debounce window |
| `EMETTEUR_ID` | `"TX1"` | Identifier checked by the receiver |
| `RESEND_INTERVAL_MS` | `100` ms | Delay between retransmissions |
| `MAX_RESEND_ATTEMPTS` | `3` | Copies sent per event |

## Build and upload

1. Install the [RadioHead](https://www.airspayce.com/mikem/arduino/RadioHead/) library.
2. Open `EMETTEUR_cmd_relais.ino` in the Arduino IDE.
3. Select the board and port, then upload.
4. Open the serial monitor at 9600 baud to follow the frames being sent.

## License

No license has been specified yet. Contact the author before reusing this code.
