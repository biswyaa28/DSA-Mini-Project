# Viva Preparation Notes — Frontend / Web Developer

## Your Role
You built the web-based user interface that talks to the REST API and displays railway data in a clean, usable dashboard.

## Key Files You Own

| File | Lines | Purpose |
|------|-------|---------|
| `web/index.html` | ~130 | Page structure — tabs, forms, tables |
| `web/styles.css` | ~200 | All styling — colors, layout, responsive |
| `web/app.js` | 231 | All interactivity — API calls, DOM rendering |

---

## Architecture

```
Browser (app.js) ──fetch()──▶ ApiServer (port 8080) ──▶ C++ Engine
                                    │
                              serves web/ folder
                              as static files
```

No build tools, no bundler. Just vanilla HTML/CSS/JS served by the C++ HTTP server.

---

## HTML Structure (`index.html`)

### Tab-Based Layout
```
┌──────────────────────────────────────────┐
│  [Stations] [Routes] [Book] [Search] ... │  ← Tab buttons
├──────────────────────────────────────────┤
│                                          │
│  Content for selected tab                │  ← Tab content (shown/hidden)
│  - Forms (add station, book seat, etc.)  │
│  - Tables (list stations, routes, etc.)  │
│  - Results (path, MST, fare trend, etc.) │
│                                          │
└──────────────────────────────────────────┘
```

### Tabs Implementation
```html
<button class="tab-btn active" data-tab="stations">Stations</button>
<button class="tab-btn" data-tab="routes">Routes</button>
<!-- ... more tabs ... -->

<div id="tab-stations" class="tab-content">
  <!-- form + table -->
</div>
<div id="tab-routes" class="tab-content hidden">
  <!-- form + table -->
</div>
```

### Key HTML Elements
- **Forms**: Each with `id="X-form"` and `submit` event — `station-form`, `route-form`, `book-form`, `cancel-form`, `route-search-form`, `mst-form`, `fare-trend-form`
- **Tables**: `stations-table`, `routes-table`, `bookings-table`, `waitlist-table`
- **Result containers**: `path-result`, `mst-result`, `profitability-result`, `occupancy-map`, `fare-trend-result`
- **Toast**: Notification popup at top

---

## CSS Styling (`styles.css`)

### Key Features
- **Dark theme**: Dark background, light text — modern dashboard look
- **Badges**: `.badge-confirmed` (green), `.badge-waitlisted` (yellow), `.badge-cancelled` (red), `.badge-surge` (orange/pulse animation)
- **Occupancy bars**: Horizontal bar chart with color coding
  - `< 50%`: green (low)
  - `50-80%`: orange (medium)
  - `> 80%`: red (high)
- **Responsive**: Tables scroll horizontally on small screens
- **Toast notifications**: Slide-in from top, auto-dismiss after 3 seconds

### CSS Concepts to Know
- **Flexbox**: Used for layout (`.container`, tabs, form rows)
- **CSS Grid**: Could be used but flexbox was sufficient here
- **Transitions/Animations**: `.badge-surge` pulsing, `.toast` slide-in
- **Media queries**: Any? (check if responsive)

---

## JavaScript Logic (`app.js`)

### Initialization
```javascript
document.addEventListener('DOMContentLoaded', () => {
  initTabs();    // Set up tab switching
  initForms();   // Attach submit handlers to all forms
  loadStations(); // Load initial data
  loadRoutes();
});
```

### Tab Switching
```javascript
function initTabs() {
  document.querySelectorAll('.tab-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      // Remove 'active' from all tabs
      // Add 'hidden' to all content panels
      // Activate clicked tab + show its panel
    });
  });
}
```

### API Communication (Two Helper Functions)

```javascript
async function apiPost(url, body) {
  const resp = await fetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body)
  });
  const j = await resp.json();
  if (!j.success) { showToast(j.message, 'error'); return null; }
  return j;
}

async function apiGet(url) {
  const resp = await fetch(url);
  const j = await resp.json();
  if (!j.success) { showToast(j.message, 'error'); return null; }
  return j;
}
```

**Key point**: Every AJAX call goes through one of these two functions. Error handling is centralized — if the API returns `success: false`, a toast appears automatically.

### Data Loading Pattern (Example: Stations)
```javascript
async function loadStations() {
  const res = await apiGet('/api/stations');
  if (!res) return;
  const el = document.getElementById('stations-table');
  if (!res.data || res.data.length === 0) {
    el.innerHTML = '<p class="hint">No stations yet.</p>';
    return;
  }
  el.innerHTML = '<table>...' +
    res.data.map(s => `<tr><td>${esc(s.id)}</td><td>${esc(s.name)}</td></tr>`).join('') +
    '</table>';
}
```

**Pattern**: Fetch → check error → check empty → build HTML → inject.

### XSS Prevention
```javascript
function esc(s) {
  const d = document.createElement('div');
  d.textContent = s;
  return d.innerHTML;  // Escapes < > & etc.
}
```

### Occupancy Bar Visualization
```javascript
const pct = r.occupancyPercent;
const cls = pct < 50 ? 'low' : pct < 80 ? 'medium' : 'high';
// Renders: <div class="occupancy-fill low" style="width:42%"></div>
```

### Fare Trend Table
```javascript
// Displays day-by-day projected fare × occupancy for N days
data.trend.map(t => `<tr>
  <td>${t.day}</td>
  <td>₹${t.projectedFare.toFixed(2)}</td>
  <td>${t.projectedOccupancy}</td>
  <td>${t.occupancyPercent.toFixed(1)}%</td>
</tr>`)
```

---

## Common Viva Questions

**Q: How does the frontend communicate with the backend?** Via HTTP `fetch()` API — GET requests for reading data, POST requests with JSON body for creating data. All responses are JSON with `{success, message, data}` format.

**Q: Why no React/Vue/Angular?** This is a C++ DSA mini project, not a production web app. Vanilla JS keeps dependencies to zero — the only HTTP library is on the C++ side. Simpler to understand for a viva.

**Q: How do you handle errors?** Two layers: (1) `apiPost`/`apiGet` check the `success` field and show a toast on failure. (2) Try-catch catches network errors (server down, etc.) and shows "Network error" toast.

**Q: How does the occupancy bar chart work?** It's pure CSS — a `div` with percentage width and color class. No canvas, no chart library. Green (<50%), orange (<80%), red (≥80%).

**Q: How does the tab system work?** CSS class toggling. `classList.toggle('hidden')` shows/hides content panels. Only one tab's content is visible at a time. No routing, no URL hash — simple and effective.

**Q: What if the API returns unexpected data?** The code assumes the API always returns the expected shape. If fields are missing, the table cells would show `undefined`. Not robust, but fine for a demo project.

---

## Data Flow Summary

```
User clicks "Book" → form submit → apiPost() → fetch POST /api/bookings
  → C++ processes booking → returns JSON
  → apiPost() checks success → shows toast
  → loadBookings(), loadWaitlist(), loadRoutes() refresh tables
```

All data loading functions are called after mutations (create/cancel) to reflect the latest state.
