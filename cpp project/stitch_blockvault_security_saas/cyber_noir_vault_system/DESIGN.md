---
name: Cyber Noir Vault System
colors:
  surface: '#0e1323'
  surface-dim: '#0e1323'
  surface-bright: '#34394a'
  surface-container-lowest: '#080d1d'
  surface-container-low: '#161b2b'
  surface-container: '#1a1f30'
  surface-container-high: '#25293a'
  surface-container-highest: '#2f3446'
  on-surface: '#dee1f9'
  on-surface-variant: '#c9c4d8'
  inverse-surface: '#dee1f9'
  inverse-on-surface: '#2b3041'
  outline: '#938ea1'
  outline-variant: '#484555'
  surface-tint: '#cabeff'
  primary: '#cabeff'
  on-primary: '#31009a'
  primary-container: '#947dff'
  on-primary-container: '#2a0088'
  inverse-primary: '#603ce2'
  secondary: '#5de6ff'
  on-secondary: '#00363e'
  secondary-container: '#00cbe6'
  on-secondary-container: '#00515d'
  tertiary: '#ffb780'
  on-tertiary: '#4e2600'
  tertiary-container: '#d7791f'
  on-tertiary-container: '#442000'
  error: '#ffb4ab'
  on-error: '#690005'
  error-container: '#93000a'
  on-error-container: '#ffdad6'
  primary-fixed: '#e6deff'
  primary-fixed-dim: '#cabeff'
  on-primary-fixed: '#1c0062'
  on-primary-fixed-variant: '#4816cb'
  secondary-fixed: '#a2eeff'
  secondary-fixed-dim: '#2fd9f4'
  on-secondary-fixed: '#001f25'
  on-secondary-fixed-variant: '#004e5a'
  tertiary-fixed: '#ffdcc4'
  tertiary-fixed-dim: '#ffb780'
  on-tertiary-fixed: '#2f1400'
  on-tertiary-fixed-variant: '#6f3800'
  background: '#0e1323'
  on-background: '#dee1f9'
  surface-variant: '#2f3446'
typography:
  display-hero:
    fontFamily: Inter
    fontSize: 40px
    fontWeight: '700'
    lineHeight: 48px
    letterSpacing: -0.02em
  display-hero-mobile:
    fontFamily: Inter
    fontSize: 30px
    fontWeight: '700'
    lineHeight: 38px
    letterSpacing: -0.015em
  headline-lg:
    fontFamily: Inter
    fontSize: 28px
    fontWeight: '600'
    lineHeight: 36px
    letterSpacing: -0.015em
  headline-md:
    fontFamily: Inter
    fontSize: 22px
    fontWeight: '600'
    lineHeight: 28px
    letterSpacing: -0.01em
  headline-sm:
    fontFamily: Inter
    fontSize: 18px
    fontWeight: '600'
    lineHeight: 24px
    letterSpacing: -0.005em
  body-lg:
    fontFamily: Inter
    fontSize: 16px
    fontWeight: '400'
    lineHeight: 24px
  body-md:
    fontFamily: Inter
    fontSize: 14px
    fontWeight: '400'
    lineHeight: 20px
  body-sm:
    fontFamily: Inter
    fontSize: 12px
    fontWeight: '400'
    lineHeight: 16px
  mono-data:
    fontFamily: JetBrains Mono
    fontSize: 13px
    fontWeight: '400'
    lineHeight: 18px
    letterSpacing: 0.02em
  mono-key:
    fontFamily: JetBrains Mono
    fontSize: 11px
    fontWeight: '500'
    lineHeight: 14px
    letterSpacing: 0.05em
  badge-label:
    fontFamily: JetBrains Mono
    fontSize: 11px
    fontWeight: '600'
    lineHeight: 14px
    letterSpacing: 0.04em
rounded:
  sm: 0.125rem
  DEFAULT: 0.25rem
  md: 0.375rem
  lg: 0.5rem
  xl: 0.75rem
  full: 9999px
spacing:
  gutter: 1.5rem
  gutter-mobile: 0.75rem
  margin: 2rem
  margin-mobile: 1rem
  space-xs: 0.25rem
  space-sm: 0.5rem
  space-md: 1rem
  space-lg: 1.5rem
  space-xl: 2.5rem
---

## Brand & Style

This design system expresses a **Premium Cyber Noir** aesthetic engineered specifically for high-trust credential orchestration and zero-knowledge environments. It marries the disciplined, utilitarian architecture of cryptographic tooling with the polished restraint of contemporary enterprise interfaces.

### Core Philosophy & Visual Voice
- **Cryptographic Authority:** Unforgivingly precise, sharp, and structured. Layouts signal airtight security through deliberate spatial economy, strict alignments, and predictable visual hierarchy.
- **Obsidian Restraint:** Rejecting hyper-saturated neon clichés in favor of deep atmospheric midnight surfaces (`#0B1020`), precision-cut hairline borders (`#243047`), and focused emissive strokes. Glows and luminous indicators serve strictly diagnostic and telemetry functions.
- **Tactile High-Density:** Balances quick-scanning data density for enterprise security engineers with frictionless, calming utility for daily key management. Micro-interactions are brief, sharp (150ms ease-out transitions), and weighted to instill tactile finality.

## Colors

The palette leverages a dark-first hierarchy where luminosity strictly dictates elevation, interaction readiness, and operational status.

### Color Tokens & System Roles

- **Base Canvas (`#0B1020`):** The foundational obsidian floor. Deep, low-noise backdrop that anchors all overlays and views.
- **Surface / Card (`#111A2E`):** Default card, panel, and navigation structural container.
- **Surface Elevated / Active (`#19253F`):** Hover states, open popovers, drawer canvases, and focused table rows.
- **Primary Accent (`#7C5CFF`):** Signature cryptographic purple. Reserved for primary calls to action, committed actions, active states, and identity markers. Hover transitions to `#6945F8`.
- **Secondary / Telemetry (`#22D3EE`):** Crisp cyan telemetry indicator. Applied to network activity, cryptographic hashes, copy verification signals, and secondary focal accents.
- **Border / Hairline Stroke (`#243047`):** The foundational 1px framing unit. Creates clean structural boundaries across dark planes without heavy contrast jarring.
- **Text & Foreground:**
  - `High Contrast Text`: `#F8FAFC` (Titles, keys, table values, interactive labels)
  - `Muted Secondary Text`: `#94A3B8` (Metadata, timestamps, helper descriptions, structural keys)
  - `Subtle / Disabled Text`: `#475569` (Placeholder values, disabled buttons)
- **Status & Integrity Indicators:**
  - `Success`: `#22C55E` (Strong passwords, zero-breach audit, sync confirmed)
  - `Warning`: `#F59E0B` (Expiring keys, shared passwords, at-risk entropy)
  - `Danger`: `#EF4444` (Breached credentials, weak keys, destructive revocation)

## Typography

The typographic hierarchy pairs the structured grotesque clarity of **Inter** for narrative, metadata, and controls with the mechanical exactness of **JetBrains Mono** for all operational payloads, security status indicators, keyboard shortcuts, and cryptographic values.

### Usage Standards
- **Numerical Alignment & Tabular Figures:** All tabular lists, password lengths, latency numbers, and audit counters render using OpenType tabular figures (`tnum`).
- **Telemetry & Payload Treatment:** Secret fields, token digests, recovery phrases, IP logs, and shortcut hints strictly use `JetBrains Mono`. Letters must never blur into ambiguous glyphs (`0` vs `O`, `1` vs `l`).
- **Caps & Spacing:** Micro-labels, cipher indicators, and status badges are styled in uppercase with tracked spacing (`letter-spacing: 0.04em`) to maintain legibility at 11px.

## Layout & Spacing

The spatial engine is built upon a dense 4px base increment mapped to an 8px structural layout model. Space is treated with architectural rigor to maximize visual continuity across multi-column vault workflows.

### Grid Architecture & Reflow Rules
- **Desktop (1200px+):** 12-column fluid grid inside a 1440px max-width boundary. Side navigation (260px fixed width) docks to the left, followed by a secondary vault explorer pane (320px fixed or collapsable), and a flexible detail pane (spans remaining 8 columns with 24px gutters).
- **Tablet (768px – 1199px):** Split-view architecture transforms into an 8-column layout with 16px gutters. Navigation collapses into an icon rail or command-bar-driven modal pattern. Detail panes slide in from the right edge as overlays.
- **Mobile (< 768px):** Single-column stacked view with full-bleed canvas and 16px margins. Drill-down screens replace multi-pane layouts. Bottom sheets handle secret inspections and cryptographic key operations.

## Elevation & Depth

This design system avoids heavy drop shadows in favor of **Tonal Layering**, **Crisp Hairline Boundaries**, and **Emissive Edge Lighting**. 

### Elevation Tiers
- **Layer 0 (Canvas Base - `#0B1020`):** Recessed backdrop plane.
- **Layer 1 (Card & List Surface - `#111A2E`):** Encased with `1px solid #243047`. No shadow. Depth is created purely through color contrast against the canvas.
- **Layer 2 (Elevated & Hover - `#19253F`):** Floating items, dropdown menus, context popovers. Outlined with `1px solid #334466` and backed by a tight, dark ambient shadow: `0 8px 24px -4px rgba(3, 7, 18, 0.6)`.
- **Layer 3 (Modal Windows & Command Palette):** Overlaid with a backdrop blur (`backdrop-filter: blur(12px) bg-opacity: 75%`). Bounded by `1px solid #3A4D73`, casting `0 20px 40px -8px rgba(0, 0, 0, 0.8)`.

### Emissive Accents
Active inputs, selected vault credentials, and focused triggers receive a subtle interior glow:
- Primary Focus: `box-shadow: 0 0 0 1px #7C5CFF, 0 0 16px -2px rgba(124, 92, 255, 0.35)`
- Telemetry Alert / Warning: `box-shadow: 0 0 0 1px #22D3EE, 0 0 12px -2px rgba(34, 211, 238, 0.25)`

## Shapes

The geometric personality adheres to **Soft Precision (Level 1)**. Geometry is disciplined, tight, and engineered. Overly rounded or playful geometries are strictly omitted to maintain military-grade trustworthiness.

### Corner Radius Tokens
- **Micro Elements (Badges, Tooltips, Checkboxes, Switches):** `4px` (`rounded-xs` / `0.25rem`)
- **Interactive Controls (Buttons, Text Inputs, Segmented Controls):** `6px` (`rounded-sm` / `0.375rem`)
- **Structural Modules (Cards, Modals, Vault Containers, Dropdowns):** `8px` (`rounded-lg` / `0.5rem`)
- **Pill Badges (Health Indicators, Telemetry Tags):** Fully rounded pill geometry (`rounded-full`) is reserved solely for inline status tokens to distinguish state chips from interactive rectangular controls.

## Components

### 1. Buttons
- **Primary:** Background `#7C5CFF`, text `#F8FAFC`, font weight `500`. Hover state triggers background `#6945F8` with a micro-glow `0 0 12px rgba(124, 92, 255, 0.4)`. Active state shifts to `#5B34E6`.
- **Secondary (Ghost Outline):** Background transparent, 1px solid border `#243047`, text `#F8FAFC`. Hover state sets background to `#19253F` and border to `#334466`.
- **Danger:** Background transparent, 1px solid border `#EF4444`, text `#EF4444`. Hover state transitions to background `rgba(239, 68, 68, 0.12)` with border `#EF4444`. Solid danger variant uses background `#EF4444` with text `#F8FAFC` for confirmed permanent actions.

### 2. Form Inputs & Secret Masking
- **Input Architecture:** Height `40px`, background `#0B1020`, 1px border `#243047`, text `#F8FAFC`, placeholder `#475569`. Corner radius `6px`.
- **Focus State:** 1px border `#7C5CFF` coupled with `box-shadow: 0 0 0 2px rgba(124, 92, 255, 0.2)`.
- **Masked Password Field:** Secrets render with custom bullet glyphs (`••••••••••••`) using `JetBrains Mono` for uniform width. Right accessory contains a quick-reveal button (eye toggle) and a single-click copy button with cyan `#22D3EE` confirmation tooltip feedback.

### 3. Command Bar (Omnisearch - Cmd+K)
- Floating dialog locked to the upper-third of the screen. Max width `640px`.
- Background `#111A2E`, 1px perimeter border `#334466`, blurred backdrop layer (`#0B1020` at 70% opacity with 16px blur).
- Top search input features an inline keyboard badge (`Cmd+K` / `Ctrl+K`) rendered in `JetBrains Mono` (`#94A3B8`, background `#19253F`, border `#243047`). Instant down-arrow navigation with `#19253F` item selection indicator and cyan `#22D3EE` trailing action labels.

### 4. Security Health Status Pills
- Pill-shaped tags (`height: 22px`, `padding: 2px 10px`, `font: JetBrains Mono 11px uppercase`).
- **Strong:** Border `rgba(34, 197, 94, 0.3)`, background `rgba(34, 197, 94, 0.1)`, text `#22C55E`. Left dot indicator: solid green pulse dot (`6px`).
- **Weak:** Border `rgba(239, 68, 68, 0.3)`, background `rgba(239, 68, 68, 0.1)`, text `#EF4444`.
- **Reused / Vulnerable:** Border `rgba(245, 158, 11, 0.3)`, background `rgba(245, 158, 11, 0.1)`, text `#F59E0B`.

### 5. Cards & Item Lists
- Vault rows utilize alternating or hairline-separated tiers (`1px solid #243047`).
- Hovering an item shifts surface color from `#111A2E` to `#19253F` seamlessly across a 150ms ease curve, revealing contextual actions (Copy Username, Copy Password, Launch URL, More Options).
- Favicons and entity logos sit within a 32x32px squircle frame with background `#0B1020` and 1px border `#243047`.

### 6. Checkboxes & Switches
- **Checkbox:** `16x16px`, corner radius `4px`, background `#0B1020`, border `1px solid #243047`. Checked state fills with `#7C5CFF`, border `#7C5CFF`, checkmark rendered in white (`#F8FAFC`).
- **Toggle Switch:** Capsule track `36px x 20px`, background `#19253F`, border `1px solid #243047`. Thumb diameter `14px`, white `#F8FAFC`. When active, track fills with `#7C5CFF` and thumb shifts with clean linear snap.