# Overview Graph Hero Stage Refresh

Date: 2026-06-01

## 1) Objective

Improve the Overview section graph so it feels presentation-grade, with a clean map-like canvas, bold edge emphasis, and cinematic interaction. This must address issues in layout balance, node/edge styling, density, and interaction feel while keeping all backend behavior intact.

## 2) Scope and Constraints

### In scope
- Frontend-only changes in:
  - `web/index.html`
  - `web/styles.css`
  - `web/app.js`
- Overview layout re-balance around the graph
- Graph canvas styling, node/edge styling, labels, legend/callout
- Vis-network configuration changes for physics and hover behavior

### Out of scope
- Backend code, API routes, payloads, or logic
- Data model changes
- New frameworks or graph libraries

### Hard constraints
- Existing endpoints and response formats must remain unchanged
- Graph must still render in Overview and fullscreen modal
- Existing controls (Expand, Reset) remain available

## 3) Approved Direction

### 3.1 Layout and balance
- Use the graph as the primary visual stage (1.5x to 2x the visual mass of surrounding KPI tiles).
- KPI tiles become slim insight strips to reduce competition for attention.
- Graph card height increases relative to current layout; the graph canvas should be the single most prominent element in the Overview page.

### 3.2 Canvas styling
- Light, high-contrast canvas: crisp white base with a subtle dot or grid texture at 2-3% opacity.
- Graph card includes a gentle inner border and soft shadow to frame the canvas without heavy chrome.
- Avoid dark panels; keep the graph background bright and clean.

### 3.3 Node and edge styling
- Minimal nodes, bold edges to create a clean map feel:
  - Nodes: smaller radius, low fill, crisp outline; avoid heavy glow.
  - Edges: thicker default strokes, higher contrast against the light canvas.
- Highlight styles:
  - Dijkstra path edges emphasize `--accent-primary` with a faint glow.
  - MST edges emphasize `--success` with a faint glow.
- Labels appear only on hover or selection to avoid density overload.

### 3.4 Interaction feel
- Motion is smooth and cinematic with slight inertia:
  - Heavier drag feel and slower settle time.
  - Avoid jitter or rapid snap-backs.
- Hover behavior:
  - Node hover reveals label and a subtle halo.
  - Selection locks label and brightens connected edges.

### 3.5 Legend and callout
- Legend and callout remain in the graph card but are compact:
  - Use pill-style chips aligned left under the header.
  - Keep vertical height low to preserve canvas space.
- Callout text is data-dense and short; fades in/out in 200-280ms.

## 4) Layout Details

- The graph card should dominate the Overview grid; KPI tiles adjust to a slimmer format and secondary visual weight.
- On large screens, place KPI strips adjacent to the graph (right side) or above it in a narrow band. The graph retains clear priority.
- On smaller screens, KPI strips stack above the graph; the graph stays full width with a tall canvas.

## 5) Interaction and Motion Details

- Vis-network physics tuned for a cinematic feel:
  - Higher inertia (lower damping), gentler stabilization.
- Edge highlight animation should remain subtle (no flashing).
- Labels only show on hover/selection; default view is clean and uncluttered.

## 6) Accessibility and Clarity

- Ensure labels are readable on light canvas with sufficient contrast.
- Keep hover-only labels keyboard accessible via focus where possible.
- Respect `prefers-reduced-motion` by reducing easing and stabilization duration.

## 7) Success Criteria

- Graph reads clearly as the hero visual element on the Overview page.
- Default view is clean, with minimal label clutter.
- Dijkstra and MST highlights are immediately distinguishable.
- Interaction feels smooth, not jittery.
- No backend behavior changes; all existing routes function as before.

## 8) Verification

- Manual: open Overview, confirm graph is primary visual element.
- Manual: hover a node to see labels; selection locks label visibility.
- Manual: run Dijkstra and MST and confirm highlight colors and callout behavior.
- Automated: existing UI contract tests still pass.
