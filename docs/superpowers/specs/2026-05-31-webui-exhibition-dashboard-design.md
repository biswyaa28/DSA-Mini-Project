# Railway Network DSA System - Exhibition Dashboard UI/UX Redesign

Date: 2026-05-31

## 1) Objective

Redesign the existing web frontend to deliver a presentation-grade "Exhibition Dashboard" experience for professor demos, while preserving all existing backend behavior and API contracts.

This redesign prioritizes:

- Stronger visual wow
- Clear, fast task flow across all 6 tabs
- Better readability on laptop/projector screens
- Cohesive motion and feedback behavior

## 2) Scope and Constraints

### In scope

- Frontend-only redesign in:
  - `web/index.html`
  - `web/styles.css`
  - `web/app.js`
- Layout refresh, component redesign, motion polish
- Better loading/empty/error/success states
- Graph visual and interaction refinement

### Out of scope

- Backend code, API routes, payload shapes, or business logic changes
- Database/schema/data model changes
- New third-party frameworks

### Hard constraints

- Keep all current endpoints and request/response formats unchanged
- Preserve all 6 existing feature areas:
  - Overview
  - Stations & Routes
  - Booking
  - Route Finder
  - MST Report
  - Analysis

## 3) Product and UX Direction (Approved)

- Visual style: **Polished academic**
- Experience type: **Exhibition Dashboard**
- Primary UX emphasis: **Stronger visual wow**
- Palette direction: **Open to new palette** (not locked to previous deep blue/teal)
- Motion level: **Moderate cinematic**

## 4) Information Architecture

The information architecture remains unchanged to avoid behavioral risk:

- Left navigation with 6 tabs (same order and semantics)
- Main content switches by tab
- Top context bar updates title/subtitle by active tab
- Overview remains the landing tab

## 5) Visual System Specification

### 5.1 Color tokens

Introduce CSS variables for a new palette with strong contrast and projector-friendly clarity.

- `--bg-app`: `#ecf1f6`
- `--bg-surface`: `#ffffff`
- `--bg-surface-2`: `#f7fafc`
- `--bg-nav`: `#0b1324`
- `--bg-nav-active`: `#1d2a44`
- `--text-primary`: `#0f172a`
- `--text-secondary`: `#475569`
- `--text-muted`: `#64748b`
- `--accent-primary`: `#0ea5e9`
- `--accent-primary-strong`: `#0284c7`
- `--accent-support`: `#f59e0b`
- `--success`: `#16a34a`
- `--danger`: `#dc2626`
- `--border`: `#dbe5f0`

### 5.2 Typography

- Primary UI font stack: `"Manrope", "Sora", "Segoe UI", sans-serif`
- Numeric emphasis: same family with heavier weight for KPI consistency
- Clear hierarchy:
  - Hero/title: 28-32px
  - Section title: 18-22px
  - Card title: 14-16px
  - Body/table text: 13-14px
  - Meta/captions: 11-12px

### 5.3 Shape, spacing, elevation

- Radius scale: 8, 12, 14px
- Spacing scale: 8, 12, 16, 20, 28px
- Elevation:
  - Base cards: soft shadow
  - Hover cards: elevated shadow + subtle translateY
  - Overlays/modals: stronger depth layer

### 5.4 Background treatment

- Replace flat background with layered gradients and subtle radial highlights
- Keep decorative layers low-opacity to avoid distracting from data

## 6) Layout and Section Redesign

### 6.1 Sidebar (navigation rail)

- Keep dark rail structure but improve visual hierarchy
- Stronger active indicator with glow edge + background contrast
- Better icon/text alignment for rapid scan
- DSA badges become compact method tags aligned to right edge

### 6.2 Top hero strip

- Convert current top bar into a more expressive hero strip:
  - page title/subtitle
  - connectivity/status indicator
  - quick context micro-metrics (where appropriate)

### 6.3 Overview as center-stage screen

- Network graph remains central, enlarged prominence
- KPI cards become clean insight tiles around graph
- Add graph legend/callout area for active algorithm highlights

## 7) Component Redesign Plan

### 7.1 Cards and panels

- Standardized card anatomy:
  - Header (title + chips/actions)
  - Content area
  - Footer/meta (optional)
- Improve visual consistency across all pages

### 7.2 Forms

- Upgrade from placeholder-only inputs to explicit labels + helper text where needed
- Consistent focus rings and validation styles
- Clear primary vs secondary actions

### 7.3 Tables

- Improve readability with:
  - stronger header contrast
  - zebra or subtle row alternation
  - hover feedback
  - status pills
  - aligned numeric columns
- Preserve existing table data semantics

### 7.4 Graph controls

- Keep Expand and Reset actions
- Add clearer affordances and visual grouping
- Maintain fullscreen modal behavior

### 7.5 Analysis modules

- Unify profitability, fare range, occupancy, and fare trend visuals
- Standard module shell for title, controls, and results
- Better labels/units for at-a-glance interpretation

## 8) Motion and Interaction System

Motion must feel cinematic but controlled.

- Page transitions: 220-320ms fade/translate
- Card entry: staggered reveal on section load
- Hover interactions: subtle lift and shadow
- Button press: light compression effect
- Chart/progress fills: animated growth on render
- Graph highlight animations:
  - Dijkstra path edges animate to primary-blue emphasis
  - MST edges animate to green emphasis

Guardrails:

- No long or distracting animations
- No heavy parallax effects
- Motion must not block task completion

## 9) Data Flow and API Compatibility

### 9.1 API behavior

- All existing API calls remain unchanged in route, params, and payload shape
- UI layer only changes rendering and interaction handling

### 9.2 Frontend refresh behavior

- Keep partial-region refreshes after actions (forms/tables/graph sections)
- Avoid unnecessary full-page redraws to preserve responsiveness

### 9.3 Graph state integration

- Preserve existing route-to-edge mapping logic
- Continue highlighting returned path/MST edges on result render
- Keep default edge reset behavior before new highlight application

## 10) Error, Empty, and Loading States

Every major module must present explicit state visuals:

- Loading: skeleton or lightweight loading indicator
- Empty: instructional message with next action
- Error: clear message with retry affordance where possible
- Success: concise positive confirmation (toast/status chip)

This prevents "blank panel" moments during live demos.

## 11) Accessibility and Readability

- Improve contrast against all surfaces
- Ensure visible keyboard focus outlines
- Keep target sizes comfortable for click/tap
- Preserve semantic headings and readable text sizes
- Optimize visual clarity for projector display

## 12) Responsive Behavior

- Desktop: full sidebar and multi-column cards
- Tablet: reduce density, collapse multi-column forms/tables gracefully
- Mobile: stacked layout and horizontally scrollable tables where needed
- Keep all six tabs fully reachable on smaller screens

## 13) Verification Plan

### 13.1 Functional checks

- Tab switching works for all 6 sections
- Overview stats and graph render
- Station/route CRUD forms still function
- Booking/cancellation/waitlist flows still function
- Dijkstra path search still works and highlights graph edges
- MST generation still works and highlights graph edges
- Analysis modules still return and render data

### 13.2 Visual/UX checks

- New palette/tokens applied consistently
- Motion feels smooth and non-intrusive
- Readability is strong at typical projector zoom levels
- Graph modal expand/close/reset remains stable

### 13.3 Regression checks

- Backend integration unchanged
- Existing backend tests continue to pass (`ctest --test-dir build -V`)

## 14) Deliverables

- Updated `web/index.html`
- Updated `web/styles.css`
- Updated `web/app.js`
- No backend file modifications

## 15) Acceptance Criteria

The redesign is complete when:

1. All approved visual direction choices are reflected in the UI
2. All existing features remain functionally intact without backend changes
3. Dijkstra and MST graph highlighting remains reliable
4. The interface looks presentation-ready and cohesive across tabs
5. Responsive behavior remains usable on desktop/tablet/mobile
