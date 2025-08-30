#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H
#include <Arduino.h>
const char dashboard_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Configuration Panel</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: linear-gradient(135deg, #1e293b 0%, #0f172a 100%);
      color: #e2e8f0;
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      padding: 20px;
    }

    .container {
      width: 100%;
      max-width: 1200px;
      background: rgba(30, 41, 59, 0.4);
      backdrop-filter: blur(20px);
      border: 1px solid rgba(71, 85, 105, 0.3);
      border-radius: 20px;
      padding: 32px;
      box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.6);
    }

    .header {
      text-align: center;
      margin-bottom: 40px;
    }

    .header h1 {
      font-size: 2.5rem;
      font-weight: 700;
      background: linear-gradient(135deg, #22d3ee, #06b6d4);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      background-clip: text;
      margin-bottom: 8px;
    }

    .header p {
      color: #94a3b8;
      font-size: 1rem;
    }

    /* Tabs navigation */
    .tabs {
      display: flex;
      justify-content: center;
      margin-bottom: 32px;
      background: rgba(51, 65, 85, 0.3);
      border-radius: 12px;
      padding: 6px;
      gap: 4px;
    }

    .tab-btn {
      background: transparent;
      border: none;
      padding: 16px 32px;
      cursor: pointer;
      border-radius: 8px;
      color: #94a3b8;
      font-weight: 600;
      font-size: 0.95rem;
      transition: all 0.3s ease;
      position: relative;
    }

    .tab-btn:hover {
      color: #e2e8f0;
      background: rgba(51, 65, 85, 0.5);
    }

    .tab-btn.active {
      background: linear-gradient(135deg, #22d3ee, #06b6d4);
      color: #0f172a;
      font-weight: 700;
    }

    /* Tab content */
    .tab-content {
      display: none;
      animation: fadeIn 0.4s ease;
    }

    .tab-content.active {
      display: block;
    }

    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(20px); }
      to { opacity: 1; transform: translateY(0); }
    }

    .content-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
      gap: 24px;
    }

    .card {
      background: rgba(51, 65, 85, 0.3);
      border: 1px solid rgba(71, 85, 105, 0.4);
      border-radius: 16px;
      padding: 24px;
      backdrop-filter: blur(10px);
    }

    .card h3 {
      font-size: 1.25rem;
      font-weight: 700;
      color: #f1f5f9;
      margin-bottom: 20px;
      display: flex;
      align-items: center;
      gap: 12px;
    }

    .status-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 12px 0;
      border-bottom: 1px solid rgba(71, 85, 105, 0.3);
      margin-bottom: 16px;
    }

    .status-row:last-child {
      border-bottom: none;
      margin-bottom: 0;
    }

    .status-label {
      color: #94a3b8;
      font-weight: 500;
    }

    .status-value {
      color: #e2e8f0;
      font-weight: 600;
    }

    .status-connected {
      color: #22d3ee;
    }

    .status-disconnected {
      color: #ef4444;
    }

    .status-warning {
      color: #f59e0b;
    }

    /* Form elements */
    .form-group {
      margin-bottom: 20px;
    }

    .form-group label {
      display: block;
      margin-bottom: 8px;
      font-weight: 600;
      color: #cbd5e1;
      font-size: 0.9rem;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }

    .input-row {
      display: grid;
      grid-template-columns: 1fr auto;
      gap: 12px;
      align-items: end;
    }

    input, select {
      width: 100%;
      padding: 14px 16px;
      border: 1px solid rgba(71, 85, 105, 0.4);
      border-radius: 8px;
      background: rgba(15, 23, 42, 0.6);
      color: #e2e8f0;
      font-size: 0.95rem;
      transition: all 0.3s ease;
    }

    input:focus, select:focus {
      outline: none;
      border-color: #22d3ee;
      box-shadow: 0 0 0 3px rgba(34, 211, 238, 0.1);
    }

    input:hover, select:hover {
      border-color: rgba(71, 85, 105, 0.6);
    }

    /* Custom dropdown styling */
    .network-dropdown {
      position: relative;
    }

    .network-dropdown select {
      appearance: none;
      background-image: url("data:image/svg+xml,%3csvg xmlns='http://www.w3.org/2000/svg' fill='none' viewBox='0 0 20 20'%3e%3cpath stroke='%2394a3b8' stroke-linecap='round' stroke-linejoin='round' stroke-width='1.5' d='M6 8l4 4 4-4'/%3e%3c/svg%3e");
      background-position: right 12px center;
      background-repeat: no-repeat;
      background-size: 16px;
      padding-right: 40px;
      cursor: pointer;
    }

    .network-dropdown select:disabled {
      opacity: 0.6;
      cursor: not-allowed;
    }

    .signal-indicator {
      position: absolute;
      right: 40px;
      top: 50%;
      transform: translateY(-50%);
      font-size: 0.75rem;
      padding: 2px 6px;
      border-radius: 4px;
      font-weight: 600;
      pointer-events: none;
    }

    .signal-strong {
      background: rgba(34, 211, 238, 0.2);
      color: #22d3ee;
    }

    .signal-medium {
      background: rgba(245, 158, 11, 0.2);
      color: #f59e0b;
    }

    .signal-weak {
      background: rgba(239, 68, 68, 0.2);
      color: #ef4444;
    }

    .password-input {
      position: relative;
    }

    .password-toggle {
      position: absolute;
      right: 12px;
      top: 50%;
      transform: translateY(-50%);
      background: transparent;
      border: none;
      color: #94a3b8;
      cursor: pointer;
      padding: 8px;
      border-radius: 4px;
      transition: all 0.2s ease;
    }

    .password-toggle:hover {
      color: #22d3ee;
      background: rgba(51, 65, 85, 0.5);
    }

    /* Buttons */
    .button-group {
      display: flex;
      gap: 12px;
      margin-top: 24px;
      flex-wrap: wrap;
    }

    .button-row {
      display: flex;
      gap: 12px;
      width: 100%;
    }

    button {
      padding: 12px 24px;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      font-weight: 600;
      font-size: 0.95rem;
      transition: all 0.3s ease;
      position: relative;
      overflow: hidden;
      min-height: 44px;
    }

    button:hover {
      transform: translateY(-1px);
    }

    button:active {
      transform: translateY(0);
    }

    button:disabled {
      opacity: 0.6;
      cursor: not-allowed;
      transform: none;
    }

    .btn-primary {
      background: linear-gradient(135deg, #22d3ee, #06b6d4);
      color: #0f172a;
      font-weight: 700;
    }

    .btn-primary:hover:not(:disabled) {
      box-shadow: 0 8px 25px rgba(34, 211, 238, 0.3);
    }

    .btn-secondary {
      background: rgba(51, 65, 85, 0.6);
      color: #e2e8f0;
      border: 1px solid rgba(71, 85, 105, 0.4);
    }

    .btn-secondary:hover:not(:disabled) {
      background: rgba(51, 65, 85, 0.8);
      border-color: rgba(71, 85, 105, 0.6);
    }

    .btn-danger {
      background: linear-gradient(135deg, #ef4444, #dc2626);
      color: #ffffff;
    }

    .btn-danger:hover:not(:disabled) {
      box-shadow: 0 8px 25px rgba(239, 68, 68, 0.3);
    }

    .btn-scan {
      background: linear-gradient(135deg, #10b981, #059669);
      color: #ffffff;
    }

    .btn-scan:hover:not(:disabled) {
      box-shadow: 0 8px 25px rgba(16, 185, 129, 0.3);
    }

    .btn-full {
      flex: 1;
    }

    /* Loading state */
    .loading {
      pointer-events: none;
      opacity: 0.7;
    }

    .loading::after {
      content: '';
      position: absolute;
      inset: 0;
      background: linear-gradient(45deg, transparent, rgba(34, 211, 238, 0.2), transparent);
      animation: shimmer 1.5s infinite;
    }

    @keyframes shimmer {
      0% { transform: translateX(-100%); }
      100% { transform: translateX(100%); }
    }

    /* Responsive design */
    @media (max-width: 768px) {
      .container {
        padding: 20px;
      }
      
      .content-grid {
        grid-template-columns: 1fr;
      }
      
      .tabs {
        flex-direction: column;
      }
      
      .button-row {
        flex-direction: column;
      }
      
      .input-row {
        grid-template-columns: 1fr;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>Configuration Panel</h1>
      <p>Advanced IoT Monitoring & Configuration System</p>
    </div>

    <!-- Tabs -->
    <div class="tabs">
      <button class="tab-btn active" onclick="showTab('wifi')">WiFi Configuration</button>
      <button class="tab-btn" onclick="showTab('gsm')">GSM Settings</button>
      <button class="tab-btn" onclick="showTab('user')">User Management</button>
    </div>

    <!-- WiFi Tab -->
    <div id="wifi" class="tab-content active">
      <div class="content-grid">
        <div class="card">
          <h3>Network Status</h3>
          <div class="status-row">
            <span class="status-label">Access Point IP</span>
            <span class="status-value" id="apAddress">192.168.4.1</span>
          </div>
          <div class="status-row">
            <span class="status-label">Access Point SSID</span>
            <span class="status-value" id="apSSID">ESP32-AccessPoint</span>
          </div>
          <div class="status-row">
            <span class="status-label">WiFi Station IP</span>
            <span class="status-value" id="staAddress">Not connected</span>
          </div>
          <div class="status-row">
            <span class="status-label">Connected Network</span>
            <span class="status-value" id="connectedNetwork">None</span>
          </div>
          <div class="status-row">
            <span class="status-label">Internet Connection</span>
            <span class="status-value" id="internetStatus">Not connected</span>
          </div>
        </div>

        <div class="card">
          <h3>WiFi Configuration</h3>
          
          <div class="input-row">
            <button class="btn-scan btn-full" onclick="scanWifi()" id="scanButton">Scan Networks</button>
          </div>

          <div class="form-group">
            <label for="networkSelect">Available Networks</label>
            <div class="network-dropdown">
              <select id="networkSelect" onchange="selectNetwork()" disabled>
                <option value="">Select a network...</option>
              </select>
              <span class="signal-indicator" id="signalIndicator"></span>
            </div>
          </div>

          <div class="form-group">
            <label for="wifiPass">Network Password</label>
            <div class="password-input">
              <input type="password" id="wifiPass" placeholder="Enter WiFi password" disabled>
              <button type="button" class="password-toggle" onclick="togglePassword()">👁</button>
            </div>
          </div>

          <div class="button-group">
            <div class="button-row">
              <button class="btn-primary btn-full" onclick="connectNetwork()" disabled id="connectBtn">Connect Network</button>
              <button class="btn-danger btn-full" onclick="disconnectNetwork()" id="disconnectBtn">Disconnect</button>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- GSM Tab -->
    <div id="gsm" class="tab-content">
      <div class="content-grid">
        <div class="card">
          <h3>GSM Status</h3>
          <div class="status-row">
            <span class="status-label">Connection Status</span>
            <span class="status-value status-connected">Active</span>
          </div>
          <div class="status-row">
            <span class="status-label">Signal Quality</span>
            <span class="status-value" id="signalQuality">23/31 (Good)</span>
          </div>
          <div class="status-row">
            <span class="status-label">Network Operator</span>
            <span class="status-value">Carrier Network</span>
          </div>
        </div>

        <div class="card">
          <h3>GSM Configuration</h3>

          <div class="form-group">
            <label for="carrierName">Carrier Name</label>
            <input type="text" id="carrierName" placeholder="Enter carrier name">
          </div>

          <div class="form-group">
            <label for="apnSettings">APN Settings</label>
            <input type="text" id="apnSettings" placeholder="Access Point Name">
          </div>

          <div class="form-group">
            <label for="signalDisplay">Signal Quality (CSQ)</label>
            <input type="text" id="signalDisplay" placeholder="Signal strength indicator" readonly>
          </div>

          <div class="button-group">
            <div class="button-row">
              <button class="btn-primary btn-full">Save Configuration</button>
              <button class="btn-secondary btn-full">Refresh Status</button>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- User Tab -->
    <div id="user" class="tab-content">
      <div class="content-grid">
        <div class="card">
          <h3>User Profile</h3>

          <div class="form-group">
            <label for="userName">Full Name</label>
            <input type="text" id="userName" placeholder="Enter your full name">
          </div>

          <div class="form-group">
            <label for="userEmail">Email Address</label>
            <input type="email" id="userEmail" placeholder="your.email@example.com">
          </div>

          <div class="form-group">
            <label for="userContact">Contact Number</label>
            <input type="tel" id="userContact" placeholder="+94 XX XXXXXXX">
          </div>

          <div class="button-group">
            <div class="button-row">
              <button class="btn-primary btn-full">Save Profile</button>
              <button class="btn-danger btn-full">Clear Form</button>
            </div>
          </div>
        </div>

        <div class="card">
          <h3>System Information</h3>
          <div class="status-row">
            <span class="status-label">Device Model</span>
            <span class="status-value">ESP32 DevKit</span>
          </div>
          <div class="status-row">
            <span class="status-label">Firmware Version</span>
            <span class="status-value">v2.1.0</span>
          </div>
          <div class="status-row">
            <span class="status-label">Last Updated</span>
            <span class="status-value">2025-08-30</span>
          </div>
        </div>
      </div>
    </div>
  </div>

  <script>
  let availableNetworks = [];
  let selectedNetworkData = null;

  // ---------- Utilities ----------
  async function apiGet(url) {
    const r = await fetch(url);
    if (!r.ok) throw new Error(await r.text());
    return r.json();
  }
  async function apiPost(url, bodyObj) {
    const r = await fetch(url, {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify(bodyObj || {})
    });
    if (!r.ok) throw new Error(await r.text());
    try { return await r.json(); } catch { return {}; }
  }
  function strengthFromRSSI(rssi) {
    if (rssi >= -60) return 'strong';
    if (rssi >= -75) return 'medium';
    return 'weak';
  }

  // ---------- Tabs ----------
  function showTab(tabId) {
    document.querySelectorAll(".tab-content").forEach(tab => tab.classList.remove("active"));
    document.querySelectorAll(".tab-btn").forEach(btn => btn.classList.remove("active"));
    document.getElementById(tabId).classList.add("active");
    event.target.classList.add("active");
  }

  // ---------- Password toggle ----------
  function togglePassword() {
    const passField = document.getElementById("wifiPass");
    const toggleBtn = document.querySelector(".password-toggle");
    if (passField.type === "password") { passField.type = "text"; toggleBtn.textContent = "👁"; }
    else { passField.type = "password"; toggleBtn.textContent = "👁"; }
  }

  // ---------- Status ----------
  async function refreshStatus() {
    try {
      const st = await apiGet('/api/status');
      console.log('Status response:', st);
      
      // Update Access Point info
      document.getElementById('apAddress').textContent = st.ap?.ip || '—';
      document.getElementById('apSSID').textContent = st.ap?.ssid || '—';

      // Update Station info
      const staConnected = st.sta?.connected || false;
      const staIP = st.sta?.ip || '0.0.0.0';
      const staSSID = st.sta?.ssid || '';
      
      // Station IP
      const staAddressEl = document.getElementById('staAddress');
      if (staConnected && staIP !== '0.0.0.0') {
        staAddressEl.textContent = staIP;
        staAddressEl.className = 'status-value status-connected';
      } else {
        staAddressEl.textContent = 'Not connected';
        staAddressEl.className = 'status-value status-disconnected';
      }
      
      // Connected Network
      const connectedNetworkEl = document.getElementById('connectedNetwork');
      if (staConnected && staSSID) {
        connectedNetworkEl.textContent = staSSID;
        connectedNetworkEl.className = 'status-value status-connected';
      } else {
        connectedNetworkEl.textContent = 'None';
        connectedNetworkEl.className = 'status-value status-disconnected';
      }
      
      // Internet status (same as station status for now)
      const internetStatusEl = document.getElementById('internetStatus');
      if (staConnected) {
        internetStatusEl.textContent = 'Connected';
        internetStatusEl.className = 'status-value status-connected';
      } else {
        internetStatusEl.textContent = 'Not connected';
        internetStatusEl.className = 'status-value status-disconnected';
      }
      
    } catch (e) {
      console.warn('Status error', e);
      // Set error state
      document.getElementById('apAddress').textContent = 'Error';
      document.getElementById('apSSID').textContent = 'Error';
      document.getElementById('staAddress').textContent = 'Error';
      document.getElementById('connectedNetwork').textContent = 'Error';
      document.getElementById('internetStatus').textContent = 'Error';
    }
  }

  // ---------- Scan WiFi ----------
  async function scanWifi() {
    const button = document.getElementById('scanButton');
    const networkSelect = document.getElementById('networkSelect');
    const signalIndicator = document.getElementById('signalIndicator');
    const wifiPass = document.getElementById('wifiPass');
    const connectBtn = document.getElementById('connectBtn');

    button.classList.add('loading'); 
    button.textContent = 'Scanning...'; 
    button.disabled = true;
    networkSelect.disabled = true; 
    wifiPass.disabled = true; 
    connectBtn.disabled = true;
    wifiPass.value = ''; 
    signalIndicator.style.display = 'none';
    networkSelect.innerHTML = '<option value="">Scanning for networks...</option>';

    try {
      const scan = await apiGet('/api/wifi/scan');
      console.log('Scan results:', scan);
      
      // De-duplicate by SSID, keep best RSSI
      const best = {};
      scan.forEach(n => {
        if (!n.ssid) return;
        if (!best[n.ssid] || (n.rssi > best[n.ssid].rssi)) best[n.ssid] = n;
      });
      availableNetworks = Object.values(best).sort((a,b)=>b.rssi - a.rssi);

      networkSelect.innerHTML = '<option value="">Select a network...</option>';
      availableNetworks.forEach(n => {
        const opt = document.createElement('option');
        const sec = (n.security || (n.auth===0?'Open':'Secure'));
        opt.value = n.ssid;
        opt.textContent = `${n.ssid} (${sec})`;
        networkSelect.appendChild(opt);
      });
      
      console.log('Found networks:', availableNetworks.length);
    } catch (e) {
      alert('Scan failed: ' + e.message);
      networkSelect.innerHTML = '<option value="">Scan failed</option>';
      console.error('Scan error:', e);
    } finally {
      networkSelect.disabled = false;
      button.classList.remove('loading'); 
      button.textContent = 'Scan Networks'; 
      button.disabled = false;
    }
  }

  function selectNetwork() {
    const select = document.getElementById('networkSelect');
    const signalIndicator = document.getElementById('signalIndicator');
    const wifiPass = document.getElementById('wifiPass');
    const connectBtn = document.getElementById('connectBtn');

    const ssid = select.value;
    if (ssid) {
      selectedNetworkData = availableNetworks.find(n => n.ssid === ssid) || {ssid, rssi:-80, security:'Secure'};
      const strength = selectedNetworkData.strength || strengthFromRSSI(selectedNetworkData.rssi || -80);
      signalIndicator.textContent = strength[0].toUpperCase() + strength.slice(1);
      signalIndicator.className = `signal-indicator signal-${strength}`;
      signalIndicator.style.display = 'block';
      wifiPass.disabled = (selectedNetworkData.security === 'Open');
      connectBtn.disabled = false;
      if (!wifiPass.disabled) wifiPass.focus();
    } else {
      selectedNetworkData = null;
      signalIndicator.style.display = 'none';
      wifiPass.disabled = true; wifiPass.value = '';
      connectBtn.disabled = true;
    }
  }

  async function connectNetwork() {
    const wifiPass = document.getElementById('wifiPass');
    const connectBtn = document.getElementById('connectBtn');
    if (!selectedNetworkData) { alert('Please select a network'); return; }
    if ((selectedNetworkData.security !== 'Open') && !wifiPass.value.trim()) {
      alert('Please enter the network password'); wifiPass.focus(); return;
    }
    
    connectBtn.classList.add('loading'); 
    connectBtn.textContent = 'Connecting...'; 
    connectBtn.disabled = true;
    
    try {
      const resp = await apiPost('/api/wifi/connect', {
        ssid: selectedNetworkData.ssid,
        password: wifiPass.value
      });
      
      console.log('Connect response:', resp);
      
      if (resp.success) {
        alert(`Connected to ${resp.ssid} — IP ${resp.ip}`);
      } else {
        alert('Failed to connect: ' + (resp.error || 'timeout or wrong password'));
      }
    } catch(e) {
      alert('Connect failed: ' + e.message);
      console.error('Connect error:', e);
    } finally {
      connectBtn.classList.remove('loading'); 
      connectBtn.textContent = 'Connect Network'; 
      connectBtn.disabled = false;
      await refreshStatus();
    }
  }

  async function disconnectNetwork() {
    if (!confirm('Disconnect from current WiFi network?')) return;
    
    const disconnectBtn = document.getElementById('disconnectBtn');
    disconnectBtn.classList.add('loading');
    disconnectBtn.textContent = 'Disconnecting...';
    disconnectBtn.disabled = true;
    
    try {
      const resp = await apiPost('/api/wifi/disconnect', {});
      console.log('Disconnect response:', resp);
      alert('Disconnected from WiFi network.');
    } catch (e) {
      alert('Disconnect failed: ' + e.message);
      console.error('Disconnect error:', e);
    } finally {
      disconnectBtn.classList.remove('loading');
      disconnectBtn.textContent = 'Disconnect';
      disconnectBtn.disabled = false;
      await refreshStatus();
      
      // Clear the network selection
      document.getElementById('networkSelect').value = '';
      document.getElementById('wifiPass').value = '';
      document.getElementById('signalIndicator').style.display = 'none';
      document.getElementById('connectBtn').disabled = true;
      selectedNetworkData = null;
    }
  }

  // ---------- GSM save/load ----------
  async function loadGsm() {
    try {
      const d = await apiGet('/api/load/gsm');
      document.getElementById('carrierName').value = d.carrierName || '';
      document.getElementById('apnSettings').value = d.apn || '';
    } catch (e) { console.warn('GSM load failed', e); }
  }
  
  async function saveGsm() {
    try {
      await apiPost('/api/save/gsm', {
        carrierName: document.getElementById('carrierName').value,
        apn: document.getElementById('apnSettings').value,
        apnUser: "",
        apnPass: ""
      });
      alert('GSM settings saved.');
    } catch (e) { alert('Save failed: ' + e.message); }
  }

  // ---------- USER save/load ----------
  async function loadUser() {
    try {
      const d = await apiGet('/api/load/user');
      document.getElementById('userName').value   = d.name  || '';
      document.getElementById('userEmail').value  = d.email || '';
      document.getElementById('userContact').value= d.phone || '';
    } catch (e) { console.warn('User load failed', e); }
  }
  
  async function saveUser() {
    try {
      await apiPost('/api/save/user', {
        name:  document.getElementById('userName').value,
        email: document.getElementById('userEmail').value,
        phone: document.getElementById('userContact').value
      });
      alert('Profile saved.');
    } catch (e) { alert('Save failed: ' + e.message); }
  }

  // ---------- Wire up buttons on load ----------
  document.addEventListener('DOMContentLoaded', function() {
    console.log('Dashboard loaded');
    
    // Auto scan after 1s
    setTimeout(() => {
      const scanButton = document.querySelector('.btn-scan');
      if (scanButton) {
        console.log('Starting auto scan...');
        scanButton.click();
      }
    }, 1000);

    // Wire up functions to global scope
    window.scanWifi = scanWifi;
    window.selectNetwork = selectNetwork;
    window.connectNetwork = connectNetwork;
    window.disconnectNetwork = disconnectNetwork;
    window.togglePassword = togglePassword;

    // GSM buttons
    const gsmCard = document.querySelector('#gsm .card:nth-child(2)');
    if (gsmCard) {
      const [saveBtn, refreshBtn] = gsmCard.querySelectorAll('.button-row button');
      if (saveBtn)  saveBtn.addEventListener('click', saveGsm);
      if (refreshBtn) refreshBtn.addEventListener('click', async () => { await loadGsm(); });
    }

    // USER buttons
    const userCard = document.querySelector('#user .card:nth-child(1)');
    if (userCard) {
      const [saveBtn, clearBtn] = userCard.querySelectorAll('.button-row button');
      if (saveBtn)  saveBtn.addEventListener('click', saveUser);
      if (clearBtn) clearBtn.addEventListener('click', () => {
        document.getElementById('userName').value='';
        document.getElementById('userEmail').value='';
        document.getElementById('userContact').value='';
      });
    }

    // Initial loads
    refreshStatus();
    loadGsm();
    loadUser();

    // Refresh status every 10 seconds
    setInterval(refreshStatus, 10000);

    // Simulated GSM CSQ ticker
    setInterval(() => {
      const signalField = document.getElementById('signalDisplay');
      const signalStatus = document.getElementById('signalQuality');
      if (signalField && signalStatus) {
        const r = Math.floor(Math.random() * 31) + 1;
        const q = r > 20 ? 'Excellent' : r > 15 ? 'Good' : r > 10 ? 'Fair' : 'Poor';
        const val = `${r}/31 (${q})`;
        signalField.value = val;
        signalStatus.textContent = val;
      }
    }, 5000);
  });
</script>

</body>
</html>
)rawliteral";

// Get the length of the HTML content for Content-Length header
const size_t dashboard_html_len = sizeof(dashboard_html) - 1;

#endif // DASHBOARD_HTML_H