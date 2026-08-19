# Stadia_Pico_PS5

[![Build Firmware & Release](https://github.com/BjornBrorsson/Stadia_Pico_PS5/actions/workflows/build.yml/badge.svg)](https://github.com/BjornBrorsson/Stadia_Pico_PS5/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Raspberry%20Pi%20Pico%20W%20%2F%20WH-blue.svg)](https://www.raspberrypi.com/products/raspberry-pi-pico/)
[![Hardware Support](https://img.shields.io/badge/Supports-PS5%20%7C%20Brook%20Wingman%20P5%20%7C%20PC%20%7C%20Switch-green.svg)](https://www.brookaccessory.com/)

An ultra-low-latency, zero-timeout wireless **BLE-to-USB gamepad bridge** for the **Raspberry Pi Pico W / Pico WH** (RP2040 + CYW43439). 

It connects wirelessly over Bluetooth Low Energy (BLE) to an official **Google Stadia Controller** (unlocked to Bluetooth mode) using native BTstack HOGP Central architecture, translating all analog sticks, triggers, buttons, and haptics into real-time USB gamepad profiles for the **Brook Wingman P5 / FGC** adapter (for PS5 gaming), direct PC, and Nintendo Switch.

Includes a standalone **Diagnostic & Button Remapping Web Studio** for real-time latency benchmarking, dual-motor rumble testing, and visual button remapping.

---

## 🌟 Key Features

- **Native BLE HOGP Stack**: Uses BTstack's official `hids_host` client to connect to the Google Stadia Controller (VID `0x18D1`, PID `0x9400`) over low-latency negotiated connection intervals (7.5ms–15ms).
- **Fast Auto-Reconnection & Bonding**: Stores Bluetooth Security Manager (SM) bonding keys and LTKs so your Stadia controller reconnects in under 1 second when powered on.
- **Multi-Profile USB Emulation (Brook Wingman P5 Ready)**:
  - **Profile 0: Microsoft Xbox 360 / XInput (Default)** — *Native 1:1 plug-and-play with Brook Wingman P5 adapter for PlayStation 5 gaming with zero 8-minute authentication timeouts and zero input latency.*
  - **Profile 1: Sony PlayStation 4 DualShock 4** — *Recognized natively by PS4 and Brook Wingman P5.*
  - **Profile 2: Nintendo Switch Pro Controller** — *Supported by Nintendo Switch and Brook converters.*
  - **Profile 3: Sony PlayStation 5 DualSense (USB HID Mode)** — *Standard DualSense HID structure for PC and direct PS5 applications.*
  - **Profile 4: Sony PlayStation 3 DualShock 3 / DInput** — *Legacy DInput compatibility.*
- **Bi-Directional Haptic Rumble**: Passes host rumble motor commands directly to the Stadia controller's dual haptic motors.
- **Persistent Profile Storage**: Switch profiles on-the-fly with controller shortcuts; selections are saved directly to RP2040 Flash memory.
- **Built-in Diagnostic & Remap Web Studio (`web/`)**:
  - Real-time visual controller mapping and analog stick deflection display.
  - Live polling frequency (Hz) and jitter (ms) monitor.
  - Dual-motor haptic rumble test tool.
  - Interactive button remapper with JSON and C header export (`custom_remap.h`).
- **Status LED Engine**: Visual feedback for scanning, handshaking, active streaming, and pairing resets.

---

## 🕹️ System Architecture

```
+------------------------------------+
|      Google Stadia Controller      |
|    (Bluetooth Mode / Unlocked)     |
+------------------------------------+
                  |
                  | Wireless BLE HOGP (Report ID 0x03 & 0x05)
                  v
+-------------------------------------------------------------+
|               Raspberry Pi Pico WH (RP2040)                 |
|                                                             |
|  [ CYW43439 Bluetooth Stack (BTstack HIDS Host Central) ]   |
|                             |                               |
|                             v                               |
|         [ Real-time Input Mapper & Normalizer ]             |
|        - Analog Sticks (Centered 128 -> 16-bit)             |
|        - Analog Triggers (0-255)                            |
|        - D-Pad (8-Way Hat -> Directional Bitmask)           |
|        - Face & Shoulder Buttons                            |
|        - Assistant / Capture (Touchpad / Mute)              |
|                             |                               |
|                             v                               |
|         [ TinyUSB Multi-Profile Output Engine ]             |
+-------------------------------------------------------------+
                  |
                  | USB Cable (Micro-USB from Pico WH)
                  v
+------------------------------------+
|       Brook Wingman P5 Adapter     |
|   (Provides Hardware PS5 Auth IC)  |
+------------------------------------+
                  |
                  | USB-A Direct
                  v
+------------------------------------+
|        Sony PlayStation 5          |
+------------------------------------+
```

---

## 🎮 Button Mapping Matrix

| Stadia Controller Button | PS5 / DualSense Equivalent | Xbox 360 Equivalent (Wingman P5) | Switch Pro Equivalent |
| :--- | :--- | :--- | :--- |
| **A** | **Cross (✕)** | **A** | **B** |
| **B** | **Circle (◯)** | **B** | **A** |
| **X** | **Square (▢)** | **X** | **Y** |
| **Y** | **Triangle (△)** | **Y** | **X** |
| **L1** (Left Bumper) | **L1** | **Left Bumper (LB)** | **L** |
| **R1** (Right Bumper) | **R1** | **Right Bumper (RB)** | **R** |
| **L2** (Left Trigger) | **L2 (Analog 0-255)** | **Left Trigger (LT)** | **ZL** |
| **R2** (Right Trigger) | **R2 (Analog 0-255)** | **Right Trigger (RT)** | **ZR** |
| **L3** (Left Stick Click) | **L3** | **Left Stick (LS)** | **Left Stick Click** |
| **R3** (Right Stick Click) | **R3** | **Right Stick (RS)** | **Right Stick Click** |
| **D-Pad** (Up/Down/Left/Right) | **D-Pad** | **D-Pad** | **D-Pad** |
| **Stadia Button** | **PS Button (Home)** | **Xbox Guide Button** | **Home** |
| **Menu Button** (☰) | **Options** | **Start** | **Plus (+)** |
| **Options Button** (...) | **Create / Share** | **Back** | **Minus (-)** |
| **Google Assistant** (●) | **Touchpad Click** | **Back / Aux** | **Capture** |
| **Capture Button** ([ ]) | **Mute / Mic Toggle** | **Guide / Aux** | **Capture** |

---

## 🚀 Quick Start Guide

### Step 1: Unlock Google Stadia Controller Bluetooth Mode
1. Ensure your Stadia controller is updated to Bluetooth mode via Google's official tool: [Google Stadia Bluetooth Tool](https://stadia.google.com/controller).
2. To verify, turn it on by holding **`Stadia` + `Y`** for 2 seconds. The LED ring will blink **orange** (Pairing mode).

### Step 2: Flash Firmware to Raspberry Pi Pico W / WH
1. Download the latest **`stadia_ps5_bridge.uf2`** from the [GitHub Releases](https://github.com/BjornBrorsson/Stadia_Pico_PS5/releases) page (or build from source).
2. Hold down the white **BOOTSEL** button on your Pico WH while plugging it into your computer via Micro-USB.
3. Release the button. A drive named **`RPI-RP2`** will appear in File Explorer.
4. Drag and drop **`stadia_ps5_bridge.uf2`** onto the `RPI-RP2` drive.
5. The Pico WH will automatically reboot and start scanning for your Stadia controller.

### Step 3: Pair Controller to Pico WH
1. Put the Stadia controller in pairing mode by holding **`Stadia` + `Y`** until the light blinks **orange**.
2. Bring the controller near the Pico WH.
3. The Pico WH LED will turn **solid green**, and the Stadia Controller ring light will turn **solid white**.
4. Once paired, the controller will automatically reconnect in under 1 second every time you turn it on (just press the **Stadia** button).

### Step 4: Connect to Brook Wingman P5 & PS5
1. Plug the Raspberry Pi Pico WH into the USB-A port of your **Brook Wingman P5**.
2. Plug the Brook Wingman P5 into a USB port on your **PlayStation 5** (or PC).
3. The Wingman LED will turn **solid blue**, indicating a successful controller handshake.
4. Press the **Stadia** button (PS Home) and enjoy gaming!

---

## ⚙️ Profile Switching

The firmware defaults to **Profile 0 (Xbox 360 / XInput)** for plug-and-play compatibility with the Brook Wingman P5 adapter and PC.

To switch profiles on-the-fly, hold **Google Assistant (●)** + face button for **2 seconds**:

- **`Assistant (●)` + `A` (Cross)** &rarr; **Profile 0: Xbox 360 / XInput** *(Recommended for Brook Wingman P5 & PC)*
- **`Assistant (●)` + `X` (Square)** &rarr; **Profile 1: PS4 DualShock 4** *(For PS4 & Wingman P5)*
- **`Assistant (●)` + `B` (Circle)** &rarr; **Profile 2: Nintendo Switch Pro** *(For Nintendo Switch & Wingman P5)*
- **`Assistant (●)` + `Y` (Triangle)** &rarr; **Profile 3: PS5 DualSense** *(Direct USB HID)*

The Pico WH will persist your selection to RP2040 Flash memory and reboot with the new profile active.

---

## 🚦 Status LED Guide

### Raspberry Pi Pico W / WH LED
| LED State | Meaning |
| :--- | :--- |
| **Fast Blink (100ms)** | Scanning for Stadia Controller. |
| **Medium Blink (250ms)** | Connecting & Handshaking with Stadia Controller. |
| **Solid ON (Green)** | Connected, Paired, and Streaming inputs. |
| **Solid ON with quick dip** | Active button press or analog joystick movement. |
| **Rapid Strobe (50ms)** | Stored bonding keys cleared; searching for new devices. |

### Brook Wingman P5 LED
| LED State | Meaning |
| :--- | :--- |
| **Blinking Blue** | Powered on, searching for controller. |
| **Solid Blue** | Connected and communicating with Pico WH / Controller. |
| **Solid Purple** | Firmware Update / ISP Bootloader mode. *(See Troubleshooting below)* |
| **Solid Red** | Tournament Mode / direct lockout profile. |

---

## 🖥️ Built-in Test & Remap Web App

The repository includes a standalone web testing suite located in [`web/`](web/):

1. Connect your Raspberry Pi Pico WH (with your Stadia controller paired wirelessly) to your PC via USB.
2. Launch the web app:
   ```bash
   python web/serve.py
   ```
   *Or open [`web/index.html`](web/index.html) directly in Google Chrome, Edge, or Firefox.*
3. **Press any button on the controller** to activate the browser Gamepad API.
4. The studio displays real-time input responses, analog stick deflection, polling frequency benchmarks (Hz), rumble motor testing, and custom remapping export.

---

## ❓ Troubleshooting FAQ

### 1. The Brook Wingman P5 adapter stays solid purple
- **Cause**: A solid purple LED indicates the Brook adapter is in **Nuvoton ISP Bootloader / Firmware Update mode**. This happens if an update was interrupted (common when updating through multi-port USB-C hubs) or if the adapter has not been power-cycled.
- **Fix**:
  1. Unplug the Wingman from your computer / hub.
  2. Use a direct USB-A port or the official simple USB-C OTG adapter.
  3. Open Google Chrome and visit the [Brook Official Web Updater](https://www.brookaccessory.com/BrookUpdate/index.html).
  4. Complete the firmware flash (wait for 100%), then unplug the adapter and plug it back in without holding any buttons. The LED will turn blue.

### 2. Controller is connected (white LED), but PC / joy.cpl shows no button input
- **Cause**: The browser Gamepad API requires at least one physical button press after page load to register the gamepad for privacy reasons.
- **Fix**: Press any face button (A/B/X/Y) or move the analog sticks once to wake the Gamepad API context.

### 3. How to unpair and pair to a different Stadia Controller
- Hold **Google Assistant (●)** + **Capture ([ ])** together for **3 seconds**.
- The Pico WH will clear stored bonding keys from non-volatile storage and enter fast-scanning mode (fast blinking LED).

---

## 🛠️ Building from Source

### Prerequisites
- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) (v1.5.0 or v2.0.0+)
- **CMake** (v3.13+) and **Ninja** / **Make**
- **ARM GNU Toolchain** (`arm-none-eabi-gcc`)

### Build Commands

#### Windows (PowerShell):
```powershell
.\build.ps1
```

#### Linux / macOS (Bash):
```bash
chmod +x build.sh
./build.sh
```

#### Manual CMake:
```bash
mkdir build && cd build
cmake .. -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```
The output file **`stadia_ps5_bridge.uf2`** will be generated in `build/`.

---

## 🤝 Contributing

Contributions are welcome! Please read [`CONTRIBUTING.md`](CONTRIBUTING.md) for details on code style, testing procedures, and the pull request process.

---

## 📄 License

This project is open-source under the **MIT License**. See [`LICENSE`](LICENSE) for details.
