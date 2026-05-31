/* ─── State ─── */
let network, networkFullscreen;
let allStations = [], allRoutes = [];
let routeEdgeMap = {};
let nodesDataSet, edgesDataSet;

const PAGE_META = {
  overview: { title: 'Overview', subtitle: 'Dashboard summary of your railway network', mode: 'Network Overview' },
  stations: { title: 'Stations & Routes', subtitle: 'Manage your railway infrastructure', mode: 'Infrastructure Control' },
  booking: { title: 'Booking', subtitle: 'Book seats, manage waitlist, process cancellations', mode: 'Booking Operations' },
  routing: { title: 'Route Finder', subtitle: 'Find shortest or least-crowded paths using Dijkstra\'s algorithm', mode: 'Path Planning' },
  mst: { title: 'MST Report', subtitle: 'Build a minimum spanning tree using Prim\'s algorithm', mode: 'MST Analysis' },
  analysis: { title: 'Analysis', subtitle: 'Reports, fare search, occupancy, and trend prediction', mode: 'Advanced Analytics' }
};

/* ─── Init ─── */
document.addEventListener('DOMContentLoaded', () => {
  initSidebar();
  initForms();
  initGraphModal();
  updateHeroMetrics('overview');
  setGraphCallout('none', '');
  loadAllData();
});

function initSidebar() {
  document.querySelectorAll('.nav-item').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.nav-item').forEach(b => b.classList.remove('active'));
      document.querySelectorAll('.page').forEach(p => p.classList.remove('active', 'fade-in'));
      btn.classList.add('active');
      const page = document.getElementById('page-' + btn.dataset.page);
      page.classList.add('active', 'fade-in');

      const meta = PAGE_META[btn.dataset.page] || { title: '', subtitle: '' };
      document.getElementById('page-title').textContent = meta.title;
      document.getElementById('page-subtitle').textContent = meta.subtitle;
      updateHeroMetrics(btn.dataset.page);
      if (btn.dataset.page !== 'routing' && btn.dataset.page !== 'mst') {
        resetGraphColors();
        setGraphCallout('none', '');
      }
    });
  });
}

function renderStatePanel(state, message) {
  if (state === 'loading') return '<div class="empty-state"><p>Loading...</p></div>';
  if (state === 'error') return '<div class="empty-state"><p>' + esc(message || 'Unable to load data.') + '</p></div>';
  if (state === 'success' && message) return '<div class="empty-state"><p>' + esc(message) + '</p></div>';
  return '';
}

function setSectionState(sectionKey, state, message) {
  const sectionMap = {
    route: 'path-result',
    mst: 'mst-result',
    profitability: 'profitability-result',
    fareRange: 'fare-range-result',
    occupancy: 'occupancy-map',
    fareTrend: 'fare-trend-result'
  };
  const targetId = sectionMap[sectionKey];
  if (!targetId) return;
  const el = document.getElementById(targetId);
  if (!el) return;
  if (state === 'loading' || state === 'error') {
    el.innerHTML = renderStatePanel(state, message);
  } else if (state === 'success' && message) {
    el.innerHTML = renderStatePanel(state, message);
  }
}

function updateHeroMetrics(pageKey) {
  const heroMode = document.getElementById('hero-mode');
  const heroStations = document.getElementById('hero-stations');
  const heroRoutes = document.getElementById('hero-routes');
  if (heroMode) heroMode.textContent = (PAGE_META[pageKey] || PAGE_META.overview).mode;
  if (heroStations) heroStations.textContent = allStations.length;
  if (heroRoutes) heroRoutes.textContent = allRoutes.length;
}

function setGraphCallout(type, detail) {
  const callout = document.getElementById('graph-callout');
  if (!callout) return;
  if (type === 'dijkstra') {
    callout.textContent = 'Dijkstra: ' + detail;
  } else if (type === 'mst') {
    callout.textContent = 'MST: ' + detail;
  } else {
    callout.textContent = '';
  }
  callout.classList.toggle('active', type && detail ? true : false);
}

function initGraphModal() {
  document.getElementById('expand-graph-btn').addEventListener('click', () => {
    document.getElementById('graph-modal').classList.add('open');
    if (!networkFullscreen) {
      setTimeout(() => {
        const container = document.getElementById('network-graph-fullscreen');
        networkFullscreen = new vis.Network(container, { nodes: nodesDataSet, edges: edgesDataSet }, {
          physics: { enabled: false },
          interaction: { hover: true, tooltipDelay: 200, dragView: true, zoomView: true },
          layout: { improvedLayout: true },
          edges: { smooth: { type: 'curvedCW', roundness: 0.1 } }
        });
      }, 100);
    }
  });
  document.getElementById('graph-modal-close').addEventListener('click', () => {
    document.getElementById('graph-modal').classList.remove('open');
  });
  document.getElementById('graph-modal').addEventListener('click', e => {
    if (e.target === e.currentTarget) document.getElementById('graph-modal').classList.remove('open');
  });
  document.getElementById('reset-graph-btn').addEventListener('click', () => {
    resetGraphColors();
    if (network) network.fit({ animation: true });
    if (networkFullscreen) networkFullscreen.fit({ animation: true });
    setGraphCallout('none', '');
  });
}

function initForms() {
  document.getElementById('station-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const res = await apiPost('/api/stations', { id: fd.get('id'), name: fd.get('name') });
    if (res) { showToast(res.message, 'success'); e.target.reset(); loadAllData(); }
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
    if (res) { showToast(res.message, 'success'); e.target.reset(); loadAllData(); }
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
    if (res) {
      showToast(res.message, 'success');
      e.target.reset();
      document.getElementById('last-refund').textContent = 'Last cancelled: ' + (res.data?.refundId || '—');
      loadBookings(); loadWaitlist(); loadRoutes();
    }
  });

  document.getElementById('route-search-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    const from = fd.get('from'), to = fd.get('to');
    const mode = document.querySelector('#mode-toggle .active')?.dataset.mode || 'shortest';
    showToast('Searching...', '');
    setSectionState('route', 'loading');
    const res = await apiGet(`/api/routes/search?from=${from}&to=${to}&mode=${mode}`);
    if (res) {
      setSectionState('route', 'success');
      renderPathResult(res.data);
    } else {
      resetGraphColors();
      setGraphCallout('none', '');
      setSectionState('route', 'error', 'Route search failed. Try different stations.');
    }
  });

  document.getElementById('mst-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    showToast('Building MST...', '');
    setSectionState('mst', 'loading');
    const res = await apiGet('/api/mst?start=' + fd.get('start'));
    if (res) {
      setSectionState('mst', 'success');
      renderMstResult(res.data);
    } else {
      resetGraphColors();
      setGraphCallout('none', '');
      setSectionState('mst', 'error', 'Failed to build MST from selected station.');
    }
  });

  document.getElementById('fare-trend-form').addEventListener('submit', async e => {
    e.preventDefault();
    const fd = new FormData(e.target);
    setSectionState('fareTrend', 'loading');
    const res = await apiGet(`/api/fare-trend?routeId=${fd.get('routeId')}&days=${fd.get('days')}`);
    if (res) {
      setSectionState('fareTrend', 'success');
      renderFareTrend(res.data);
    } else {
      setSectionState('fareTrend', 'error', 'Fare trend data is unavailable right now.');
    }
  });

  document.querySelectorAll('#mode-toggle .btn').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('#mode-toggle .btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
    });
  });
}

/* ─── Data Loading ─── */
async function loadAllData() {
  const [sRes, rRes] = await Promise.all([apiGet('/api/stations'), apiGet('/api/routes')]);
  allStations = sRes?.data || [];
  allRoutes = rRes?.data || [];

  initGraph(allStations, allRoutes);
  updateStats(allStations, allRoutes);
  updateHeroMetrics(document.querySelector('.nav-item.active')?.dataset.page || 'overview');
  loadStationsTable();
  loadRoutesTable();
}

function updateStats(stations, routes) {
  const totalKm = routes.reduce((s, r) => s + r.distanceKm, 0);
  document.getElementById('stat-stations').textContent = stations.length;
  document.getElementById('stat-stations-sub').textContent = stations.length + ' stations';
  document.getElementById('stat-stations-sub').className = 'stat-sub green';
  document.getElementById('stat-routes').textContent = routes.length;
  document.getElementById('stat-routes-sub').textContent = totalKm + ' total km';
  document.getElementById('stat-routes-sub').className = 'stat-sub muted';
}

function updateBookingStats(bookings) {
  const confirmed = bookings.filter(b => b.status === 'Confirmed').length;
  const waitlisted = bookings.filter(b => b.status === 'Waitlisted').length;
  const totalRevenue = bookings.reduce((s, b) => s + (b.chargedFare || 0), 0);
  document.getElementById('stat-bookings').textContent = bookings.length;
  document.getElementById('stat-bookings-sub').textContent = confirmed + ' confirmed, ' + waitlisted + ' waitlisted';
  document.getElementById('stat-bookings-sub').className = 'stat-sub green';
  document.getElementById('stat-revenue').textContent = '₹' + totalRevenue.toFixed(0);
  document.getElementById('stat-revenue-sub').textContent = 'Across ' + allRoutes.length + ' routes';
  document.getElementById('stat-revenue-sub').className = 'stat-sub muted';
}

/* ─── Graph ─── */
function initGraph(stations, routes) {
  const nodes = stations.map((s) => ({
    id: s.id,
    label: s.id + '\n' + s.name,
    title: s.name + ' (' + s.id + ')',
    color: { background: '#f8fafc', border: '#1f2937', highlight: { background: '#ffffff', border: '#0ea5e9' }, hover: { background: '#ffffff', border: '#0ea5e9' } },
    font: { color: '#1f2937', size: 12, face: 'Manrope, Sora, sans-serif', multi: true },
    borderWidth: 2,
    size: 16,
    shape: 'dot'
  }));

  const edges = [];
  routeEdgeMap = {};
  routes.forEach(r => {
    const eid = 'e-' + r.id;
    routeEdgeMap[r.id] = eid;
    edges.push({
      id: eid,
      from: r.fromStationId, to: r.toStationId,
      label: r.distanceKm + 'km',
      title: r.id + ': ' + r.fromStationId + ' → ' + r.toStationId + ' (' + r.distanceKm + 'km, ₹' + r.baseFare + ')',
      font: { size: 9, color: 'rgba(0,0,0,0)', strokeWidth: 2, strokeColor: '#ffffff' },
      color: { color: '#1e293b', highlight: '#0ea5e9', hover: '#0284c7' },
      width: 3,
      smooth: { type: 'curvedCW', roundness: 0.05 },
      chosen: { edge: values => { values.shadow = true; } }
    });
  });

  nodesDataSet = new vis.DataSet(nodes);
  edgesDataSet = new vis.DataSet(edges);
  const data = { nodes: nodesDataSet, edges: edgesDataSet };
  const options = {
    physics: {
      solver: 'forceAtlas2Based',
      forceAtlas2Based: { gravitationalConstant: -55, springLength: 190, springConstant: 0.004, avoidOverlap: 1 },
      stabilization: { iterations: 140, updateInterval: 10 }
    },
    interaction: { hover: true, tooltipDelay: 250, dragView: true, zoomView: true },
    layout: { improvedLayout: true },
    nodes: { margin: 6 },
    edges: { smooth: { type: 'curvedCW', roundness: 0.05 } }
  };

  const container = document.getElementById('network-graph');
  if (network) network.destroy();
  container.innerHTML = '';
  network = new vis.Network(container, data, options);
  networkFullscreen = null;

  network.on('hoverNode', params => {
    // Label already visible; optional: highlight node border or size if desired
    // For now, do nothing to keep labels always visible
  });

  network.on('blurNode', params => {
    // Label already visible; do nothing
  });

  network.on('selectNode', params => {
    params.nodes.forEach(nodeId => {
      nodesDataSet.update({ id: nodeId, font: { color: '#1f2937', size: 12, face: 'Manrope, Sora, sans-serif', multi: true } });
    });
  });

  network.on('deselectNode', params => {
    params.previousSelection.nodes.forEach(nodeId => {
      nodesDataSet.update({ id: nodeId, font: { color: '#1f2937', size: 12, face: 'Manrope, Sora, sans-serif', multi: true } });
    });
  });

  network.on('hoverEdge', params => {
    edgesDataSet.update({ id: params.edge, font: { size: 9, color: '#64748b', strokeWidth: 2, strokeColor: '#ffffff' } });
  });

  network.on('blurEdge', params => {
    edgesDataSet.update({ id: params.edge, font: { size: 9, color: 'rgba(0,0,0,0)', strokeWidth: 2, strokeColor: '#ffffff' } });
  });

  network.on('click', params => {
    if (params.nodes.length > 0) {
      const stationId = params.nodes[0];
      document.querySelector('.nav-item[data-page="routing"]')?.click();
      const fromInput = document.querySelector('#route-search-form input[name="from"]');
      if (fromInput) fromInput.value = stationId;
    }
  });
}

function resetGraphColors() {
  if (!edgesDataSet) return;
  allRoutes.forEach(r => {
    const eid = routeEdgeMap[r.id];
    if (eid) edgesDataSet.update({ id: eid, color: { color: '#1e293b', highlight: '#0ea5e9', hover: '#0284c7' }, width: 3 });
  });
}

function highlightEdges(routeIds, color, width) {
  if (!edgesDataSet) return;
  resetGraphColors();
  routeIds.forEach(rid => {
    const eid = routeEdgeMap[rid];
    if (eid) edgesDataSet.update({ id: eid, color: { color, highlight: color, hover: color }, width });
  });
}

function syncFullscreenEdges() {
  if (!networkFullscreen || !network) return;
  const data = network.body.data;
  networkFullscreen.setData({ nodes: data.nodes, edges: data.edges });
}

function reconstructPath(from, routeIds) {
  const stops = [{ id: from, name: '' }];
  let current = from;
  routeIds.forEach(rid => {
    const route = allRoutes.find(r => r.id === rid);
    if (route) {
      if (route.fromStationId === current) {
        stops.push({ id: route.toStationId, name: '', edge: rid, dist: route.distanceKm, fare: route.baseFare });
        current = route.toStationId;
      } else if (route.toStationId === current) {
        stops.push({ id: route.fromStationId, name: '', edge: rid, dist: route.distanceKm, fare: route.baseFare });
        current = route.fromStationId;
      }
    }
  });
  stops.forEach(s => {
    const st = allStations.find(st => st.id === s.id);
    if (st) s.name = st.name;
  });
  return stops;
}

/* ─── Stations & Routes Tables ─── */
function loadStationsTable() {
  const el = document.getElementById('stations-table');
  const cnt = document.getElementById('station-count');
  if (!allStations.length) { el.innerHTML = '<div class="empty-state"><p>No stations yet.</p></div>'; cnt.textContent = '0'; return; }
  cnt.textContent = allStations.length;
  el.innerHTML = '<table><thead><tr><th>ID</th><th>Name</th></tr></thead><tbody>' +
    allStations.map(s => `<tr><td style="font-weight:600;color:#2563eb">${esc(s.id)}</td><td>${esc(s.name)}</td></tr>`).join('') +
    '</tbody></table>';
}

function loadRoutesTable() {
  const el = document.getElementById('routes-table');
  const cnt = document.getElementById('route-count');
  if (!allRoutes.length) { el.innerHTML = '<div class="empty-state"><p>No routes yet.</p></div>'; cnt.textContent = '0'; return; }
  cnt.textContent = allRoutes.length;
  el.innerHTML = '<table><thead><tr><th>ID</th><th>From</th><th>To</th><th>Dist</th><th>Fare</th><th>Cap</th><th>Avail</th><th>Status</th></tr></thead><tbody>' +
    allRoutes.map(r => `<tr>
      <td style="font-weight:600">${esc(r.id)}</td>
      <td>${esc(r.fromStationId)}</td>
      <td>${esc(r.toStationId)}</td>
      <td>${r.distanceKm}km</td>
      <td>₹${r.baseFare}</td>
      <td>${r.capacity}</td>
      <td>${r.availableSeats}</td>
      <td>${r.surge ? '<span class="badge badge-surge">SURGE</span>' : '<span class="text-muted">—</span>'}</td>
    </tr>`).join('') + '</tbody></table>';
}

/* ─── Route Finding ─── */
function renderPathResult(data) {
  const el = document.getElementById('path-result');
  if (!data.routeIds || data.routeIds.length === 0) {
    el.innerHTML = '<div class="result-box" style="background:#fef2f2;border-color:#fecaca"><p style="color:#dc2626">No path found between these stations.</p></div>';
    setGraphCallout('none', '');
    return;
  }

  highlightEdges(data.routeIds, '#2563eb', 4);

  const stops = reconstructPath(data.from, data.routeIds);
  const fromName = allStations.find(s => s.id === data.from)?.name || data.from;
  const toName = allStations.find(s => s.id === data.to)?.name || data.to;
  setGraphCallout('dijkstra', data.from + ' -> ' + data.to);

  let pathHtml = '<div class="path-stops">';
  stops.forEach((s, i) => {
    if (i > 0) {
      const prevRoute = allRoutes.find(r => r.id === s.edge);
      pathHtml += `<span class="route-edge">→ <span class="edge-label">${esc(s.edge)} ${prevRoute ? '(' + prevRoute.distanceKm + 'km)' : ''}</span></span>`;
    }
    pathHtml += `<span class="route-stop">${esc(s.id)}${s.name ? ' · ' + esc(s.name) : ''}</span>`;
  });
  pathHtml += '</div>';

  el.innerHTML = `<div class="result-box">
    ${pathHtml}
    <div class="result-meta">
      <span><strong>From:</strong> ${esc(fromName)}</span>
      <span><strong>To:</strong> ${esc(toName)}</span>
      <span><strong>Total:</strong> ${data.totalDistance} km</span>
      <span><strong>Mode:</strong> ${data.mode === 'shortest' ? 'Shortest' : 'Least Crowded'}</span>
    </div>
  </div>`;
}

/* ─── MST ─── */
function renderMstResult(data) {
  const el = document.getElementById('mst-result');
  if (!data.routeIds || data.routeIds.length === 0) {
    el.innerHTML = '<div class="empty-state"><p>No reachable stations from this start point.</p></div>';
    setGraphCallout('none', '');
    return;
  }

  highlightEdges(data.routeIds, '#059669', 4);
  setGraphCallout('mst', data.totalDistance + ' km total');

  el.innerHTML = `<div class="result-box success">
    <div><span style="font-size:0.8rem;color:#475569">Minimum total distance to connect all stations:</span></div>
    <div class="mst-total">${data.totalDistance} km</div>
    <div class="mst-pills">
      ${data.routeIds.map(rid => {
        const route = allRoutes.find(r => r.id === rid);
        return `<span class="mst-pill">${esc(rid)}${route ? ' (' + route.distanceKm + 'km)' : ''}</span>`;
      }).join('')}
    </div>
    <div style="margin-top:10px;font-size:0.75rem;color:#065f46">
      Starting from <strong>${esc(data.startStation)}</strong> · MST edges highlighted in green on the graph
    </div>
  </div>`;
}

/* ─── Bookings ─── */
async function loadStations() {
  const res = await apiGet('/api/stations');
  if (!res) return;
  allStations = res.data || [];
  loadStationsTable();
}

async function loadRoutes() {
  const res = await apiGet('/api/routes');
  if (!res) return;
  allRoutes = res.data || [];
  loadRoutesTable();
  if (network) {
    initGraph(allStations, allRoutes);
    updateStats(allStations, allRoutes);
  }
}

async function loadBookings() {
  const res = await apiGet('/api/bookings');
  if (!res) return;
  const bookings = res.data || [];
  document.getElementById('booking-count').textContent = bookings.length;
  updateBookingStats(bookings);

  const el = document.getElementById('bookings-table');
  if (!bookings.length) { el.innerHTML = '<div class="empty-state"><p>No bookings yet.</p></div>'; return; }
  el.innerHTML = '<table><thead><tr><th>ID</th><th>Route</th><th>Passenger</th><th>VIP</th><th>Status</th><th>Fare</th><th>Date</th></tr></thead><tbody>' +
    bookings.map(b => `<tr>
      <td style="font-weight:600">${esc(b.id)}</td>
      <td>${esc(b.routeId)}</td>
      <td>${esc(b.passengerName)}</td>
      <td>${b.isVip ? '⭐' : '—'}</td>
      <td><span class="badge badge-${b.status.toLowerCase()}">${esc(b.status)}</span></td>
      <td>₹${(b.chargedFare || 0).toFixed(2)}</td>
      <td>${esc(b.travelDate || '—')}</td>
    </tr>`).join('') + '</tbody></table>';
}

async function loadWaitlist() {
  const res = await apiGet('/api/waitlist');
  if (!res) return;
  const entries = res.data || [];
  document.getElementById('waitlist-count').textContent = entries.length;
  const el = document.getElementById('waitlist-table');
  if (!entries.length) { el.innerHTML = '<div class="empty-state"><p>Waitlist is empty.</p></div>'; return; }
  el.innerHTML = '<table><thead><tr><th>ID</th><th>Route</th><th>Passenger</th><th>VIP</th><th>Fare</th></tr></thead><tbody>' +
    entries.map(b => `<tr>
      <td style="font-weight:600">${esc(b.id)}</td><td>${esc(b.routeId)}</td>
      <td>${esc(b.passengerName)}</td><td>${b.isVip ? '⭐' : '—'}</td>
      <td>₹${(b.chargedFare || 0).toFixed(2)}</td>
    </tr>`).join('') + '</tbody></table>';
}

/* ─── Analysis ─── */
async function loadProfitability() {
  setSectionState('profitability', 'loading');
  const res = await apiGet('/api/reports/profitability');
  if (!res) { setSectionState('profitability', 'error', 'Could not fetch profitability report.'); return; }
  const el = document.getElementById('profitability-result');
  const entries = Object.entries(res.data || {});
  if (!entries.length) { setSectionState('profitability', 'success', 'No data.'); return; }

  const maxVal = Math.max(...entries.map(([, v]) => v), 1);
  el.innerHTML = '<div class="profit-bar-container">' +
    entries.map(([rid, val]) => {
      const pct = (val / maxVal) * 100;
      return `<div class="profit-item">
        <span class="profit-label">${esc(rid)}</span>
        <div class="profit-track"><div class="profit-fill" style="width:${pct}%"></div></div>
        <span class="profit-value">₹${val.toFixed(0)}</span>
      </div>`;
    }).join('') + '</div>';
}

async function searchFareRange() {
  const min = document.getElementById('fare-min').value;
  const max = document.getElementById('fare-max').value;
  if (!min || !max) { showToast('Enter both min and max fare', 'error'); return; }
  setSectionState('fareRange', 'loading');
  const res = await apiGet(`/api/fares/range?min=${min}&max=${max}`);
  if (!res) { setSectionState('fareRange', 'error', 'Failed to search fare range.'); return; }
  const el = document.getElementById('fare-range-result');
  const results = res.data?.results || [];
  if (!results.length) { setSectionState('fareRange', 'success', 'No bookings in this range.'); return; }
  el.innerHTML = '<p style="font-size:0.75rem;color:#64748b;margin-bottom:8px">Found ' + results.length + ' booking(s) <span style="color:#94a3b8">· binary search on sorted fares</span></p>' +
    '<table><thead><tr><th>ID</th><th>Route</th><th>Passenger</th><th>Fare</th></tr></thead><tbody>' +
    results.map(b => `<tr>
      <td>${esc(b.id)}</td><td>${esc(b.routeId)}</td>
      <td>${esc(b.passengerName)}</td><td>₹${(b.chargedFare || 0).toFixed(2)}</td>
    </tr>`).join('') + '</tbody></table>';
}

async function loadOccupancy() {
  setSectionState('occupancy', 'loading');
  const res = await apiGet('/api/occupancy-map');
  if (!res) { setSectionState('occupancy', 'error', 'Occupancy data could not be loaded.'); return; }
  const el = document.getElementById('occupancy-map');
  const data = res.data || [];
  if (!data.length) { setSectionState('occupancy', 'success', 'No routes to display.'); return; }
  el.innerHTML = '<div class="occupancy-list">' +
    data.map(r => {
      const pct = r.occupancyPercent;
      const cls = pct < 50 ? 'low' : pct < 80 ? 'medium' : 'high';
      return `<div class="occupancy-item">
        <span class="occupancy-label">${esc(r.routeId)} (${esc(r.fromStationId)}→${esc(r.toStationId)})</span>
        <div class="occupancy-track"><div class="occupancy-fill ${cls}" style="width:${Math.max(pct, 2)}%"></div></div>
        <span class="occupancy-pct">${pct.toFixed(0)}%</span>
      </div>`;
    }).join('') + '</div>';
}

function renderFareTrend(data) {
  const el = document.getElementById('fare-trend-result');
  if (!data.trend || !data.trend.length) { setSectionState('fareTrend', 'success', 'No data.'); return; }

  const maxVal = Math.max(...data.trend.map(t => t.projectedFare), 1);
  const bars = data.trend.map((t, i) => {
    const h = (t.projectedFare / maxVal) * 100;
    const surge = t.occupancyPercent > 80;
    return `<div class="trend-bar" style="height:${h}%;background:${surge ? 'linear-gradient(180deg,#ef4444,#f87171)' : 'linear-gradient(180deg,#2563eb,#60a5fa)'};border-radius:6px 6px 0 0">
      <span class="bar-value" style="color:${surge ? '#dc2626' : '#2563eb'}">₹${t.projectedFare.toFixed(0)}</span>
      <span class="bar-label">Day ${t.day}</span>
    </div>`;
  }).join('');

  el.innerHTML = `<div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:8px">
    <div><span style="font-size:0.85rem;font-weight:600">Route: ${esc(data.routeId)}</span> <span style="font-size:0.75rem;color:#64748b">· ${data.days}-day projection</span></div>
    <div style="font-size:0.7rem;color:#94a3b8"><span style="display:inline-block;width:10px;height:10px;border-radius:2px;background:#2563eb;margin-right:4px"></span> Normal <span style="display:inline-block;width:10px;height:10px;border-radius:2px;background:#ef4444;margin:0 4px 0 12px"></span> Surge (>80%)</div>
  </div>
  <div style="display:flex;align-items:flex-end;gap:6px;height:140px;padding:4px 4px 0;border-bottom:1px solid #e2e8f0">
    ${bars}
  </div>
  <table style="margin-top:12px"><thead><tr><th>Day</th><th>Fare (₹)</th><th>Occupancy</th><th>%</th><th>Status</th></tr></thead><tbody>
    ${data.trend.map(t => `<tr>
      <td>${t.day}</td><td>₹${t.projectedFare.toFixed(2)}</td>
      <td>${t.projectedOccupancy}</td>
      <td>${t.occupancyPercent.toFixed(1)}%</td>
      <td>${t.occupancyPercent > 80 ? '<span class="badge badge-surge">SURGE</span>' : '—'}</td>
    </tr>`).join('')}</tbody></table>`;
}

/* ─── API ─── */
async function apiPost(url, body) {
  try {
    const resp = await fetch(url, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) });
    const j = await resp.json();
    if (!j.success) { showToast(j.message, 'error'); return null; }
    return j;
  } catch (e) { showToast('Network error', 'error'); return null; }
}

async function apiGet(url) {
  try {
    const resp = await fetch(url);
    const j = await resp.json();
    if (!j.success) { showToast(j.message, 'error'); return null; }
    return j;
  } catch (e) { showToast('Network error', 'error'); return null; }
}

function showToast(msg, type) {
  const t = document.getElementById('toast');
  t.textContent = msg;
  t.className = 'hidden';
  void t.offsetWidth;
  t.className = type || '';
  t.classList.remove('hidden');
  if (type !== '') setTimeout(() => t.classList.add('hidden'), 3000);
}

function esc(s) {
  const d = document.createElement('div');
  d.textContent = s;
  return d.innerHTML;
}
