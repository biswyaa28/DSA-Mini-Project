# Web UI Redesign Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rewrite web frontend (HTML/CSS/JS) with a professional dark sidebar layout, interactive network graph, and polished UI. Zero backend changes.

**Architecture:** Three files rewritten — `index.html` (structure), `styles.css` (design system), `app.js` (logic + graph). All existing API calls preserved. vis.js CDN for network graph.

**Tech Stack:** HTML5, CSS3, Vanilla JS, vis-network.js (CDN)

---

### Task 1: Write styles.css

**Files:**
- Rewrite: `web/styles.css`

This is the full design system — dark sidebar, cards, tables, forms, badges, graph container, animations.

- [ ] **Step 1: Write styles.css with the complete design system** (full file content)

```css
/* ============================================
   Railway Network DSA System — Design System
   Colors: Navy #0f172a, Blue #2563eb, Teal #0891b2
   ============================================ */

*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

body {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
  background: #f1f5f9;
  color: #0f172a;
  line-height: 1.6;
  min-height: 100vh;
}

/* Layout */
.app-layout { display: flex; min-height: 100vh; }

/* ─── Sidebar ─── */
.sidebar {
  width: 240px;
  background: #0f172a;
  display: flex;
  flex-direction: column;
  flex-shrink: 0;
  position: sticky;
  top: 0;
  height: 100vh;
  padding: 20px 12px;
}
.sidebar-logo {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 8px 12px;
  margin-bottom: 24px;
  font-size: 1.1rem;
  font-weight: 700;
  color: #fff;
}
.sidebar-nav { display: flex; flex-direction: column; gap: 2px; flex: 1; }
.nav-item {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 10px 12px;
  border-radius: 8px;
  font-size: 0.85rem;
  color: #94a3b8;
  cursor: pointer;
  transition: all 0.2s ease;
  border: none;
  background: none;
  width: 100%;
  text-align: left;
}
.nav-item:hover { background: rgba(255,255,255,0.05); color: #e2e8f0; }
.nav-item.active { background: #1e293b; color: #fff; font-weight: 500; }
.nav-badge {
  margin-left: auto;
  font-size: 0.6rem;
  padding: 2px 6px;
  border-radius: 4px;
  font-weight: 600;
  letter-spacing: 0.3px;
}
.nav-badge.dijkstra { background: #7c3aed; color: #fff; }
.nav-badge.prim { background: #059669; color: #fff; }
.sidebar-footer {
  margin-top: auto;
  padding-top: 16px;
  border-top: 1px solid #1e293b;
  text-align: center;
  font-size: 0.7rem;
  color: #475569;
}

/* ─── Main Content ─── */
.main-content { flex: 1; padding: 24px; overflow-y: auto; max-width: calc(100vw - 240px); }

/* ─── Top Bar ─── */
.top-bar {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 16px 24px;
  background: #fff;
  border-radius: 12px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
  margin-bottom: 24px;
}
.top-bar-title { font-size: 1.15rem; font-weight: 700; color: #0f172a; }
.top-bar-subtitle { font-size: 0.8rem; color: #64748b; margin-top: 2px; }
.top-bar-pills { display: flex; gap: 8px; }
.pill {
  padding: 4px 12px;
  border-radius: 20px;
  font-size: 0.7rem;
  font-weight: 500;
  background: #f1f5f9;
  color: #475569;
}
.pill.success { background: #f0fdf4; color: #16a34a; }
.pill.primary { background: #eff6ff; color: #2563eb; }

/* ─── Page Content ─── */
.page { display: none; animation: fadeIn 0.3s ease; }
.page.active { display: block; }

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(8px); }
  to { opacity: 1; transform: translateY(0); }
}

/* ─── Stat Cards ─── */
.stats-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 16px; margin-bottom: 24px; }
.stat-card {
  background: #fff;
  border-radius: 12px;
  padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
  transition: transform 0.2s ease, box-shadow 0.2s ease;
  cursor: default;
}
.stat-card:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(0,0,0,0.1); }
.stat-label { font-size: 0.7rem; color: #64748b; text-transform: uppercase; font-weight: 600; letter-spacing: 0.5px; }
.stat-value { font-size: 1.8rem; font-weight: 700; color: #0f172a; margin-top: 4px; }
.stat-sub { font-size: 0.75rem; margin-top: 4px; }
.stat-sub.green { color: #22c55e; }
.stat-sub.muted { color: #94a3b8; }

/* ─── Graph ─── */
.graph-card {
  background: #fff;
  border-radius: 12px;
  padding: 20px;
  box-shadow: 0 1px 3px rgba(0,0,0,0.06);
  margin-bottom: 24px;
}
.graph-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 16px;
}
.graph-title { font-size: 0.9rem; font-weight: 600; color: #0f172a; }
#network-graph {
  width: 100%;
  height: 400px;
  background: linear-gradient(135deg, #f8fafc, #f1f5f9);
  border-radius: 8px;
  border: 1px solid #e2e8f0;
}

/* ─── Cards ─── */
.card { background: #fff; border-radius: 10px; padding: 20px; box-shadow: 0 1px 3px rgba(0,0,0,0.06); margin-bottom: 16px; }
.card-row { display: flex; gap: 16px; margin-bottom: 16px; }
.card-row .card { flex: 1; margin-bottom: 0; }
.card-title { font-size: 0.9rem; font-weight: 600; color: #0f172a; margin-bottom: 12px; display: flex; align-items: center; gap: 8px; }
.card-count { margin-left: auto; background: #f1f5f9; color: #475569; font-size: 0.65rem; padding: 2px 8px; border-radius: 4px; font-weight: 400; }

/* ─── Section Headers ─── */
.section-header { display: flex; align-items: center; gap: 8px; margin-bottom: 16px; }
.section-header h2 { font-size: 1rem; font-weight: 700; color: #0f172a; }
.dsa-badge {
  font-size: 0.6rem;
  padding: 3px 8px;
  border-radius: 4px;
  font-weight: 600;
  letter-spacing: 0.3px;
}
.dsa-badge.fifo { background: #ecfeff; color: #0e7490; }
.dsa-badge.lifo { background: #fef2f2; color: #dc2626; }
.dsa-badge.dijkstra { background: #ede9fe; color: #6d28d9; }
.dsa-badge.prim { background: #d1fae5; color: #065f46; }
.dsa-badge.sorting { background: #fef3c7; color: #92400e; }
.dsa-badge.binary { background: #eff6ff; color: #1d4ed8; }

/* ─── Forms ─── */
.form-group { margin-bottom: 10px; }
.form-row { display: flex; gap: 8px; }
.form-row .form-group { flex: 1; }
label { display: block; font-size: 0.75rem; font-weight: 500; color: #475569; margin-bottom: 4px; }
input, select {
  width: 100%;
  padding: 10px 12px;
  border: 1px solid #e2e8f0;
  border-radius: 8px;
  font-size: 0.85rem;
  background: #f8fafc;
  outline: none;
  transition: border-color 0.2s, box-shadow 0.2s;
  color: #0f172a;
}
input:focus, select:focus { border-color: #2563eb; box-shadow: 0 0 0 3px rgba(37,99,235,0.1); }
input::placeholder { color: #94a3b8; }
.checkbox-group { display: flex; align-items: center; gap: 8px; padding: 8px 0; }
.checkbox-group input[type="checkbox"] { width: auto; accent-color: #2563eb; }
.checkbox-group label { margin-bottom: 0; font-size: 0.85rem; color: #475569; cursor: pointer; }

/* ─── Buttons ─── */
.btn {
  padding: 10px 20px;
  border: none;
  border-radius: 8px;
  font-size: 0.85rem;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.15s ease;
  display: inline-flex;
  align-items: center;
  gap: 6px;
}
.btn:active { transform: scale(0.97); }
.btn-primary { background: #2563eb; color: #fff; }
.btn-primary:hover { background: #1d4ed8; }
.btn-danger { background: #ef4444; color: #fff; }
.btn-danger:hover { background: #dc2626; }
.btn-success { background: #16a34a; color: #fff; }
.btn-success:hover { background: #15803d; }
.btn-ghost { background: #f1f5f9; color: #475569; }
.btn-ghost:hover { background: #e2e8f0; }
.btn-sm { padding: 6px 12px; font-size: 0.75rem; }
.btn-full { width: 100%; justify-content: center; }

.btn-group { display: flex; gap: 4px; margin-bottom: 10px; }
.btn-group .btn { flex: 1; justify-content: center; }
.btn-group .btn.active { background: #2563eb; color: #fff; }
.btn-group .btn:not(.active) { background: #f1f5f9; color: #475569; }

/* ─── Tables ─── */
.table-container { overflow-x: auto; }
table { width: 100%; border-collapse: collapse; font-size: 0.8rem; }
th {
  background: #f8fafc;
  padding: 10px 12px;
  text-align: left;
  font-weight: 600;
  color: #64748b;
  font-size: 0.7rem;
  text-transform: uppercase;
  letter-spacing: 0.5px;
  border-bottom: 2px solid #e2e8f0;
}
td { padding: 10px 12px; border-bottom: 1px solid #f1f5f9; color: #334155; }
tr:hover td { background: #f8fafc; }
tr:last-child td { border-bottom: none; }
.text-muted { color: #94a3b8; }

/* ─── Badges ─── */
.badge {
  display: inline-block;
  padding: 3px 10px;
  border-radius: 20px;
  font-size: 0.7rem;
  font-weight: 600;
}
.badge-confirmed { background: #dcfce7; color: #16a34a; }
.badge-waitlisted { background: #fef3c7; color: #d97706; }
.badge-cancelled { background: #fee2e2; color: #dc2626; }
.badge-surge { background: #fef2f2; color: #ef4444; }

/* ─── Result Box ─── */
.result-box {
  background: linear-gradient(135deg, #f0f9ff, #e0f2fe);
  border-radius: 10px;
  padding: 20px;
  margin-top: 12px;
  border: 1px solid #bae6fd;
}
.result-box.success { background: linear-gradient(135deg, #f0fdf4, #dcfce7); border-color: #bbf7d0; }
.result-box .path-stops { display: flex; align-items: center; flex-wrap: wrap; gap: 6px; }
.result-box .route-stop {
  background: #2563eb;
  color: #fff;
  padding: 4px 14px;
  border-radius: 20px;
  font-weight: 600;
  font-size: 0.85rem;
}
.result-box .route-edge {
  color: #94a3b8;
  font-size: 0.75rem;
  display: flex;
  align-items: center;
  gap: 4px;
}
.result-box .route-edge .edge-label {
  background: rgba(255,255,255,0.7);
  padding: 2px 6px;
  border-radius: 4px;
  font-size: 0.65rem;
  color: #64748b;
}
.result-meta { display: flex; gap: 16px; margin-top: 10px; font-size: 0.8rem; color: #475569; }
.result-meta strong { color: #0f172a; }

/* ─── MST Routes ─── */
.mst-pills { display: flex; flex-wrap: wrap; gap: 6px; margin-top: 8px; }
.mst-pill {
  background: #059669;
  color: #fff;
  padding: 4px 12px;
  border-radius: 20px;
  font-size: 0.75rem;
  font-weight: 500;
}

/* ─── Occupancy Bars ─── */
.occupancy-list { display: flex; flex-direction: column; gap: 12px; }
.occupancy-item { display: flex; align-items: center; gap: 12px; }
.occupancy-label { width: 120px; font-size: 0.8rem; color: #475569; flex-shrink: 0; }
.occupancy-track { flex: 1; height: 28px; background: #f1f5f9; border-radius: 6px; overflow: hidden; position: relative; }
.occupancy-fill { height: 100%; border-radius: 6px; transition: width 0.6s ease; min-width: 4px; }
.occupancy-fill.low { background: linear-gradient(90deg, #22c55e, #4ade80); }
.occupancy-fill.medium { background: linear-gradient(90deg, #f59e0b, #fbbf24); }
.occupancy-fill.high { background: linear-gradient(90deg, #ef4444, #f87171); }
.occupancy-pct { font-size: 0.8rem; font-weight: 600; width: 45px; flex-shrink: 0; color: #475569; }

/* ─── Toast ─── */
#toast {
  position: fixed;
  bottom: 24px;
  left: 50%;
  transform: translateX(-50%) translateY(0);
  background: #0f172a;
  color: #fff;
  padding: 12px 24px;
  border-radius: 10px;
  font-size: 0.85rem;
  z-index: 9999;
  box-shadow: 0 4px 20px rgba(0,0,0,0.2);
  transition: opacity 0.3s ease, transform 0.3s ease;
}
#toast.hidden { opacity: 0; transform: translateX(-50%) translateY(20px); pointer-events: none; }
#toast.error { background: #dc2626; }
#toast.success { background: #16a34a; }

/* ─── Loading ─── */
.spinner {
  display: inline-block;
  width: 20px;
  height: 20px;
  border: 3px solid #e2e8f0;
  border-top-color: #2563eb;
  border-radius: 50%;
  animation: spin 0.6s linear infinite;
}
@keyframes spin { to { transform: rotate(360deg); } }
.loading-overlay { display: flex; align-items: center; justify-content: center; padding: 40px; gap: 10px; color: #64748b; font-size: 0.85rem; }

/* ─── Graph Modal ─── */
.graph-modal {
  display: none;
  position: fixed;
  inset: 0;
  background: rgba(0,0,0,0.6);
  z-index: 9998;
  align-items: center;
  justify-content: center;
  backdrop-filter: blur(4px);
}
.graph-modal.open { display: flex; }
.graph-modal-content {
  width: 90vw;
  height: 90vh;
  background: #fff;
  border-radius: 16px;
  overflow: hidden;
  display: flex;
  flex-direction: column;
  box-shadow: 0 20px 60px rgba(0,0,0,0.3);
}
.graph-modal-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 16px 24px;
  border-bottom: 1px solid #e2e8f0;
}
.graph-modal-body { flex: 1; }
.graph-modal-body #network-graph { height: 100%; border: none; border-radius: 0; }
.graph-modal-close {
  background: #f1f5f9;
  border: none;
  width: 36px;
  height: 36px;
  border-radius: 50%;
  font-size: 1.2rem;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #64748b;
  transition: background 0.2s;
}
.graph-modal-close:hover { background: #e2e8f0; }

/* ─── Empty State ─── */
.empty-state { text-align: center; padding: 40px 20px; color: #94a3b8; }
.empty-state .icon { font-size: 2rem; margin-bottom: 8px; }
.empty-state p { font-size: 0.85rem; }

/* ─── Responsive ─── */
@media (max-width: 900px) {
  .stats-grid { grid-template-columns: repeat(2, 1fr); }
  .card-row { flex-direction: column; }
  .card-row .card { margin-bottom: 0; }
}
@media (max-width: 700px) {
  .sidebar { width: 60px; padding: 12px 8px; }
  .sidebar-logo span, .nav-item span:not(.nav-icon), .nav-badge, .sidebar-footer { display: none; }
  .nav-item { justify-content: center; padding: 10px; }
  .main-content { max-width: calc(100vw - 60px); padding: 16px; }
  .stats-grid { grid-template-columns: 1fr 1fr; }
  .top-bar { flex-direction: column; gap: 8px; align-items: flex-start; }
}
```

- [ ] **Step 2: Write the file** using the Write tool to `web/styles.css`

---

### Task 2: Write index.html

**Files:**
- Rewrite: `web/index.html`

The HTML structure with sidebar navigation and all section pages.

- [ ] **Step 1: Write index.html** (full file content)

Write a complete new `web/index.html` with:
- CDN links for vis-network CSS and JS
- App layout: sidebar + main content
- Sidebar: logo, 6 nav items with DSA badges, footer
- Top bar with title/subtitle
- 6 page sections: overview, stations, booking, routing, mst, analysis
- Overview: stat cards + graph container
- Stations: add station form + add route form + tables
- Booking: book form + cancel form + bookings table + waitlist
- Routing: search from/to + mode toggle + path result
- MST: start station input + result
- Analysis: 4-card grid (profitability, fare range, occupancy, fare trend)
- Toast notification
- Graph modal (fullscreen graph)

```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Railway Network DSA System</title>
  <link rel="stylesheet" href="styles.css">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/vis-network/9.1.6/dist/vis-network.min.css">
</head>
<body>
  <div class="app-layout">
    <!-- Sidebar -->
    <aside class="sidebar">
      <div class="sidebar-logo">
        <span>🚂</span>
        <span>RailDSA</span>
      </div>
      <nav class="sidebar-nav">
        <button class="nav-item active" data-page="overview">
          <span class="nav-icon">📊</span>
          <span>Overview</span>
        </button>
        <button class="nav-item" data-page="stations">
          <span class="nav-icon">🚉</span>
          <span>Stations &amp; Routes</span>
        </button>
        <button class="nav-item" data-page="booking">
          <span class="nav-icon">🎫</span>
          <span>Booking</span>
        </button>
        <button class="nav-item" data-page="routing">
          <span class="nav-icon">🛤️</span>
          <span>Route Finder</span>
          <span class="nav-badge dijkstra">Dijkstra</span>
        </button>
        <button class="nav-item" data-page="mst">
          <span class="nav-icon">🌲</span>
          <span>MST Report</span>
          <span class="nav-badge prim">Prim's</span>
        </button>
        <button class="nav-item" data-page="analysis">
          <span class="nav-icon">📈</span>
          <span>Analysis</span>
        </button>
      </nav>
      <div class="sidebar-footer">DSA Mini Project</div>
    </aside>

    <!-- Main Content -->
    <main class="main-content">

      <!-- Top Bar -->
      <div class="top-bar">
        <div>
          <div class="top-bar-title" id="page-title">Overview</div>
          <div class="top-bar-subtitle" id="page-subtitle">Dashboard summary of your railway network</div>
        </div>
        <div class="top-bar-pills">
          <span class="pill success" id="status-pill">● Connected</span>
        </div>
      </div>

      <!-- ==================== OVERVIEW ==================== -->
      <div class="page active" id="page-overview">
        <div class="stats-grid" id="stats-grid">
          <div class="stat-card">
            <div class="stat-label">Stations</div>
            <div class="stat-value" id="stat-stations">—</div>
            <div class="stat-sub muted">Loading...</div>
          </div>
          <div class="stat-card">
            <div class="stat-label">Routes</div>
            <div class="stat-value" id="stat-routes">—</div>
            <div class="stat-sub muted">Loading...</div>
          </div>
          <div class="stat-card">
            <div class="stat-label">Bookings</div>
            <div class="stat-value" id="stat-bookings">—</div>
            <div class="stat-sub muted">Loading...</div>
          </div>
          <div class="stat-card">
            <div class="stat-label">Revenue</div>
            <div class="stat-value" id="stat-revenue">—</div>
            <div class="stat-sub muted">Loading...</div>
          </div>
        </div>

        <div class="graph-card">
          <div class="graph-header">
            <div class="graph-title">🌐 Network Graph</div>
            <div>
              <button class="btn btn-ghost btn-sm" id="expand-graph-btn">⛶ Expand</button>
              <button class="btn btn-ghost btn-sm" id="reset-graph-btn">⟲ Reset View</button>
            </div>
          </div>
          <div id="network-graph"></div>
        </div>
      </div>

      <!-- ==================== STATIONS & ROUTES ==================== -->
      <div class="page" id="page-stations">
        <div class="section-header">
          <h2>🚉 Stations &amp; Routes</h2>
        </div>
        <div class="card-row">
          <div class="card">
            <div class="card-title">Add Station</div>
            <form id="station-form">
              <div class="form-group">
                <input type="text" name="id" placeholder="Station ID (e.g. S1)" required>
              </div>
              <div class="form-group">
                <input type="text" name="name" placeholder="Station Name (e.g. Central)" required>
              </div>
              <button type="submit" class="btn btn-primary btn-full">+ Add Station</button>
            </form>
          </div>
          <div class="card">
            <div class="card-title">Add Route</div>
            <form id="route-form">
              <div class="form-row">
                <div class="form-group">
                  <input type="text" name="id" placeholder="Route ID" required>
                </div>
                <div class="form-group">
                  <input type="text" name="fromStationId" placeholder="From" required>
                </div>
                <div class="form-group">
                  <input type="text" name="toStationId" placeholder="To" required>
                </div>
              </div>
              <div class="form-row">
                <div class="form-group">
                  <input type="number" name="distanceKm" placeholder="Distance (km)" required>
                </div>
                <div class="form-group">
                  <input type="number" step="0.01" name="baseFare" placeholder="Fare (₹)" required>
                </div>
                <div class="form-group">
                  <input type="number" name="capacity" placeholder="Capacity" required>
                </div>
              </div>
              <button type="submit" class="btn btn-primary btn-full">+ Add Route</button>
            </form>
          </div>
        </div>
        <div class="card-row">
          <div class="card">
            <div class="card-title">
              Stations
              <span class="card-count" id="station-count">0</span>
              <button class="btn btn-ghost btn-sm" style="margin-left:auto" onclick="loadStations(); loadRoutes();">⟳ Refresh</button>
            </div>
            <div class="table-container" id="stations-table">
              <div class="empty-state"><p>No stations yet.</p></div>
            </div>
          </div>
          <div class="card">
            <div class="card-title">
              Routes
              <span class="card-count" id="route-count">0</span>
            </div>
            <div class="table-container" id="routes-table">
              <div class="empty-state"><p>No routes yet.</p></div>
            </div>
          </div>
        </div>
      </div>

      <!-- ==================== BOOKING ==================== -->
      <div class="page" id="page-booking">
        <div class="section-header">
          <h2>🎫 Booking</h2>
          <span class="dsa-badge fifo">Queue — FIFO</span>
          <span class="dsa-badge lifo">Stack — LIFO</span>
        </div>
        <div class="card-row">
          <div class="card">
            <div class="card-title">Book a Seat</div>
            <form id="book-form">
              <div class="form-group">
                <input type="text" name="routeId" placeholder="Route ID (e.g. R1)" required>
              </div>
              <div class="form-group">
                <input type="text" name="passengerName" placeholder="Passenger Name" required>
              </div>
              <div class="form-row">
                <div class="form-group">
                  <input type="text" name="travelDate" placeholder="Travel Date (optional)">
                </div>
                <div class="checkbox-group">
                  <input type="checkbox" name="isVip" id="vip-check">
                  <label for="vip-check">VIP</label>
                </div>
              </div>
              <button type="submit" class="btn btn-primary btn-full">Book Seat</button>
            </form>
          </div>
          <div class="card">
            <div class="card-title">Cancel Booking</div>
            <form id="cancel-form">
              <div class="form-group">
                <input type="text" name="bookingId" placeholder="Booking ID (e.g. B1)" required>
              </div>
              <button type="submit" class="btn btn-danger btn-full">Cancel Booking</button>
            </form>
            <div style="margin-top:12px;padding:12px;background:#fef2f2;border-radius:8px;">
              <div style="font-size:0.7rem;color:#dc2626;font-weight:600">Refund Stack (LIFO)</div>
              <div style="font-size:0.75rem;color:#991b1b;margin-top:2px" id="last-refund">No cancellations yet.</div>
            </div>
          </div>
        </div>
        <div class="card-row">
          <div class="card">
            <div class="card-title">
              All Bookings
              <span class="card-count" id="booking-count">0</span>
              <button class="btn btn-ghost btn-sm" style="margin-left:auto" onclick="loadBookings(); loadWaitlist();">⟳ Refresh</button>
            </div>
            <div class="table-container" id="bookings-table">
              <div class="empty-state"><p>No bookings yet.</p></div>
            </div>
          </div>
          <div class="card">
            <div class="card-title">
              Waitlist Queue
              <span class="card-count" id="waitlist-count">0</span>
            </div>
            <div class="table-container" id="waitlist-table">
              <div class="empty-state"><p>Waitlist is empty.</p></div>
            </div>
          </div>
        </div>
      </div>

      <!-- ==================== ROUTE FINDER ==================== -->
      <div class="page" id="page-routing">
        <div class="section-header">
          <h2>🛤️ Route Finder</h2>
          <span class="dsa-badge dijkstra">Algorithm: Dijkstra</span>
        </div>
        <div class="card-row">
          <div class="card" style="max-width:320px">
            <div class="card-title">Search Parameters</div>
            <form id="route-search-form">
              <div class="form-group">
                <input type="text" name="from" placeholder="From Station ID" required>
              </div>
              <div class="form-group">
                <input type="text" name="to" placeholder="To Station ID" required>
              </div>
              <div class="btn-group" id="mode-toggle">
                <button type="button" class="btn btn-sm active" data-mode="shortest">Shortest</button>
                <button type="button" class="btn btn-sm" data-mode="least_crowded">Least Crowded</button>
              </div>
              <button type="submit" class="btn btn-primary btn-full">Find Path</button>
            </form>
          </div>
          <div class="card" style="flex:1">
            <div class="card-title">Path Result</div>
            <div id="path-result">
              <div class="empty-state"><p>Search for a route to see results.</p></div>
            </div>
          </div>
        </div>
      </div>

      <!-- ==================== MST REPORT ==================== -->
      <div class="page" id="page-mst">
        <div class="section-header">
          <h2>🌲 MST Report</h2>
          <span class="dsa-badge prim">Algorithm: Prim's</span>
        </div>
        <div class="card-row">
          <div class="card" style="max-width:320px">
            <div class="card-title">Build Minimum Spanning Tree</div>
            <form id="mst-form">
              <div class="form-group">
                <input type="text" name="start" placeholder="Start Station ID" required>
              </div>
              <button type="submit" class="btn btn-success btn-full">Build MST</button>
            </form>
          </div>
          <div class="card" style="flex:1">
            <div class="card-title">Result</div>
            <div id="mst-result">
              <div class="empty-state"><p>Enter a start station and build the MST.</p></div>
            </div>
          </div>
        </div>
      </div>

      <!-- ==================== ANALYSIS ==================== -->
      <div class="page" id="page-analysis">
        <div class="section-header">
          <h2>📈 Analysis</h2>
          <span class="dsa-badge sorting">Sorting</span>
          <span class="dsa-badge binary">Binary Search</span>
        </div>
        <div class="card-row">
          <div class="card">
            <div class="card-title">💰 Profitability by Route</div>
            <button class="btn btn-primary btn-sm" onclick="loadProfitability()">Generate Report</button>
            <div id="profitability-result" style="margin-top:12px"></div>
          </div>
          <div class="card">
            <div class="card-title">🎯 Fare Range Search</div>
            <p style="font-size:0.75rem;color:#64748b;margin-bottom:10px">Binary search: find bookings within a fare range</p>
            <div class="form-row">
              <div class="form-group">
                <input type="number" id="fare-min" step="0.01" placeholder="Min (₹)">
              </div>
              <div class="form-group">
                <input type="number" id="fare-max" step="0.01" placeholder="Max (₹)">
              </div>
            </div>
            <button class="btn btn-primary btn-sm" onclick="searchFareRange()">Search</button>
            <div id="fare-range-result" style="margin-top:12px"></div>
          </div>
        </div>
        <div class="card-row">
          <div class="card">
            <div class="card-title">📊 Seat Occupancy Map</div>
            <button class="btn btn-primary btn-sm" onclick="loadOccupancy()">Load Map</button>
            <div id="occupancy-map" style="margin-top:12px"></div>
          </div>
          <div class="card">
            <div class="card-title">📈 Fare Trend Predictor</div>
            <form id="fare-trend-form" style="margin-bottom:10px">
              <div class="form-row">
                <div class="form-group">
                  <input type="text" name="routeId" placeholder="Route ID" required>
                </div>
                <div class="form-group">
                  <input type="number" name="days" placeholder="Days (1-30)" min="1" max="30" required>
                </div>
              </div>
              <button type="submit" class="btn btn-primary btn-sm">Project Fares</button>
            </form>
            <div id="fare-trend-result"></div>
          </div>
        </div>
      </div>

    </main>
  </div>

  <!-- Graph Fullscreen Modal -->
  <div class="graph-modal" id="graph-modal">
    <div class="graph-modal-content">
      <div class="graph-modal-header">
        <div style="font-weight:600;color:#0f172a">🌐 Network Graph</div>
        <button class="graph-modal-close" id="graph-modal-close">✕</button>
      </div>
      <div class="graph-modal-body">
        <div id="network-graph-fullscreen"></div>
      </div>
    </div>
  </div>

  <div id="toast" class="hidden"></div>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/vis-network/9.1.6/vis-network.min.js"></script>
  <script src="app.js"></script>
</body>
</html>
```

- [ ] **Step 2: Write the file** using the Write tool to `web/index.html`

---

### Task 3: Write app.js

**Files:**
- Rewrite: `web/app.js`

Full rewrite keeping the same API calls but with new UI rendering, sidebar navigation, and vis.js graph.

- [ ] **Step 1: Write app.js** — full file content

The app.js must:
1. Initialize sidebar navigation (tab switching)
2. Initialize vis.js network graph (stations → nodes, routes → edges)
3. Style graph nodes/edges per design system
4. Expose functions to highlight path edges (Dijkstra) and MST edges (Prim's)
5. Render all data tables with the new card design
6. Handle graph fullscreen modal
7. Preserve all API calls (`apiGet`, `apiPost`) and response handling

Key functions:
```
initSidebar(), switchPage(page)
initGraph(stations, routes)  — vis.js network
highlightPath(routeIds)      — blue highlight
highlightMST(routeIds)       — green highlight
resetGraph()                 — back to default
updateStats(stations, routes, bookings)
loadStations(), loadRoutes()
loadBookings(), loadWaitlist()
findPath()                   — Dijkstra with visual highlight
buildMST()                   — Prim's with visual highlight
loadProfitability(), loadOccupancy(), searchFareRange()
renderFareTrend(data)
apiPost(), apiGet(), showToast()
```

- [ ] **Step 2: Write the file** using the Write tool to `web/app.js`

Then build, run, and test all features work with the new UI.

---

### Task 4: Build, run, and test

- [ ] **Step 1: Build the C++ backend**

```bash
cmake --build build 2>&1 | tail -5
```

Expected: Build succeeds

- [ ] **Step 2: Start the server and test the UI**

```bash
./build/railway_app &
sleep 1
# Test that all API endpoints work
curl -s http://localhost:8080/api/stations | python3 -c "import sys,json;d=json.load(sys.stdin);print(f'{len(d[\"data\"])} stations')"
curl -s http://localhost:8080/api/routes | python3 -c "import sys,json;d=json.load(sys.stdin);print(f'{len(d[\"data\"])} routes')"
kill %1 2>/dev/null
```

Expected: "5 stations", "7 routes"

- [ ] **Step 3: Verify the web UI loads correctly**

Open `http://localhost:8080` in a browser and verify:
- Sidebar navigation works (click each tab)
- Overview shows stat cards with data
- Network graph renders stations and routes
- Stations/Routes page shows seed data
- Booking page forms submit correctly

- [ ] **Step 4: Run backend tests to ensure nothing broke**

```bash
ctest --test-dir build -V
```

Expected: All 6 tests pass
