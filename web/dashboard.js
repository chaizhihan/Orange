/**
 * ALIN Stream Dashboard - JavaScript
 * 
 * 功能:
 * - 模拟实时事件流
 * - 更新统计数据
 * - 热切换控制
 */

// State
const state = {
    totalEvents: 0,
    eventRate: 0,
    counts: {
        ERROR: 0,
        WARN: 0,
        INFO: 0,
        DEBUG: 0
    },
    filterLevel: 'WARN',
    threshold: 10,
    events: [],
    startTime: Date.now()
};

// Sample messages for simulation
const MESSAGES = {
    ERROR: [
        'Connection refused to database',
        'Out of memory exception',
        'Timeout waiting for response',
        'Critical assertion failed',
        'Unhandled exception in worker'
    ],
    WARN: [
        'Rate limit approaching threshold',
        'Memory usage high (85%)',
        'Connection pool nearly exhausted',
        'Slow query detected (>2s)',
        'Retry attempt 3 of 5'
    ],
    INFO: [
        'Request processed successfully',
        'User authentication completed',
        'Cache hit ratio: 92%',
        'Background job finished',
        'Health check passed'
    ],
    DEBUG: [
        'Parsing request payload',
        'Executing database query',
        'Loading configuration',
        'Initializing connection',
        'Validating input parameters'
    ]
};

// DOM Elements
const elements = {
    totalEvents: document.getElementById('total-events'),
    eventRate: document.getElementById('event-rate'),
    errorCount: document.getElementById('error-count'),
    warnCount: document.getElementById('warn-count'),
    eventsList: document.getElementById('events-list'),
    thresholdSlider: document.getElementById('threshold-slider'),
    thresholdValue: document.getElementById('threshold-value'),
    levelButtons: document.querySelectorAll('.button-group .btn[data-level]'),
    barError: document.getElementById('bar-error'),
    barWarn: document.getElementById('bar-warn'),
    barInfo: document.getElementById('bar-info'),
    barDebug: document.getElementById('bar-debug'),
    valError: document.getElementById('val-error'),
    valWarn: document.getElementById('val-warn'),
    valInfo: document.getElementById('val-info'),
    valDebug: document.getElementById('val-debug')
};

// Utility functions
function formatTime(date) {
    return date.toLocaleTimeString('zh-CN', { hour12: false });
}

function randomChoice(arr) {
    return arr[Math.floor(Math.random() * arr.length)];
}

// Generate a random event
function generateEvent(level = null) {
    if (!level) {
        const weights = { ERROR: 5, WARN: 15, INFO: 50, DEBUG: 30 };
        const rand = Math.random() * 100;
        let cumulative = 0;
        for (const [lvl, weight] of Object.entries(weights)) {
            cumulative += weight;
            if (rand < cumulative) {
                level = lvl;
                break;
            }
        }
    }

    return {
        level,
        message: randomChoice(MESSAGES[level]),
        timestamp: new Date()
    };
}

// Add event to the stream
function addEvent(event) {
    // Update counts
    state.totalEvents++;
    state.counts[event.level]++;

    // Add to events list (keep last 50)
    state.events.unshift(event);
    if (state.events.length > 50) {
        state.events.pop();
    }

    // Update UI
    updateStats();
    updateEventsList();
    updateChart();
}

// Update statistics display
function updateStats() {
    const elapsed = (Date.now() - state.startTime) / 1000;
    state.eventRate = elapsed > 0 ? (state.totalEvents / elapsed).toFixed(1) : 0;

    elements.totalEvents.textContent = state.totalEvents.toLocaleString();
    elements.eventRate.textContent = state.eventRate;
    elements.errorCount.textContent = state.counts.ERROR;
    elements.warnCount.textContent = state.counts.WARN;
}

// Update events list
function updateEventsList() {
    const levelPriority = { DEBUG: 0, INFO: 1, WARN: 2, ERROR: 3, FATAL: 4 };
    const minPriority = levelPriority[state.filterLevel] || 0;

    const filteredEvents = state.events.filter(e =>
        levelPriority[e.level] >= minPriority
    ).slice(0, 10);

    elements.eventsList.innerHTML = filteredEvents.map(event => `
        <div class="event-item">
            <span class="event-time">${formatTime(event.timestamp)}</span>
            <span class="event-level level-${event.level.toLowerCase()}">${event.level}</span>
            <span class="event-msg">${event.message}</span>
        </div>
    `).join('') || '<div class="event-item"><span class="event-msg">No matching events...</span></div>';
}

// Update bar chart
function updateChart() {
    const max = Math.max(1, ...Object.values(state.counts));

    const update = (bar, val, count) => {
        const percent = (count / max) * 100;
        bar.style.width = `${percent}%`;
        val.textContent = count;
    };

    update(elements.barError, elements.valError, state.counts.ERROR);
    update(elements.barWarn, elements.valWarn, state.counts.WARN);
    update(elements.barInfo, elements.valInfo, state.counts.INFO);
    update(elements.barDebug, elements.valDebug, state.counts.DEBUG);
}

// Initialize Inode displays (simulated)
function updateTopology() {
    const randomInode = () => Math.floor(Math.random() * 9000000 + 1000000);
    document.getElementById('inode-parser').textContent = randomInode();
    document.getElementById('inode-filter').textContent = randomInode();
    document.getElementById('inode-agg').textContent = randomInode();
    document.getElementById('inode-alert').textContent = randomInode();
}

// Event handlers
function setupEventHandlers() {
    // Level filter buttons
    elements.levelButtons.forEach(btn => {
        btn.addEventListener('click', () => {
            elements.levelButtons.forEach(b => b.classList.remove('active'));
            btn.classList.add('active');
            state.filterLevel = btn.dataset.level;
            updateEventsList();
        });
    });

    // Threshold slider
    elements.thresholdSlider.addEventListener('input', (e) => {
        state.threshold = parseInt(e.target.value);
        elements.thresholdValue.textContent = state.threshold;
    });

    // Refresh button
    document.getElementById('btn-refresh').addEventListener('click', () => {
        updateTopology();
    });

    // Reset button
    document.getElementById('btn-reset').addEventListener('click', () => {
        state.totalEvents = 0;
        state.counts = { ERROR: 0, WARN: 0, INFO: 0, DEBUG: 0 };
        state.events = [];
        state.startTime = Date.now();
        updateStats();
        updateEventsList();
        updateChart();
    });

    // Generate event buttons
    document.getElementById('btn-gen-info').addEventListener('click', () => {
        addEvent(generateEvent('INFO'));
    });
    document.getElementById('btn-gen-warn').addEventListener('click', () => {
        addEvent(generateEvent('WARN'));
    });
    document.getElementById('btn-gen-error').addEventListener('click', () => {
        addEvent(generateEvent('ERROR'));
    });
}

// Auto-generate events
function startAutoGeneration() {
    setInterval(() => {
        // Random chance to generate event
        if (Math.random() < 0.3) {
            addEvent(generateEvent());
        }
    }, 500);
}

// Initialize
function init() {
    updateTopology();
    setupEventHandlers();
    startAutoGeneration();

    // Initial events
    for (let i = 0; i < 5; i++) {
        addEvent(generateEvent());
    }
}

// Start when DOM is ready
document.addEventListener('DOMContentLoaded', init);
