# Railway Network DSA System — Web UI Redesign

Date: 2026-05-31

## Goal

Redesign the web frontend (HTML, CSS, JS only) to be a best-in-class UI/UX for presenting the DSA Mini Project to professors. Zero backend changes.

## Design Decisions (Approved)

- **Layout**: Dark sidebar + main content area (app-like feel)
- **Color scheme**: Deep Blue + Teal (navy #0f172a, blue #2563eb, teal #0891b2)
- **Graph**: Interactive network graph (vis.js) on the Overview page, expandable to fullscreen
- **Navigation**: Sidebar with DSA algorithm badges (Dijkstra, Prim's, Queue, Stack)
- **Visual style**: Rounded corners (8-12px), generous spacing, subtle shadows, smooth transitions

## Architecture

All changes are contained in three files:
- `web/index.html` — complete rewrite
- `web/styles.css` — complete rewrite
- `web/app.js` — rewrite keeping same API calls

## Layout Structure

```
┌──────────────────────────────────────────────────┐
│ [Dark Sidebar]          [Main Content Area]       │
│                        ┌─────────────────────────┐│
│ 🚂 RailDSA             │ Top Bar (title + pills) ││
│                        ├─────────────────────────┤│
│ 📊 Overview  (active)  │ Stat Cards Grid (4)      ││
│ 🚉 Stations & Routes   ├─────────────────────────┤│
│ 🎫 Booking             │ Network Graph (vis.js)   ││
│ 🛤️ Route Finder [Dijk] │                         ││
│ 🌲 MST Report  [Prim]  │                         ││
│ 📈 Analysis            │                         ││
│                        └─────────────────────────┘│
│ DSA Mini Project                                  │
└──────────────────────────────────────────────────┘
```

## Components

### Sidebar
- Dark navy (#0f172a) with rounded corners
- 6 navigation items with emoji icons
- Active item: light background (#1e293b), white text
- Inactive items: muted text (#94a3b8)
- DSA algorithm badges on Route Finder (Dijkstra) and MST (Prim's)
- Footer: "DSA Mini Project" text

### Top Bar
- White card with shadow
- Current page title + subtitle on left
- Status pills on right (network status, counts)

### Stat Cards (Overview)
- 4-card grid: Stations, Routes, Bookings, Revenue
- Each card: label, large number, subtle status text
- Color-coded: green for positive, neutral for others

### Network Graph (vis.js)
- Interactive force-directed graph rendered on a canvas
- Stations = colored nodes with labels
- Routes = edges with distance labels
- Initial zoom fits all nodes
- Colors: nodes match station ID colors, edges are teal
- Fullscreen toggle button expands graph to modal overlay
- Route highlight: when Dijkstra/MST results come back, highlight those edges

### Feature Tabs
Each tab has consistent styles:
- Forms: light gray backgrounds (#f8fafc), borders (#e2e8f0), blue buttons
- Tables: striped rows, compact but readable, status badges
- Cards: white, 8px border-radius, 1px border, subtle shadow
- DSA concept badges in section headers (e.g., "Algorithm: Dijkstra")

## Interactions & Animations

- **Sidebar**: smooth hover background transition (0.2s)
- **Tab switching**: fade transition (0.3s opacity)
- **Stat cards**: subtle scale-up on hover (1.02)
- **Buttons**: darken on hover, slight scale on click (0.98)
- **Toast notifications**: slide in from bottom, fade out
- **Graph**: force-directed physics animation from vis.js
- **Loading states**: spinner shown while API calls are in progress
- **Page load**: stat cards stagger in with fade-up animation

## Color Palette

| Role | Color | Hex |
|------|-------|-----|
| Sidebar bg | Dark navy | `#0f172a` |
| Sidebar active | Slate lighter | `#1e293b` |
| Sidebar text | Muted | `#94a3b8` |
| Primary | Blue | `#2563eb` |
| Primary hover | Darker blue | `#1d4ed8` |
| Accent | Teal | `#0891b2` |
| Background | Light gray | `#f1f5f9` |
| Card bg | White | `#ffffff` |
| Border | Light border | `#e2e8f0` |
| Text primary | Dark | `#0f172a` |
| Text secondary | Muted | `#64748b` |
| Success | Green | `#22c55e` |
| Warning | Amber | `#f59e0b` |
| Danger | Red | `#ef4444` |

## DSA Badges

Labels shown on relevant nav items and section headers:

| DSA Concept | Badge Color |
|-------------|-------------|
| Dijkstra | Purple (#7c3aed) |
| Prim's MST | Green (#059669) |
| Queue (FIFO) | Cyan (#0891b2) |
| Stack (LIFO) | Red (#ef4444) |
| Sorting | Amber (#d97706) |
| Binary Search | Blue (#2563eb) |

## Responsive Behavior

- Desktop: full sidebar + main content side by side
- Tablet: sidebar collapses to icon-only
- Mobile: sidebar hidden behind hamburger menu

## API Dependencies (Unchanged)

All existing API endpoints are used as-is. No new endpoints required.

## Files Changed

- `web/index.html` — complete rewrite
- `web/styles.css` — complete rewrite
- `web/app.js` — complete rewrite (API layer preserved, UI layer replaced)
