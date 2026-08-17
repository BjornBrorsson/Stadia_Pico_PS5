// Stadia to PS5 / Brook Wingman P5 Test & Remap Studio

// Standard Gamepad Map Indexes
const STANDARD_BUTTONS = [
  { id: 'btn-a', name: 'A / Cross (✕)' },
  { id: 'btn-b', name: 'B / Circle (◯)' },
  { id: 'btn-x', name: 'X / Square (▢)' },
  { id: 'btn-y', name: 'Y / Triangle (△)' },
  { id: 'btn-l1', name: 'L1 / Left Bumper (LB)' },
  { id: 'btn-r1', name: 'R1 / Right Bumper (RB)' },
  { id: 'btn-l2', name: 'L2 / Left Trigger (LT)' },
  { id: 'btn-r2', name: 'R2 / Right Trigger (RT)' },
  { id: 'btn-select', name: 'Options (...) / Share / Back' },
  { id: 'btn-start', name: 'Menu (☰) / Options / Start' },
  { id: 'btn-l-cap', name: 'L3 (Left Stick Click)' },
  { id: 'btn-r-cap', name: 'R3 (Right Stick Click)' },
  { id: 'btn-dpad-up', name: 'D-Pad Up' },
  { id: 'btn-dpad-down', name: 'D-Pad Down' },
  { id: 'btn-dpad-left', name: 'D-Pad Left' },
  { id: 'btn-dpad-right', name: 'D-Pad Right' },
  { id: 'btn-home', name: 'Stadia / PS / Guide Button' },
  { id: 'btn-touchpad', name: 'Touchpad Click / Assistant (●)' },
  { id: 'btn-capture', name: 'Capture ([ ]) / Mute' }
];

const AVAILABLE_ACTIONS = [
  { value: 'CROSS_A', label: 'Cross (✕) / A' },
  { value: 'CIRCLE_B', label: 'Circle (◯) / B' },
  { value: 'SQUARE_X', label: 'Square (▢) / X' },
  { value: 'TRIANGLE_Y', label: 'Triangle (△) / Y' },
  { value: 'L1', label: 'L1 / Left Bumper' },
  { value: 'R1', label: 'R1 / Right Bumper' },
  { value: 'L2', label: 'L2 / Left Trigger' },
  { value: 'R2', label: 'R2 / Right Trigger' },
  { value: 'SHARE_BACK', label: 'Create / Share / Back' },
  { value: 'OPTIONS_START', label: 'Options / Start' },
  { value: 'L3', label: 'L3 / Left Stick Click' },
  { value: 'R3', label: 'R3 / Right Stick Click' },
  { value: 'HOME_GUIDE', label: 'PS Home / Stadia Guide' },
  { value: 'TOUCHPAD_ASSIST', label: 'Touchpad Click / Assistant' },
  { value: 'CAPTURE_MUTE', label: 'Mic Mute / Capture' },
  { value: 'DPAD_UP', label: 'D-Pad Up' },
  { value: 'DPAD_DOWN', label: 'D-Pad Down' },
  { value: 'DPAD_LEFT', label: 'D-Pad Left' },
  { value: 'DPAD_RIGHT', label: 'D-Pad Right' },
  { value: 'NONE', label: 'Disabled / None' }
];

// Default Mapping Table
let currentMapping = {
  'A': 'CROSS_A',
  'B': 'CIRCLE_B',
  'X': 'SQUARE_X',
  'Y': 'TRIANGLE_Y',
  'L1': 'L1',
  'R1': 'R1',
  'L2': 'L2',
  'R2': 'R2',
  'OPTIONS': 'SHARE_BACK',
  'MENU': 'OPTIONS_START',
  'L3': 'L3',
  'R3': 'R3',
  'STADIA': 'HOME_GUIDE',
  'ASSISTANT': 'TOUCHPAD_ASSIST',
  'CAPTURE': 'CAPTURE_MUTE',
  'DPAD_UP': 'DPAD_UP',
  'DPAD_DOWN': 'DPAD_DOWN',
  'DPAD_LEFT': 'DPAD_LEFT',
  'DPAD_RIGHT': 'DPAD_RIGHT'
};

// State Variables
let activeGamepadIndex = null;
let lastTimestamp = 0;
let frameCount = 0;
let lastRateCheck = performance.now();
let currentPollingRate = 0;
let activeVibrationEffect = null;

// DOM Elements
const statusDot = document.getElementById('statusDot');
const statusText = document.getElementById('statusText');
const detectedProfileBadge = document.getElementById('detectedProfileBadge');
const pollingRateVal = document.getElementById('pollingRateVal');
const latencyVal = document.getElementById('latencyVal');
const rateBadge = document.getElementById('rateBadge');
const activeBtnsCount = document.getElementById('activeBtnsCount');
const reportTypeVal = document.getElementById('reportTypeVal');

const gaugeL2 = document.getElementById('gaugeL2');
const gaugeR2 = document.getElementById('gaugeR2');
const valL2 = document.getElementById('valL2');
const valR2 = document.getElementById('valR2');
const valLX = document.getElementById('valLX');
const valLY = document.getElementById('valLY');
const valRX = document.getElementById('valRX');
const valRY = document.getElementById('valRY');

const stickLCap = document.getElementById('stick-l-cap');
const stickLLine = document.getElementById('stick-l-line');
const stickRCap = document.getElementById('stick-r-cap');
const stickRLine = document.getElementById('stick-r-line');

// Initialize Remapper Table
function initRemapTable() {
  const tbody = document.getElementById('remapTableBody');
  tbody.innerHTML = '';

  const entries = [
    { key: 'A', label: 'Stadia A Button' },
    { key: 'B', label: 'Stadia B Button' },
    { key: 'X', label: 'Stadia X Button' },
    { key: 'Y', label: 'Stadia Y Button' },
    { key: 'L1', label: 'Stadia L1 (Left Bumper)' },
    { key: 'R1', label: 'Stadia R1 (Right Bumper)' },
    { key: 'L2', label: 'Stadia L2 (Left Trigger)' },
    { key: 'R2', label: 'Stadia R2 (Right Trigger)' },
    { key: 'OPTIONS', label: 'Stadia Options (...) Button' },
    { key: 'MENU', label: 'Stadia Menu (☰) Button' },
    { key: 'L3', label: 'Stadia Left Stick Click (L3)' },
    { key: 'R3', label: 'Stadia Right Stick Click (R3)' },
    { key: 'STADIA', label: 'Stadia Home Button' },
    { key: 'ASSISTANT', label: 'Stadia Assistant (●) Button' },
    { key: 'CAPTURE', label: 'Stadia Capture ([ ]) Button' },
    { key: 'DPAD_UP', label: 'Stadia D-Pad Up' },
    { key: 'DPAD_DOWN', label: 'Stadia D-Pad Down' },
    { key: 'DPAD_LEFT', label: 'Stadia D-Pad Left' },
    { key: 'DPAD_RIGHT', label: 'Stadia D-Pad Right' }
  ];

  entries.forEach(entry => {
    const tr = document.createElement('tr');
    const tdInput = document.createElement('td');
    tdInput.innerHTML = `<strong>${entry.label}</strong>`;
    
    const tdOutput = document.createElement('td');
    const select = document.createElement('select');
    select.className = 'remap-select';
    select.dataset.key = entry.key;

    AVAILABLE_ACTIONS.forEach(action => {
      const opt = document.createElement('option');
      opt.value = action.value;
      opt.textContent = action.label;
      if (currentMapping[entry.key] === action.value) {
        opt.selected = true;
      }
      select.appendChild(opt);
    });

    select.addEventListener('change', (e) => {
      currentMapping[entry.key] = e.target.value;
      document.getElementById('presetSelect').value = 'custom';
    });

    tdOutput.appendChild(select);
    tr.appendChild(tdInput);
    tr.appendChild(tdOutput);
    tbody.appendChild(tr);
  });
}

// Preset Handler
document.getElementById('presetSelect').addEventListener('change', (e) => {
  const preset = e.target.value;
  if (preset === 'default_ps5') {
    currentMapping = {
      'A': 'CROSS_A', 'B': 'CIRCLE_B', 'X': 'SQUARE_X', 'Y': 'TRIANGLE_Y',
      'L1': 'L1', 'R1': 'R1', 'L2': 'L2', 'R2': 'R2',
      'OPTIONS': 'SHARE_BACK', 'MENU': 'OPTIONS_START',
      'L3': 'L3', 'R3': 'R3', 'STADIA': 'HOME_GUIDE',
      'ASSISTANT': 'TOUCHPAD_ASSIST', 'CAPTURE': 'CAPTURE_MUTE',
      'DPAD_UP': 'DPAD_UP', 'DPAD_DOWN': 'DPAD_DOWN', 'DPAD_LEFT': 'DPAD_LEFT', 'DPAD_RIGHT': 'DPAD_RIGHT'
    };
  } else if (preset === 'swap_bumpers') {
    currentMapping['L1'] = 'L2';
    currentMapping['L2'] = 'L1';
    currentMapping['R1'] = 'R2';
    currentMapping['R2'] = 'R1';
  } else if (preset === 'southpaw') {
    currentMapping['L3'] = 'R3';
    currentMapping['R3'] = 'L3';
  } else if (preset === 'arcade_fight') {
    currentMapping['X'] = 'SQUARE_X';
    currentMapping['Y'] = 'TRIANGLE_Y';
    currentMapping['R1'] = 'R1';
    currentMapping['A'] = 'CROSS_A';
    currentMapping['B'] = 'CIRCLE_B';
    currentMapping['R2'] = 'R2';
  }
  initRemapTable();
});

// Export JSON
document.getElementById('btnExportJson').addEventListener('click', () => {
  const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(currentMapping, null, 2));
  const downloadAnchor = document.createElement('a');
  downloadAnchor.setAttribute("href", dataStr);
  downloadAnchor.setAttribute("download", "stadia_remap_profile.json");
  document.body.appendChild(downloadAnchor);
  downloadAnchor.click();
  downloadAnchor.remove();
});

// Export C Header for Firmware
document.getElementById('btnExportCHeader').addEventListener('click', () => {
  let cHeader = `/* Auto-generated by Stadia-Pico-PS5 Test & Remap Studio */\n#ifndef _CUSTOM_REMAP_H_\n#define _CUSTOM_REMAP_H_\n\n`;
  cHeader += `// Custom Button Mapping Constants\n`;
  for (const [key, val] of Object.entries(currentMapping)) {
    cHeader += `#define MAP_${key.padEnd(12)} ACTION_${val}\n`;
  }
  cHeader += `\n#endif /* _CUSTOM_REMAP_H_ */\n`;

  const dataStr = "data:text/plain;charset=utf-8," + encodeURIComponent(cHeader);
  const downloadAnchor = document.createElement('a');
  downloadAnchor.setAttribute("href", dataStr);
  downloadAnchor.setAttribute("download", "custom_remap.h");
  document.body.appendChild(downloadAnchor);
  downloadAnchor.click();
  downloadAnchor.remove();
});

// Import JSON
document.getElementById('fileImportJson').addEventListener('change', (e) => {
  const file = e.target.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = (event) => {
    try {
      const imported = JSON.parse(event.target.result);
      currentMapping = { ...currentMapping, ...imported };
      initRemapTable();
      document.getElementById('presetSelect').value = 'custom';
    } catch (err) {
      alert("Invalid JSON file!");
    }
  };
  reader.readAsText(file);
});

// Reset Defaults
document.getElementById('btnResetDefaults').addEventListener('click', () => {
  document.getElementById('presetSelect').value = 'default_ps5';
  document.getElementById('presetSelect').dispatchEvent(new Event('change'));
});

// Gamepad Connection Event Listeners
window.addEventListener("gamepadconnected", (e) => {
  console.log("Gamepad connected:", e.gamepad);
  activeGamepadIndex = e.gamepad.index;
  updateConnectionStatus(e.gamepad);
});

window.addEventListener("gamepaddisconnected", (e) => {
  console.log("Gamepad disconnected:", e.gamepad);
  if (activeGamepadIndex === e.gamepad.index) {
    activeGamepadIndex = null;
    updateConnectionStatus(null);
  }
});

function updateConnectionStatus(gp) {
  if (gp) {
    statusDot.className = 'status-dot connected';
    statusText.textContent = `Connected: ${gp.id.substring(0, 24)}...`;
    
    // Identify profile
    const idLower = gp.id.toLowerCase();
    if (idLower.includes('x-box') || idLower.includes('xbox') || idLower.includes('045e') || idLower.includes('028e')) {
      detectedProfileBadge.textContent = 'Brook Wingman P5 / XInput';
      detectedProfileBadge.className = 'badge badge-success';
    } else if (idLower.includes('054c') && idLower.includes('05c4')) {
      detectedProfileBadge.textContent = 'PS4 DualShock 4';
      detectedProfileBadge.className = 'badge badge-success';
    } else if (idLower.includes('054c') && idLower.includes('0ce6')) {
      detectedProfileBadge.textContent = 'PS5 DualSense';
      detectedProfileBadge.className = 'badge badge-success';
    } else if (idLower.includes('switch') || idLower.includes('057e')) {
      detectedProfileBadge.textContent = 'Nintendo Switch Pro';
      detectedProfileBadge.className = 'badge badge-success';
    } else if (idLower.includes('stadia') || idLower.includes('18d1')) {
      detectedProfileBadge.textContent = 'Stadia Controller (Direct)';
      detectedProfileBadge.className = 'badge badge-success';
    } else {
      detectedProfileBadge.textContent = 'Pico Gamepad Device';
      detectedProfileBadge.className = 'badge badge-success';
    }
  } else {
    statusDot.className = 'status-dot disconnected';
    statusText.textContent = 'Waiting for Controller...';
    detectedProfileBadge.textContent = 'No Device';
    detectedProfileBadge.className = 'badge';
    rateBadge.textContent = '0 Hz';
    pollingRateVal.textContent = '-- Hz';
    latencyVal.textContent = '-- ms';
  }
}

// Visualizer Highlight Helper
function setBtnActive(id, isActive) {
  const el = document.getElementById(id);
  if (el) {
    if (isActive) el.classList.add('active');
    else el.classList.remove('active');
  }
}

// Main Polling Loop
function updateGamepadState() {
  const gamepads = navigator.getGamepads ? navigator.getGamepads() : [];
  let gp = null;

  if (activeGamepadIndex !== null && gamepads[activeGamepadIndex]) {
    gp = gamepads[activeGamepadIndex];
  } else {
    for (let i = 0; i < gamepads.length; i++) {
      if (gamepads[i]) {
        gp = gamepads[i];
        activeGamepadIndex = i;
        updateConnectionStatus(gp);
        break;
      }
    }
  }

  if (gp) {
    // Benchmark calculations
    frameCount++;
    const now = performance.now();
    const delta = now - lastTimestamp;
    lastTimestamp = now;

    if (now - lastRateCheck >= 500) {
      currentPollingRate = Math.round((frameCount * 1000) / (now - lastRateCheck));
      pollingRateVal.textContent = `${currentPollingRate} Hz`;
      rateBadge.textContent = `${currentPollingRate} Hz`;
      latencyVal.textContent = `${delta.toFixed(1)} ms`;
      frameCount = 0;
      lastRateCheck = now;
    }

    let activeCount = 0;

    // Face Buttons
    const btnA = gp.buttons[0] ? gp.buttons[0].pressed : false;
    const btnB = gp.buttons[1] ? gp.buttons[1].pressed : false;
    const btnX = gp.buttons[2] ? gp.buttons[2].pressed : false;
    const btnY = gp.buttons[3] ? gp.buttons[3].pressed : false;

    setBtnActive('btn-a', btnA);
    setBtnActive('btn-b', btnB);
    setBtnActive('btn-x', btnX);
    setBtnActive('btn-y', btnY);
    if (btnA) activeCount++;
    if (btnB) activeCount++;
    if (btnX) activeCount++;
    if (btnY) activeCount++;

    // Bumpers
    const btnL1 = gp.buttons[4] ? gp.buttons[4].pressed : false;
    const btnR1 = gp.buttons[5] ? gp.buttons[5].pressed : false;
    setBtnActive('btn-l1', btnL1);
    setBtnActive('btn-r1', btnR1);
    if (btnL1) activeCount++;
    if (btnR1) activeCount++;

    // Triggers (Analog & Digital)
    const valL2Val = gp.buttons[6] ? gp.buttons[6].value : 0;
    const valR2Val = gp.buttons[7] ? gp.buttons[7].value : 0;
    setBtnActive('btn-l2', valL2Val > 0.1);
    setBtnActive('btn-r2', valR2Val > 0.1);
    gaugeL2.style.width = `${Math.round(valL2Val * 100)}%`;
    gaugeR2.style.width = `${Math.round(valR2Val * 100)}%`;
    valL2.textContent = `${Math.round(valL2Val * 100)}%`;
    valR2.textContent = `${Math.round(valR2Val * 100)}%`;
    if (valL2Val > 0.1) activeCount++;
    if (valR2Val > 0.1) activeCount++;

    // System & Center Buttons
    const btnSelect = gp.buttons[8] ? gp.buttons[8].pressed : false;
    const btnStart = gp.buttons[9] ? gp.buttons[9].pressed : false;
    const btnL3 = gp.buttons[10] ? gp.buttons[10].pressed : false;
    const btnR3 = gp.buttons[11] ? gp.buttons[11].pressed : false;
    const btnHome = gp.buttons[16] ? gp.buttons[16].pressed : false;
    const btnTouchpad = gp.buttons[17] ? gp.buttons[17].pressed : false;

    setBtnActive('btn-select', btnSelect);
    setBtnActive('btn-start', btnStart);
    setBtnActive('btn-l-cap', btnL3);
    setBtnActive('btn-r-cap', btnR3);
    setBtnActive('btn-home', btnHome);
    setBtnActive('btn-touchpad', btnTouchpad);
    if (btnSelect) activeCount++;
    if (btnStart) activeCount++;
    if (btnL3) activeCount++;
    if (btnR3) activeCount++;
    if (btnHome) activeCount++;
    if (btnTouchpad) activeCount++;

    // D-Pad
    const dUp = gp.buttons[12] ? gp.buttons[12].pressed : false;
    const dDown = gp.buttons[13] ? gp.buttons[13].pressed : false;
    const dLeft = gp.buttons[14] ? gp.buttons[14].pressed : false;
    const dRight = gp.buttons[15] ? gp.buttons[15].pressed : false;

    setBtnActive('btn-dpad-up', dUp);
    setBtnActive('btn-dpad-down', dDown);
    setBtnActive('btn-dpad-left', dLeft);
    setBtnActive('btn-dpad-right', dRight);
    if (dUp) activeCount++;
    if (dDown) activeCount++;
    if (dLeft) activeCount++;
    if (dRight) activeCount++;

    // Analog Sticks Deflection
    const lx = gp.axes[0] || 0;
    const ly = gp.axes[1] || 0;
    const rx = gp.axes[2] || 0;
    const ry = gp.axes[3] || 0;

    valLX.textContent = lx.toFixed(2);
    valLY.textContent = ly.toFixed(2);
    valRX.textContent = rx.toFixed(2);
    valRY.textContent = ry.toFixed(2);

    const maxStickDeflect = 24; // pixels in SVG
    const leftPxX = lx * maxStickDeflect;
    const leftPxY = ly * maxStickDeflect;
    stickLCap.setAttribute('cx', leftPxX);
    stickLCap.setAttribute('cy', leftPxY);
    stickLLine.setAttribute('x2', leftPxX);
    stickLLine.setAttribute('y2', leftPxY);

    const rightPxX = rx * maxStickDeflect;
    const rightPxY = ry * maxStickDeflect;
    stickRCap.setAttribute('cx', rightPxX);
    stickRCap.setAttribute('cy', rightPxY);
    stickRLine.setAttribute('x2', rightPxX);
    stickRLine.setAttribute('y2', rightPxY);

    activeBtnsCount.textContent = activeCount;
  }

  requestAnimationFrame(updateGamepadState);
}

// Rumble Test Handlers
async function triggerRumble(durationMs, weakMagnitude, strongMagnitude) {
  const gamepads = navigator.getGamepads ? navigator.getGamepads() : [];
  if (activeGamepadIndex !== null && gamepads[activeGamepadIndex]) {
    const gp = gamepads[activeGamepadIndex];
    if (gp.vibrationActuator && gp.vibrationActuator.playEffect) {
      try {
        await gp.vibrationActuator.playEffect('dual-rumble', {
          startDelay: 0,
          duration: durationMs,
          weakMagnitude: weakMagnitude,
          strongMagnitude: strongMagnitude
        });
      } catch (e) {
        console.warn("Vibration not supported or blocked:", e);
      }
    }
  }
}

document.getElementById('sliderLowFreq').addEventListener('input', (e) => {
  document.getElementById('lowFreqVal').textContent = `${e.target.value}%`;
});

document.getElementById('sliderHighFreq').addEventListener('input', (e) => {
  document.getElementById('highFreqVal').textContent = `${e.target.value}%`;
});

document.getElementById('btnRumblePulse').addEventListener('click', () => {
  const strong = parseInt(document.getElementById('sliderLowFreq').value, 10) / 100;
  const weak = parseInt(document.getElementById('sliderHighFreq').value, 10) / 100;
  triggerRumble(500, weak, strong);
});

document.getElementById('btnRumbleRamp').addEventListener('click', async () => {
  for (let i = 1; i <= 5; i++) {
    await triggerRumble(150, i * 0.2, i * 0.2);
    await new Promise(r => setTimeout(r, 180));
  }
});

document.getElementById('btnRumbleStop').addEventListener('click', () => {
  triggerRumble(10, 0, 0);
});

document.getElementById('btnIdentify').addEventListener('click', () => {
  triggerRumble(200, 1.0, 1.0);
});

// Start loop on page load
initRemapTable();
requestAnimationFrame(updateGamepadState);
