/**
 * DBS - Dashboard Subsystem
 * Web application for monitoring and controlling the Tank Monitoring System
 */

// Configuration
const API_BASE_URL = 'http://localhost:8080/api';
const UPDATE_INTERVAL = 200; // 0.2 second for real-time feel

// State
let currentMode = 'UNCONNECTED';
let chart = null;
let updateTimer = null;
let userInteracting = false; // Track if user is using the slider
let lastCommandTime = 0; // Track when the last command was sent
let sliderDirty = false; // Track if the slider value has been changed by user but not sent

// Chart.js configuration
const chartConfig = {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Water Level (cm)',
            data: [],
            borderColor: '#00d9ff',
            backgroundColor: 'rgba(0, 217, 255, 0.1)',
            borderWidth: 2,
            fill: true,
            tension: 0.4,
            pointRadius: 3,
            pointHoverRadius: 5
        }]
    },
    options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
            legend: {
                display: true,
                labels: {
                    color: '#a0a0b8',
                    font: { family: 'Inter', size: 12 }
                }
            }
        },
        scales: {
            x: {
                grid: { color: '#2a2a3e' },
                ticks: {
                    color: '#6e6e8f',
                    maxRotation: 45,
                    autoSkip: true,
                    maxTicksLimit: 10
                }
            },
            y: {
                grid: { color: '#2a2a3e' },
                ticks: { color: '#6e6e8f' },
                beginAtZero: true
            }
        }
    }
};

// ==================== INITIALIZATION ====================

document.addEventListener('DOMContentLoaded', () => {
    console.log('Dashboard initializing...');

    // Initialize chart
    const ctx = document.getElementById('waterLevelChart').getContext('2d');
    chart = new Chart(ctx, chartConfig);

    // Set up event listeners
    setupEventListeners();

    // Start auto-update
    startAutoUpdate();

    console.log('Dashboard initialized');
});

// ==================== EVENT LISTENERS ====================

function setupEventListeners() {
    // Mode toggle buttons
    document.getElementById('btnAutomatic').addEventListener('click', () => setMode('AUTOMATIC'));
    document.getElementById('btnManual').addEventListener('click', () => setMode('MANUAL'));

    // Valve slider
    const slider = document.getElementById('valveSlider');
    slider.addEventListener('input', (e) => {
        document.getElementById('sliderValue').textContent = e.target.value + '%';
        sliderDirty = true; // User has modified the value
    });

    // Track user interaction to prevent overwriting while dragging
    slider.addEventListener('mousedown', () => userInteracting = true);
    slider.addEventListener('mouseup', () => userInteracting = false);
    slider.addEventListener('touchstart', () => userInteracting = true);
    slider.addEventListener('touchend', () => userInteracting = false);

    // Apply valve button
    document.getElementById('btnSetValve').addEventListener('click', () => {
        const value = parseInt(document.getElementById('valveSlider').value);
        lastCommandTime = Date.now(); // Record command time
        sliderDirty = false; // Reset dirty flag as we are sending the value
        setValveOpening(value);
    });
}
// ==================== AUTO-UPDATE ====================

function startAutoUpdate() {
    updateStatus();
    updateHistory();

    updateTimer = setInterval(() => {
        updateStatus();
    }, UPDATE_INTERVAL);

    // Update history less frequently
    setInterval(() => {
        updateHistory();
    }, 5000);
}

// ==================== API CALLS ====================

async function updateStatus() {
    try {
        const response = await fetch(`${API_BASE_URL}/status`);

        if (!response.ok) {
            throw new Error('Server not available');
        }

        const data = await response.json();

        // Update UI with status data
        updateConnectionStatus(true);
        updateMode(data.mode);
        updateWaterLevel(data.waterLevel);
        updateValveGauge(data.valveOpening);
        updateTMSStatus(data.tmsConnected);
        updateLastUpdate();

    } catch (error) {
        console.error('Error fetching status:', error);
        updateConnectionStatus(false);
        updateMode('NOT AVAILABLE');
    }
}

async function updateHistory() {
    try {
        const response = await fetch(`${API_BASE_URL}/history`);

        if (!response.ok) {
            throw new Error('Failed to fetch history');
        }

        const data = await response.json();
        updateChart(data.readings);

    } catch (error) {
        console.error('Error fetching history:', error);
    }
}

async function setMode(mode) {
    try {
        const response = await fetch(`${API_BASE_URL}/mode`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ mode: mode })
        });

        if (!response.ok) {
            const error = await response.json();
            alert('Error: ' + error.error);
            return;
        }

        console.log('Mode set to:', mode);
        await updateStatus();

    } catch (error) {
        console.error('Error setting mode:', error);
        alert('Failed to set mode. Check connection.');
    }
}

async function setValveOpening(opening) {
    try {
        const response = await fetch(`${API_BASE_URL}/valve`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ opening: opening })
        });

        if (!response.ok) {
            const error = await response.json();
            alert('Error: ' + error.error);
            return;
        }

        console.log('Valve set to:', opening + '%');
        await updateStatus();

    } catch (error) {
        console.error('Error setting valve:', error);
        alert('Failed to set valve. Check connection.');
    }
}

// ==================== UI UPDATE FUNCTIONS ====================

function updateConnectionStatus(connected) {
    const statusElement = document.getElementById('connectionStatus');
    const indicatorElement = statusElement.querySelector('.status-indicator');
    const textElement = statusElement.querySelector('.status-text');

    if (connected) {
        indicatorElement.style.background = 'var(--accent-success)';
        textElement.textContent = 'Connected';
    } else {
        indicatorElement.style.background = 'var(--accent-danger)';
        textElement.textContent = 'Disconnected';
    }
}

function updateMode(mode) {
    currentMode = mode;

    // Update badge
    const badge = document.getElementById('modeBadge');
    badge.textContent = mode;
    badge.className = 'state-badge ' + mode.toLowerCase().replace(' ', '-');

    // Update buttons state
    const btnAuto = document.getElementById('btnAutomatic');
    const btnMan = document.getElementById('btnManual');

    if (mode === 'AUTOMATIC') {
        btnAuto.classList.add('active');
        btnMan.classList.remove('active');

        document.getElementById('manualControlGroup').style.display = 'none';
        document.getElementById('autoControlMessage').style.display = 'block';
        document.getElementById('autoControlMessage').innerHTML = '<i>Valve controlled by system logic. Switch to Manual to override.</i>';
        sliderDirty = false; // Reset dirty flag when switching modes

    } else if (mode === 'MANUAL') {
        btnAuto.classList.remove('active');
        btnMan.classList.add('active');

        document.getElementById('manualControlGroup').style.display = 'block';
        document.getElementById('autoControlMessage').style.display = 'none';

    } else {
        // UNCONNECTED
        btnAuto.classList.remove('active');
        btnMan.classList.remove('active');
        document.getElementById('manualControlGroup').style.display = 'none';
        document.getElementById('autoControlMessage').style.display = 'none';
    }
}

function updateWaterLevel(level) {
    const element = document.getElementById('currentLevel');
    element.textContent = level.toFixed(2) + ' cm';
}

function updateValveGauge(percentage) {
    // Update gauge fill
    const gaugeFill = document.getElementById('gaugeFill');
    // Arc is a semi-circle with radius 40. Length = pi * 40 = ~125.66
    const circumference = 125.6;
    const offset = circumference - (circumference * percentage / 100);
    gaugeFill.style.strokeDashoffset = offset;
    gaugeFill.style.strokeDasharray = circumference; // Ensure array matches length

    // Update percentage text
    document.getElementById('valvePercentage').textContent = percentage;
    document.getElementById('gaugeText').textContent = percentage + '%';

    // Update slider if we are in Manual mode, NOT interacting, AND haven't sent a command recently
    // This prevents the slider from "jumping back" before the server processes the new value
    const timeSinceLastCommand = Date.now() - lastCommandTime;

    // Only update slider if:
    // 1. We are NOT interacting with it (dragging)
    // 2. The slider is NOT dirty (user hasn't moved it without setting)
    // 3. We haven't just sent a command (wait for round trip)
    if (currentMode === 'MANUAL' && !userInteracting && !sliderDirty && timeSinceLastCommand > 2000) {
        document.getElementById('valveSlider').value = percentage;
        document.getElementById('sliderValue').textContent = percentage + '%';
    }
}

function updateTMSStatus(connected) {
    const element = document.getElementById('tmsStatus');
    element.textContent = connected ? 'Connected' : 'Disconnected';
    element.style.color = connected ? 'var(--accent-success)' : 'var(--accent-danger)';
}

function updateLastUpdate() {
    const element = document.getElementById('lastUpdate');
    const now = new Date();
    element.textContent = now.toLocaleTimeString();
}

function updateChart(readings) {
    if (!readings || readings.length === 0) {
        return;
    }

    // Prepare data for chart
    const labels = readings.map(r => {
        const date = new Date(r.timestamp);
        return date.toLocaleTimeString();
    });

    const data = readings.map(r => r.level);

    // Update chart
    chart.data.labels = labels;
    chart.data.datasets[0].data = data;
    chart.update('none'); // Update without animation for smoother real-time updates
}

// ==================== UTILITY FUNCTIONS ====================

function formatTime(timestamp) {
    const date = new Date(timestamp);
    return date.toLocaleTimeString();
}

// Cleanup on page unload
window.addEventListener('beforeunload', () => {
    if (updateTimer) {
        clearInterval(updateTimer);
    }
});
