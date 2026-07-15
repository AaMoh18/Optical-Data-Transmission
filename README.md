# Optical-Data-Transmission (Li-Fi)

This repository contains the source code for a wireless optical communication link utilizing On-Off Keying (OOK) packet transmission. The architecture utilizes an ESP8266 transmitter to encode and send string payloads via infrared light, paired with an ESP32 receiver designed to filter background noise and reconstruct the data stream.

---

## 📊 Technical Specifications & Parameters

| Parameter | Value | System Component |
| :--- | :--- | :--- |
| **Transmitter Pin** | Pin 5 (`LED_PIN` / D1) | ESP8266 Transmitter |
| **Receiver Pin** | Pin 35 (`SENSOR_PIN`) | ESP32 Receiver |
| **Sampling Interval** | 500 µs (`timeBetweenSamples`) | ESP32 Receiver |
| **DSP Integration Window** | 1,000 samples (500 ms duration) | ESP32 Receiver |
| **Signal Thresholds** | 200 (Analog value baseline) | ESP32 Receiver |
| **Data Bit Duration** | 500 ms (`bit_duration`) | Both Units |
| **Packet Framing Markers** | `@` (`START_MARKER` / `END_MARKER`) | Both Units |

---

## 📤 Transmitter Protocol (`esp8266tT.ino`)

The transmitter processes the text string `"Optical Data Transmission"` by enclosing the payload within starting and ending `@` markers. Each character is systematically converted into a binary format and transmitted **Most Significant Bit (MSB) first** using specific hardware delays:

1. **Character Sync Pulse:** The LED pin is driven `HIGH` for **1,500 ms** to alert the receiver that a byte transmission is beginning.
2. **Separation Gap:** The LED is driven `LOW` for **500 ms** to clear the channel immediately following the sync pulse.
3. **8-Bit Character Payload:** The individual bits are shifted out sequentially:
   * **Bit 1:** LED pin driven `HIGH` for 500 ms.
   * **Bit 0:** LED pin driven `LOW` for 500 ms.
4. **Inter-Packet Delay:** Once the full string packet (concluding with the terminal `@` marker) is sent, the system outputs a low signal and idles for **5,000 ms** before recycling the transmission loop.

---

## 📥 Receiver Architecture & FSM Logic (`esp32R.ino`)

The receiver continuously monitors raw voltage from an analog light sensor. To mitigate ambient light interference and prevent bit-error rates, the firmware implements digital signal processing logic via a **1,000-sample integration window** to compute a stable rolling average light level every 500 ms. 

Data parsing is managed by a three-state **Finite State Machine (FSM)**:

### 1. `WAITING_FOR_START`
* The unit remains idle, checking the 500 ms window averages.
* If the average light level spikes above the threshold of `200`, the FSM confirms a character sync pulse, sets a delay window counter (`windows_to_skip = 3`), and switches to the gap-skipping phase.

### 2. `SKIPPING_GAP`
* The system waits through 3 window cycles to let the transmitter's physical separation gap pass without corrupting data registers.
* Once the skip counter hits zero, it zeroes out its tracking registers and shifts to the active reading state.

### 3. `READING_BITS`
* The receiver checks 8 consecutive 500 ms window averages to reconstruct the full 8-bit byte payload:
  * Average light level > `200` $\rightarrow$ Registered as a `1` bit (written using `bitSet`).
  * Average light level $\le$ `200` $\rightarrow$ Registered as a `0` bit.
* When all 8 bits are read, the byte is cast back into a text character:
  * Catching the initial `@` marker triggers a reset and opens the `messageBuffer`.
  * Normal text characters are incrementally appended to the string buffer.
  * Catching the terminal `@` marker seals the packet, outputs the final reconstructed message to the Serial Monitor at 115200 baud, and resets the machine back to `WAITING_FOR_START`.
