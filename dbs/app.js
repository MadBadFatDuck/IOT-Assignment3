/**
 * DBS - Dashboard Subsystem
 * Web application for monitoring and controlling the Tank Monitoring System
 */

// Configuration
const API_BASE_URL = 'http://localhost:8080/api';
const UPDATE_INTERVAL = 1000; // 1 second

// State
let currentMode = 'UNCONNECTED';
let chart = null;
let updateTimer = null;

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
    document.getElementById('btnAutomatic').addEventListener('click', () => {
        setMode('AUTOMATIC');
    });
    
    document.getElementById('btnManual').addEventListener('click', () => {
        setMode('MANUAL');
    });
    
    // Valve slider
    const slider = document.getElementById('valveSlider');
    slider.addEventListener('input', (e) => {
        document.getElementById('sliderValue').textContent = e.target.value + '%';
    });
    
    // Apply valve button
    document.getElementById('btnSetValve').addEventListener('click', () => {
        const value = parseInt(document.getElementById('valveSlider').value);
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
        
        console.log('Mode changed to:', mode);
        await updateStatus();
        
    } catch (error) {
        console.error('Error setting mode:', error);
        alert('Failed to change mode. Check connection to CUS.');
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
        alert('Failed to set valve. Check connection to CUS.');
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
    
    // Update mode buttons
    const btnAutomatic = document.getElementById('btnAutomatic');
    const btnManual = document.getElementById('btnManual');
    
    btnAutomatic.classList.remove('active');
    btnManual.classList.remove('active');
    
    if (mode === 'AUTOMATIC') {
        btnAutomatic.classList.add('active');
        document.getElementById('manualControlGroup').style.display = 'none';
    } else if (mode === 'MANUAL') {
        btnManual.classList.add('active');
        document.getElementById('manualControlGroup').style.display = 'flex';
    } else {
        // UNCONNECTED or NOT AVAILABLE
        document.getElementById('manualControlGroup').style.display = 'none';
    }
}

function updateWaterLevel(level) {
    const element = document.getElementById('currentLevel');
    element.textContent = level.toFixed(2) + ' cm';
}

function updateValveGauge(percentage) {
    // Update gauge fill
    const gaugeFill = document.getElementById('gaugeFill');
    const circumference = 251.2; // Approximate arc length
    const offset = circumference - (circumference * percentage / 100);
    gaugeFill.style.strokeDashoffset = offset;
    
    // Update percentage text
    document.getElementById('valvePercentage').textContent = percentage;
    
    // Update slider if in manual mode
    if (currentMode === 'MANUAL') {
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
