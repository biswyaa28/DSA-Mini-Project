# Web UI Exhibition Dashboard Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Upgrade the existing frontend into the approved Exhibition Dashboard experience with stronger visual impact, moderate cinematic motion, and clearer UX, while keeping all backend/API behavior unchanged.

**Architecture:** Keep the same frontend boundaries (`web/index.html`, `web/styles.css`, `web/app.js`) and same backend endpoints. Implement the redesign in TDD order using a lightweight frontend contract test (`tests/test_ui_contract.py`) that verifies HTML structure, CSS tokens/motion, and JS interaction helpers. Then run full browser smoke checks and existing backend tests.

**Tech Stack:** HTML5, CSS3 (custom properties), Vanilla JavaScript, vis-network CDN, Python `unittest`, CMake/CTest.

---

## File Structure and Responsibilities

- `web/index.html`
  - Presentation structure only: navigation rail, hero strip, section shells, explicit labels/helper text, graph legend/callout, state containers.
- `web/styles.css`
  - Exhibition visual system: color tokens, typography, spacing/elevation, motion, responsive behavior, loading/empty/error states.
- `web/app.js`
  - UI orchestration: tab metadata, state rendering, hero metrics updates, graph mode callouts, existing API calls unchanged.
- `tests/test_ui_contract.py` (new)
  - Frontend contract tests for static UI requirements from the approved spec.

---

### Task 1: Add Frontend Contract Tests (Failing First)

**Files:**
- Create: `tests/test_ui_contract.py`

- [ ] **Step 1: Write the failing frontend contract test file**

```python
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]
WEB_DIR = ROOT / "web"


def read_web(filename: str) -> str:
    return (WEB_DIR / filename).read_text(encoding="utf-8")


class TestExhibitionHtmlContract(unittest.TestCase):
    def test_index_contains_exhibition_shell(self):
        html = read_web("index.html")
        required_snippets = [
            'class="app-layout exhibition-layout"',
            'class="hero-strip"',
            'id="hero-metrics"',
            'id="graph-legend"',
            'id="graph-callout"',
            'class="input-label"',
            'class="input-helper"',
        ]
        for snippet in required_snippets:
            self.assertIn(snippet, html, f"Missing HTML snippet: {snippet}")

    def test_index_keeps_six_navigation_tabs(self):
        html = read_web("index.html")
        tabs = re.findall(r'data-page="(overview|stations|booking|routing|mst|analysis)"', html)
        self.assertEqual(len(tabs), 6, "Expected exactly six primary navigation tabs")


class TestExhibitionCssContract(unittest.TestCase):
    def test_styles_define_required_tokens_and_motion(self):
        css = read_web("styles.css")
        required_tokens = [
            "--bg-app:",
            "--bg-nav:",
            "--accent-primary:",
            "--accent-support:",
            "--success:",
            "--danger:",
        ]
        for token in required_tokens:
            self.assertIn(token, css, f"Missing CSS token: {token}")

        self.assertIn("@keyframes heroReveal", css)
        self.assertIn("@keyframes cardReveal", css)
        self.assertIn(".state-panel", css)
        self.assertIn("@media (max-width: 1024px)", css)
        self.assertIn("@media (max-width: 768px)", css)


class TestExhibitionJsContract(unittest.TestCase):
    def test_app_contains_state_and_hero_helpers(self):
        js = read_web("app.js")
        required_symbols = [
            "const PAGE_META",
            "function renderStatePanel",
            "function setSectionState",
            "function updateHeroMetrics",
            "function setGraphCallout",
        ]
        for symbol in required_symbols:
            self.assertIn(symbol, js, f"Missing JS helper: {symbol}")


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run tests to verify they fail before implementation**

Run:

```bash
python3 -m unittest tests/test_ui_contract.py -v
```

Expected:

```text
FAIL: Missing HTML snippet: class="app-layout exhibition-layout"
FAIL: Missing CSS token: --bg-app:
FAIL: Missing JS helper: const PAGE_META
```

- [ ] **Step 3: Commit the failing test baseline**

```bash
git add tests/test_ui_contract.py
git commit -m "test: add frontend exhibition dashboard contract tests"
```

---

### Task 2: Implement Exhibition HTML Structure

**Files:**
- Modify: `web/index.html`
- Test: `tests/test_ui_contract.py`

- [ ] **Step 1: Run only HTML contract tests to confirm failure**

Run:

```bash
python3 -m unittest tests.test_ui_contract.TestExhibitionHtmlContract -v
```

Expected:

```text
FAIL: Missing HTML snippet: class="hero-strip"
```

- [ ] **Step 2: Add exhibition layout class and hero strip shell**

Update the top-level layout and top bar section to this structure:

```html
<div class="app-layout exhibition-layout">
  <aside class="sidebar">
    <!-- existing sidebar content stays, with current 6 data-page buttons -->
  </aside>

  <main class="main-content">
    <section class="hero-strip">
      <div class="hero-copy">
        <p class="hero-kicker">Railway Network DSA System</p>
        <h1 class="hero-title" id="page-title">Overview</h1>
        <p class="hero-subtitle" id="page-subtitle">Dashboard summary of your railway network</p>
      </div>
      <div class="hero-metrics" id="hero-metrics">
        <div class="hero-metric"><span class="hero-metric-label">Mode</span><span class="hero-metric-value" id="hero-mode">Overview</span></div>
        <div class="hero-metric"><span class="hero-metric-label">Stations</span><span class="hero-metric-value" id="hero-stations">-</span></div>
        <div class="hero-metric"><span class="hero-metric-label">Routes</span><span class="hero-metric-value" id="hero-routes">-</span></div>
      </div>
    </section>
```

- [ ] **Step 3: Add graph legend and algorithm callout container on Overview**

Insert inside the Overview graph card header area:

```html
<div class="graph-legend" id="graph-legend">
  <span><i class="legend-dot base"></i> Normal route</span>
  <span><i class="legend-dot dijkstra"></i> Dijkstra path</span>
  <span><i class="legend-dot mst"></i> Prim's MST</span>
</div>
<div class="graph-callout" id="graph-callout">No active algorithm highlight</div>
```

- [ ] **Step 4: Replace placeholder-only form rows with explicit labels/helpers**

For each form field pattern, apply this structure:

```html
<div class="form-group">
  <label class="input-label" for="station-id">Station ID</label>
  <input id="station-id" type="text" name="id" placeholder="e.g. S1" required>
  <p class="input-helper">Use uppercase IDs like S1, S2, S3.</p>
</div>
```

Apply equivalent label/helper blocks to Station, Route, Booking, Route Finder, MST, and Fare Trend forms.

- [ ] **Step 5: Run HTML contract tests to verify pass**

Run:

```bash
python3 -m unittest tests.test_ui_contract.TestExhibitionHtmlContract -v
```

Expected:

```text
OK
```

- [ ] **Step 6: Commit HTML structure changes**

```bash
git add web/index.html
git commit -m "feat: add exhibition dashboard html structure and form labeling"
```

---

### Task 3: Implement Exhibition Design System in CSS

**Files:**
- Modify: `web/styles.css`
- Test: `tests/test_ui_contract.py`

- [ ] **Step 1: Run CSS contract test to confirm failure**

Run:

```bash
python3 -m unittest tests.test_ui_contract.TestExhibitionCssContract -v
```

Expected:

```text
FAIL: Missing CSS token: --bg-app:
```

- [ ] **Step 2: Add tokenized visual system at top of styles file**

Add this root block and base body treatment:

```css
:root {
  --bg-app: #ecf1f6;
  --bg-surface: #ffffff;
  --bg-surface-2: #f7fafc;
  --bg-nav: #0b1324;
  --bg-nav-active: #1d2a44;
  --text-primary: #0f172a;
  --text-secondary: #475569;
  --text-muted: #64748b;
  --accent-primary: #0ea5e9;
  --accent-primary-strong: #0284c7;
  --accent-support: #f59e0b;
  --success: #16a34a;
  --danger: #dc2626;
  --border: #dbe5f0;
}

body {
  font-family: "Manrope", "Sora", "Segoe UI", sans-serif;
  color: var(--text-primary);
  background:
    radial-gradient(1200px 700px at 10% 0%, rgba(14, 165, 233, 0.10), transparent 60%),
    radial-gradient(900px 600px at 90% 10%, rgba(245, 158, 11, 0.08), transparent 60%),
    var(--bg-app);
}
```

- [ ] **Step 3: Add hero strip, cards, state panel, and graph legend styles**

Add these required class blocks (keep naming exact):

```css
.hero-strip { animation: heroReveal 280ms ease; }
.hero-metrics { display: grid; grid-template-columns: repeat(3, minmax(90px, 1fr)); gap: 12px; }
.hero-metric { background: rgba(255, 255, 255, 0.75); border: 1px solid var(--border); border-radius: 12px; padding: 10px 12px; }

.graph-legend { display: flex; flex-wrap: wrap; gap: 12px; font-size: 12px; color: var(--text-secondary); }
.graph-callout { margin-top: 10px; padding: 8px 12px; border-radius: 10px; background: var(--bg-surface-2); border: 1px solid var(--border); }

.state-panel { border-radius: 12px; padding: 14px; border: 1px dashed var(--border); background: var(--bg-surface-2); }
.state-panel.error { border-color: rgba(220, 38, 38, 0.4); color: var(--danger); }
.state-panel.loading { color: var(--text-secondary); }

@keyframes heroReveal {
  from { opacity: 0; transform: translateY(12px); }
  to { opacity: 1; transform: translateY(0); }
}

@keyframes cardReveal {
  from { opacity: 0; transform: translateY(8px) scale(0.98); }
  to { opacity: 1; transform: translateY(0) scale(1); }
}
```

- [ ] **Step 4: Add responsive breakpoints required by spec**

Add both breakpoints:

```css
@media (max-width: 1024px) {
  .stats-grid { grid-template-columns: repeat(2, minmax(0, 1fr)); }
  .card-row { flex-direction: column; }
}

@media (max-width: 768px) {
  .app-layout { flex-direction: column; }
  .sidebar { width: 100%; height: auto; position: static; }
  .main-content { max-width: 100%; padding: 16px; }
  .hero-metrics { grid-template-columns: 1fr; }
}
```

- [ ] **Step 5: Run CSS contract tests to verify pass**

Run:

```bash
python3 -m unittest tests.test_ui_contract.TestExhibitionCssContract -v
```

Expected:

```text
OK
```

- [ ] **Step 6: Commit CSS redesign**

```bash
git add web/styles.css
git commit -m "feat: implement exhibition dashboard visual system"
```

---

### Task 4: Implement JS UI Helpers and State Handling

**Files:**
- Modify: `web/app.js`
- Test: `tests/test_ui_contract.py`

- [ ] **Step 1: Run JS contract test to confirm failure**

Run:

```bash
python3 -m unittest tests.test_ui_contract.TestExhibitionJsContract -v
```

Expected:

```text
FAIL: Missing JS helper: const PAGE_META
```

- [ ] **Step 2: Add page metadata map and hero metric updater**

Add near the top of `web/app.js`:

```javascript
const PAGE_META = {
  overview: { title: 'Overview', subtitle: 'Dashboard summary of your railway network', mode: 'Overview' },
  stations: { title: 'Stations & Routes', subtitle: 'Manage your railway infrastructure', mode: 'Data Ops' },
  booking: { title: 'Booking', subtitle: 'Book seats, manage waitlist, process cancellations', mode: 'Queue/Stack' },
  routing: { title: 'Route Finder', subtitle: "Find shortest or least-crowded paths using Dijkstra's algorithm", mode: 'Dijkstra' },
  mst: { title: 'MST Report', subtitle: "Build a minimum spanning tree using Prim's algorithm", mode: 'Prim' },
  analysis: { title: 'Analysis', subtitle: 'Reports, fare search, occupancy, and trend prediction', mode: 'Sorting/Search' }
};

function updateHeroMetrics(pageKey) {
  const meta = PAGE_META[pageKey] || PAGE_META.overview;
  const stations = Array.isArray(allStations) ? allStations.length : 0;
  const routes = Array.isArray(allRoutes) ? allRoutes.length : 0;
  document.getElementById('hero-mode').textContent = meta.mode;
  document.getElementById('hero-stations').textContent = String(stations);
  document.getElementById('hero-routes').textContent = String(routes);
}
```

- [ ] **Step 3: Add reusable section state renderer helpers**

Add these helpers:

```javascript
function renderStatePanel(kind, message) {
  return `<div class="state-panel ${kind}">${message}</div>`;
}

function setSectionState(targetId, kind, message) {
  const el = document.getElementById(targetId);
  if (!el) return;
  el.innerHTML = renderStatePanel(kind, message);
}
```

Use `setSectionState` before/after async actions for at least:
- `path-result`
- `mst-result`
- `profitability-result`
- `fare-range-result`
- `occupancy-map`
- `fare-trend-result`

- [ ] **Step 4: Add graph algorithm callout updater**

Add helper and invoke it where highlights are applied/reset:

```javascript
function setGraphCallout(kind, detail) {
  const callout = document.getElementById('graph-callout');
  if (!callout) return;
  if (kind === 'dijkstra') {
    callout.textContent = `Dijkstra path active: ${detail}`;
    return;
  }
  if (kind === 'mst') {
    callout.textContent = `Prim's MST active: ${detail}`;
    return;
  }
  callout.textContent = 'No active algorithm highlight';
}
```

Invocation rules:
- After route search success: `setGraphCallout('dijkstra', '<from> → <to>')`
- After MST success: `setGraphCallout('mst', '<totalKm> km total')`
- On graph reset/new overview load: `setGraphCallout('none', '')`

- [ ] **Step 5: Wire tab switching to PAGE_META and hero updates**

In the nav click handler, replace hardcoded title map usage with `PAGE_META`, then call:

```javascript
updateHeroMetrics(btn.dataset.page);
```

Also call `updateHeroMetrics('overview')` after initial `loadAllData()` success.

- [ ] **Step 6: Run JS contract tests to verify pass**

Run:

```bash
python3 -m unittest tests.test_ui_contract.TestExhibitionJsContract -v
```

Expected:

```text
OK
```

- [ ] **Step 7: Commit JS UI orchestration changes**

```bash
git add web/app.js
git commit -m "feat: add exhibition hero metrics and state-driven ui helpers"
```

---

### Task 5: Full Verification and Regression Safety

**Files:**
- Verify: `web/index.html`
- Verify: `web/styles.css`
- Verify: `web/app.js`
- Verify: `tests/test_ui_contract.py`

- [ ] **Step 1: Run full frontend contract test suite**

Run:

```bash
python3 -m unittest tests/test_ui_contract.py -v
```

Expected:

```text
Ran 4 tests in <time>
OK
```

- [ ] **Step 2: Build backend to confirm no compile regressions**

Run:

```bash
cmake --build build
```

Expected:

```text
[100%] Built target railway_app
```

- [ ] **Step 3: Start app and run manual smoke checklist**

Run:

```bash
./run.sh
```

Manual checklist at `http://localhost:8080`:

- Sidebar switches all 6 pages
- Hero metrics update on navigation
- Overview graph renders + legend + callout text
- Dijkstra search highlights path and updates callout
- MST build highlights edges and updates callout
- Analysis modules show loading/success panels
- Mobile/tablet breakpoints visually hold layout

- [ ] **Step 4: Run backend test suite**

Run:

```bash
ctest --test-dir build -V
```

Expected:

```text
100% tests passed, 0 tests failed out of 6
```

- [ ] **Step 5: Commit final integrated frontend upgrade**

```bash
git add web/index.html web/styles.css web/app.js tests/test_ui_contract.py
git commit -m "feat: deliver exhibition dashboard uiux upgrade with frontend contracts"
```

---

## Spec Coverage Check

- Objective and scope (frontend-only, no backend changes): Covered in Tasks 2-5.
- Visual system (new palette, typography, spacing, elevation): Covered in Task 3.
- Layout redesign (sidebar polish, hero strip, overview center-stage): Covered in Task 2 and Task 3.
- Component redesign (forms/tables/cards/analysis modules): Covered in Tasks 2-4.
- Motion system (moderate cinematic): Covered in Task 3.
- Data/API compatibility: Preserved in Task 4 (JS updates around existing API calls).
- Error/empty/loading states: Covered in Task 4.
- Responsive behavior: Covered in Task 3.
- Verification plan and regressions: Covered in Task 5.

## Placeholder Scan

- No unresolved placeholder markers are present.
- All commands, symbols, and files referenced in tasks are explicit.

## Type and Naming Consistency

- JS helper names are consistent across tasks:
  - `PAGE_META`
  - `renderStatePanel`
  - `setSectionState`
  - `updateHeroMetrics`
  - `setGraphCallout`
- HTML IDs referenced by JS are consistent:
  - `hero-mode`, `hero-stations`, `hero-routes`, `graph-callout`
