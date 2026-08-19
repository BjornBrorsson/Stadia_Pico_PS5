# Contributing to Stadia_Pico_PS5

Thank you for your interest in contributing to **Stadia_Pico_PS5**! We welcome all contributions, including bug reports, documentation improvements, feature additions, and custom controller profiles.

---

## 🛠️ Development Setup

### Prerequisites
- **Raspberry Pi Pico W / Pico WH**
- **Google Stadia Controller** (unlocked to Bluetooth mode)
- **ARM GNU Toolchain** (`arm-none-eabi-gcc`)
- **CMake** (v3.13 or newer) and **Ninja** / **Make**
- **Raspberry Pi Pico SDK** (v1.5.0 or newer)
- *(Optional)* **Brook Wingman P5 / FGC** or PlayStation 5 / PC for hardware testing

### Clone & Build

```bash
# 1. Clone the repository
git clone https://github.com/BjornBrorsson/Stadia_Pico_PS5.git
cd Stadia_Pico_PS5

# 2. Build on Windows (PowerShell)
.\build.ps1

# Or on Linux / macOS
chmod +x build.sh
./build.sh
```

The compiled UF2 firmware will be output to `build/stadia_ps5_bridge.uf2`.

---

## 🧪 Testing Your Changes

1. **Firmware Flashing**:
   - Hold the `BOOTSEL` button on the Pico WH while connecting via USB.
   - Copy `build/stadia_ps5_bridge.uf2` to the mounted `RPI-RP2` drive.
2. **Web Testing Studio**:
   - Launch the local diagnostic test studio:
     ```bash
     python web/serve.py
     ```
   - Connect the controller and verify that all inputs, stick deflections, rumble motors, and latency benchmarks behave as expected.

---

## 📋 Pull Request Guidelines

1. **Branch Naming**:
   - `feature/your-feature-name`
   - `fix/issue-description`
   - `docs/what-changed`
2. **Commit Messages**:
   - Use clear, descriptive commit messages (e.g., `fix(ble): resolve GATT service discovery timeout on reconnect`).
3. **Code Style**:
   - Maintain C11 standard formatting.
   - Use descriptive variable and function names.
   - Keep TinyUSB and BTstack callback logic non-blocking.
4. **Testing**:
   - Ensure the project builds cleanly without warnings (`cmake --build build`).
   - Test on physical hardware whenever possible.

---

## 💡 Submitting Issues

- Use the GitHub Issue Tracker to report bugs or request features.
- When filing a bug report, please provide:
  - Controller model & firmware version.
  - Target host (PS5 + Brook Wingman P5, direct PC, Switch, etc.).
  - LED behavior on both the Pico WH and the controller.
  - Steps to reproduce the issue.
