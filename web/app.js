document.addEventListener('DOMContentLoaded', () => {
  initTabs();
  initForms();
  loadStations();
  loadRoutes();
});

function initTabs() {
  document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
      document.querySelectorAll('.tab-content').forEach(c => c.classList.add('hidden'));
      btn.classList.add('active');
      document.getElementById('tab-' + btn.dataset.tab).classList.remove('hidden');
    });
  });
}

function initForms() {
  document.getElementById('station-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const body = { id: fd.get('id'), name: fd.get('name') };
    const res = await apiPost('/api/stations', body);
    if (res) { showToast(res.message, 'success'); e.target.reset(); loadStations(); }
  });

  document.getElementById('route-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const body = {
      id: fd.get('id'), fromStationId: fd.get('fromStationId'),
      toStationId: fd.get('toStationId'), distanceKm: parseInt(fd.get('distanceKm')),
      baseFare: parseFloat(fd.get('baseFare')), capacity: parseInt(fd.get('capacity'))
    };
    const res = await apiPost('/api/routes', body);
    if (res) { showToast(res.message, 'success'); e.target.reset(); loadRoutes(); }
  });

  document.getElementById('book-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const body = {
      routeId: fd.get('routeId'), passengerName: fd.get('passengerName'),
      isVip: fd.get('isVip') === 'on', travelDate: fd.get('travelDate') || ''
    };
    const res = await apiPost('/api/bookings', body);
    if (res) { showToast(res.message, 'success'); e.target.reset(); loadBookings(); loadWaitlist(); loadRoutes(); }
  });

  document.getElementById('cancel-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const res = await apiPost('/api/cancellations', { bookingId: fd.get('bookingId') });
    if (res) { showToast(res.message, 'success'); e.target.reset(); loadBookings(); loadWaitlist(); loadRoutes(); }
  });

  document.getElementById('route-search-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const from = fd.get('from'), to = fd.get('to'), mode = fd.get('mode');
    const res = await apiGet(`/api/routes/search?from=${from}&to=${to}&mode=${mode}`);
    if (res) renderPathResult(res.data);
  });

  document.getElementById('mst-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const res = await apiGet('/api/mst?start=' + fd.get('start'));
    if (res) renderMstResult(res.data);
  });

  document.getElementById('fare-trend-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const res = await apiGet(`/api/fare-trend?routeId=${fd.get('routeId')}&days=${fd.get('days')}`);
    if (res) renderFareTrend(res.data);
  });
}

async function apiPost(url, body) {
  try {
    const resp = await fetch(url, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body)
    });
    const j = await resp.json();
    if (!j.success) { showToast(j.message, 'error'); return null; }
    return j;
  } catch (e) {
    showToast('Network error: ' + e.message, 'error');
    return null;
  }
}

async function apiGet(url) {
  try {
    const resp = await fetch(url);
    const j = await resp.json();
    if (!j.success) { showToast(j.message, 'error'); return null; }
    return j;
  } catch (e) {
    showToast('Network error: ' + e.message, 'error');
    return null;
  }
}

function showToast(msg, type) {
  const t = document.getElementById('toast');
  t.textContent = msg;
  t.className = 'toast ' + (type || '');
  t.classList.remove('hidden');
  setTimeout(() => t.classList.add('hidden'), 3000);
}

async function loadStations() {
  const res = await apiGet('/api/stations');
  if (!res) return;
  const el = document.getElementById('stations-table');
  if (!res.data || res.data.length === 0) { el.innerHTML = '<p class="hint">No stations yet.</p>'; return; }
  el.innerHTML = '<table><thead><tr><th>ID</th><th>Name</th></tr></thead><tbody>' +
    res.data.map(s => `<tr><td>${esc(s.id)}</td><td>${esc(s.name)}</td></tr>`).join('') +
    '</tbody></table>';
}

async function loadRoutes() {
  const res = await apiGet('/api/routes');
  if (!res) return;
  const el = document.getElementById('routes-table');
  if (!res.data || res.data.length === 0) { el.innerHTML = '<p class="hint">No routes yet.</p>'; return; }
  el.innerHTML = '<table><thead><tr><th>ID</th><th>From</th><th>To</th><th>Dist</th><th>Fare</th><th>Cap</th><th>Occupied</th><th>Avail</th><th>Status</th></tr></thead><tbody>' +
    res.data.map(r => `<tr>
      <td>${esc(r.id)}</td><td>${esc(r.fromStationId)}</td><td>${esc(r.toStationId)}</td>
      <td>${r.distanceKm}</td><td>₹${r.baseFare}</td><td>${r.capacity}</td>
      <td>${r.occupiedSeats}</td><td>${r.availableSeats}</td>
      <td>${r.surge ? '<span class="badge badge-surge">SURGE</span>' : '-'}</td>
    </tr>`).join('') + '</tbody></table>';
}

async function loadBookings() {
  const res = await apiGet('/api/bookings');
  if (!res) return;
  const el = document.getElementById('bookings-table');
  if (!res.data || res.data.length === 0) { el.innerHTML = '<p class="hint">No bookings yet.</p>'; return; }
  el.innerHTML = '<table><thead><tr><th>ID</th><th>Route</th><th>Passenger</th><th>VIP</th><th>Status</th><th>Fare</th><th>Date</th></tr></thead><tbody>' +
    res.data.map(b => `<tr>
      <td>${esc(b.id)}</td><td>${esc(b.routeId)}</td><td>${esc(b.passengerName)}</td>
      <td>${b.isVip ? '✓' : ''}</td>
      <td><span class="badge badge-${b.status.toLowerCase()}">${esc(b.status)}</span></td>
      <td>₹${b.chargedFare.toFixed(2)}</td>
      <td>${esc(b.travelDate || '-')}</td>
    </tr>`).join('') + '</tbody></table>';
}

async function loadWaitlist() {
  const res = await apiGet('/api/waitlist');
  if (!res) return;
  const el = document.getElementById('waitlist-table');
  if (!res.data || res.data.length === 0) { el.innerHTML = '<p class="hint">Waitlist is empty.</p>'; return; }
  el.innerHTML = '<table><thead><tr><th>ID</th><th>Route</th><th>Passenger</th><th>VIP</th><th>Fare</th></tr></thead><tbody>' +
    res.data.map(b => `<tr>
      <td>${esc(b.id)}</td><td>${esc(b.routeId)}</td><td>${esc(b.passengerName)}</td>
      <td>${b.isVip ? '✓' : ''}</td>
      <td>₹${b.chargedFare.toFixed(2)}</td>
    </tr>`).join('') + '</tbody></table>';
}

function renderPathResult(data) {
  const el = document.getElementById('path-result');
  if (!data.routeIds || data.routeIds.length === 0) {
    el.innerHTML = '<p class="hint">No path found.</p>';
    return;
  }
  el.innerHTML = `<p><strong>From:</strong> ${esc(data.from)} &nbsp; <strong>To:</strong> ${esc(data.to)} &nbsp; <strong>Mode:</strong> ${esc(data.mode)}</p>
    <p><strong>Total Distance:</strong> ${data.totalDistance} km</p>
    <p><strong>Route(s):</strong> ${data.routeIds.join(' → ')}</p>`;
}

function renderMstResult(data) {
  const el = document.getElementById('mst-result');
  el.innerHTML = `<p><strong>Start Station:</strong> ${esc(data.startStation)}</p>
    <p><strong>Total Distance:</strong> ${data.totalDistance} km</p>
    <p><strong>Routes in MST:</strong> ${data.routeIds.join(', ')}</p>`;
}

async function loadProfitability() {
  const res = await apiGet('/api/reports/profitability');
  if (!res) return;
  const el = document.getElementById('profitability-result');
  const data = res.data;
  const entries = Object.entries(data);
  if (entries.length === 0) { el.innerHTML = '<p class="hint">No data.</p>'; return; }
  el.innerHTML = '<table><thead><tr><th>Route</th><th>Revenue (₹)</th></tr></thead><tbody>' +
    entries.map(([rid, val]) => `<tr><td>${esc(rid)}</td><td>₹${val.toFixed(2)}</td></tr>`).join('') +
    '</tbody></table>';
}

async function loadOccupancy() {
  const res = await apiGet('/api/occupancy-map');
  if (!res) return;
  const el = document.getElementById('occupancy-map');
  if (!res.data || res.data.length === 0) { el.innerHTML = '<p class="hint">No routes to display.</p>'; return; }
  el.innerHTML = '<div class="occupancy-bar-container">' +
    res.data.map(r => {
      const pct = r.occupancyPercent;
      const cls = pct < 50 ? 'low' : pct < 80 ? 'medium' : 'high';
      return `<div class="occupancy-item">
        <span class="occupancy-label">${esc(r.routeId)} (${esc(r.fromStationId)}→${esc(r.toStationId)})</span>
        <div class="occupancy-track"><div class="occupancy-fill ${cls}" style="width:${pct}%"></div></div>
        <span class="occupancy-pct">${pct.toFixed(0)}%</span>
      </div>`;
    }).join('') + '</div>';
}

function renderFareTrend(data) {
  const el = document.getElementById('fare-trend-result');
  if (!data.trend || data.trend.length === 0) { el.innerHTML = '<p class="hint">No data.</p>'; return; }
  el.innerHTML = `<p><strong>Route:</strong> ${esc(data.routeId)} &nbsp; <strong>Projection Days:</strong> ${data.days}</p>
    <table><thead><tr><th>Day</th><th>Projected Fare (₹)</th><th>Occupancy</th><th>Occupancy %</th></tr></thead><tbody>` +
    data.trend.map(t => `<tr>
      <td>${t.day}</td><td>₹${t.projectedFare.toFixed(2)}</td>
      <td>${t.projectedOccupancy}</td><td>${t.occupancyPercent.toFixed(1)}%</td>
    </tr>`).join('') + '</tbody></table>';
}

function esc(s) {
  const d = document.createElement('div');
  d.textContent = s;
  return d.innerHTML;
}
