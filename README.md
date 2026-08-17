# Stadia_Pico_PS5

An ultra-low-latency BLE-to-USB bridge firmware for the **Raspberry Pi Pico WH** (RP2040 + CYW43439). 
It connects wirelessly over Bluetooth Low Energy (BLE) to a **Google Stadia Controller** (unlocked to Bluetooth mode) and translates all controller inputs in real-time into USB gamepad profiles supported by the **Brook Wingman P5** adapter (or direct PS5/PC/Nintendo Switch connections).

Includes a built-in **Diagnostic & Button Remapping Web Studio** for real-time latency benchmarking, dual-motor rumble testing, and visual button remapping.

---

## 🌟 Key Features

- **Direct Wireless BLE HOGP Connection**: Connects to the official Google Stadia Controller Bluetooth Mode (VID `0x18D1`, PID `0x9400`) over BLE HID with negotiated low-latency connection intervals (7.5ms–15ms).
- **Fast Auto-Reconnection & Bonding**: Stores BLE Long-Term Keys (LTK) so your Stadia controller reconnects in under 1 second when powered on.
- **Multi-Profile USB Emulation (Brook Wingman P5 Ready)**:
  - **Profile 0: Microsoft Xbox 360 / XInput (Default)** — *Native 1:1 plug-and-play with the Brook Wingman P5 adapter, ensuring zero 8-minute authentication timeouts on PS5 and zero latency.*
  - **Profile 1: Sony PlayStation 4 DualShock 4** — *Recognized natively by Brook Wingman P5 and PS4.*
  - **Profile 2: Nintendo Switch Pro Controller** — *Fully supported by Brook Wingman P5 and Nintendo Switch.*
  - **Profile 3: Sony PlayStation 5 DualSense (USB HID Mode)** — *Standard DualSense HID structure for PC / direct PS5 applications.*
  - **Profile 4: Sony PlayStation 3 DualShock 3 / DInput** — *Legacy DInput compatibility.*
- **Built-in Diagnostic & Remap Studio (`web/`)**:
  - Live visual controller mapping with real-time button highlight and analog stick deflection.
  - Latency and Polling Rate Monitor (Hz and ms jitter).
  - Dual-motor haptic rumble test (Low-frequency heavy motor + High-frequency light motor).
  - Interactive Button Remapper with export to JSON and C Header (`custom_remap.h`).
- **Non-Volatile Profile Memory**: Profiles can be switched on boot and are persisted in RP2040 Flash memory.
- **Haptic Rumble Feedback**: Bi-directional rumble pass-through from the host console/adapter to the Stadia controller's dual haptic motors.
- **CYW43 On-board Status LED**: Visual indicators for scanning, connecting, active streaming, button activity, and pairing resets.
- **Pairing Reset Shortcut**: Hold `Google Assistant` + `Capture` buttons for 3 seconds to unpair and enter pairing mode immediately.

---

## 🕹️ System Architecture

```
+------------------------------------+
|      Google Stadia Controller      |
|    (Bluetooth Mode / Unlocked)     |
+------------------------------------+
                  |
                  | BLE (HID over GATT / Report ID 0x03)
                  v
+-------------------------------------------------------------+
|               Raspberry Pi Pico WH (RP2040)                 |
|                                                             |
|  [ CYW43439 Bluetooth Stack (BTstack HOGP Central) ]        |
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

## 🖥️ Built-in Test & Remap Web App

The repository includes a standalone web testing suite located in [`web/`](web/):

1. Connect your Raspberry Pi Pico WH (with your Stadia controller paired wirelessly) to your PC via USB (or plug your Brook Wingman P5 + Pico into PC).
2. Launch the web app:
   ```bash
   python web/serve.py
   ```
   *Or open [`web/index.html`](web/index.html) directly in any modern browser (Chrome, Edge, Firefox).*
3. Press any button on the controller — the tool will automatically detect the controller, show real-time input responses on the interactive controller graphic, benchmark your polling frequency, test rumble feedback, and allow custom button remapping!

---

## 🚦 Status LED Guide (Pico WH On-board LED)

| LED State | Meaning |
| :--- | :--- |
| **Fast Blink (100ms)** | Scanning for Stadia Controller. |
| **Medium Blink (250ms)** | Connecting & Handshaking with Stadia Controller. |
| **Solid ON** | Connected, Paired, and Streaming to Host. |
| **Solid ON with quick dip** | Active button / joystick motion detected. |
| **Rapid Strobe (50ms)** | Pairing cleared; searching for new devices. |

---

## 🚀 Quick Start Guide

### Step 1: Flash the Firmware to Raspberry Pi Pico WH
1. Hold down the white **BOOTSEL** button on your Raspberry Pi Pico WH.
2. While holding the button, plug the Pico WH into your computer via a Micro-USB cable.
3. Release the button. A drive named **`RPI-RP2`** will appear on your computer.
4. Drag and drop the compiled **`stadia_ps5_bridge.uf2`** file onto the `RPI-RP2` drive.
5. The Pico WH will automatically reboot and start the firmware.

### Step 2: Put Stadia Controller in Pairing Mode
1. Ensure your Stadia Controller has been updated to Bluetooth mode using Google's official tool.
2. Turn on the Stadia Controller by pressing and holding **`Stadia` + `Y`** simultaneously for 2 seconds until the status ring blinks **orange**.
3. Bring the controller near the Pico WH.
4. The Pico WH LED will rapid-blink, find the controller, pair, and turn **solid ON**. The Stadia Controller ring light will turn **solid white**.

### Step 3: Connect to Brook Wingman P5 and PS5
1. Plug the Raspberry Pi Pico WH into the USB-A port of your **Brook Wingman P5**.
2. Plug the Brook Wingman P5 into a USB port on your **PlayStation 5**.
3. Press the **Stadia** button (PS Home button).
4. You are now playing on PS5 with your Stadia Controller!

---

## ⚙️ Profile Switching

The default profile is **Profile 0 (Xbox 360 / XInput)** because it offers 100% compatibility with the Brook Wingman P5 adapter on PS5 without any setup.

To switch profiles:
1. When booting or via software configuration, choose:
   - **Profile 0**: Xbox 360 / XInput *(Recommended for Brook Wingman P5 & PC)*
   - **Profile 1**: PS4 DualShock 4 *(For PS4 & Wingman P5)*
   - **Profile 2**: Nintendo Switch Pro *(For Nintendo Switch & Wingman P5)*
   - **Profile 3**: PS5 DualSense *(Direct USB HID)*
   - **Profile 4**: PS3 DualShock 3 *(Generic DInput)*
2. The selection is automatically written to the RP2040 Flash memory and will remain active across power cycles.

---

## 🛠️ Building from Source

### Prerequisites
- **Raspberry Pi Pico SDK** (v1.5.0 or v2.0.0+)
- **CMake** (v3.13+)
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

#### Manual CMake Build:
```bash
mkdir build
cd build
cmake .. -DPICO_BOARD=pico_w -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```
The output file **`stadia_ps5_bridge.uf2`** will be generated in the `build/` directory.

---

## 🔧 Pairing Reset Shortcut

If you want to pair your Pico WH with a different Stadia Controller:
- While connected, press and hold **Google Assistant (●)** and **Capture ([ ])** together for **3 seconds**.
- The Pico WH will clear stored pairing keys and immediately enter scanning mode (fast LED blink) to connect to any advertising Stadia Controller.

---

## 📄 License
This project is open-source under the MIT License. See [`LICENSE`](LICENSE) for details.
