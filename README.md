# Dechinus WM
![Captain Dechinus](assets/captain-dechinus.svg)
> **Stay Sharp.** The minimalist sea urchin of window managers.

## The Dechinus Identity: A Specialized Organism
Dechinus is designed like a sea urchin: a compact, specialized organism with sharp "spines" (reparenting frames) that define its space. It is a **Dynamic Fusion** window manager that provides the structured protection of a desktop environment with the raw speed of a tiling engine.

In an ecosystem of bloated desktops, Dechinus remains small and sharp—providing a core engine of ~1,900 lines of C that manages windows with surgical precision while respecting your choice of external tools.

## Core Philosophy: The Sharp Edge
*   **The Spines (Reparenting):** Dechinus provides native titlebars and frames. These "spines" give your windows handles for mouse interaction, allowing you to grab, move, and manage windows without losing the minimalist aesthetic.
*   **Sharp Tiling:** A mathematical tiling stack that cuts through workspace clutter.
*   **Fluid Fusion:** Seamlessly transition from rigid, spine-hidden tiling to free-flowing floating modes where the titlebars emerge for precise control.
*   **Urchin Synergy:** Dechinus does not try to be a panel or a system tray. It is designed to co-exist in perfect synergy with third-party tools like **Polybar**, **Lemonbar**, or **Tint2**, respecting the "Work Area" defined by your chosen bar.
*   **Hybrid Control:** Optimized for the "Keyboard-First, Mouse-Friendly" user. Use the keyboard to snap windows into place, or use the mouse to interact with the titlebar widgets.

## Features

*   **XDG Base Directory Compliant** — Configuration stays organized in `~/.config/dewm/`.
*   **The 2K Flex** — A complete, reparenting engine with multi-monitor support (XRandR) in under 2,000 lines of code.
*   **Marine-Grade Performance** — High-performance event handling with minimal CPU and memory overhead.
*   **Smart Deployment** — An intelligent build system that detects the user and deploys `dewmrc` to the home directory automatically during `make install`.
*   **Xresources Configuration** — Change gaps, colors, titlebars, and layouts instantly via the `dewmrc` file without recompiling.
*   **Stabilized DNA** — Features a color fallback system and refactored Atom handling to ensure your session remains stable even if your configuration has errors.

## Installation

### Dependencies
*   `X11` (libX11)
*   `Xft` (for anti-aliased text)
*   `pkg-config`
*   `libxrandr` (for multi-monitor support)

### Build & Install
```bash
git clone https://github.com/dpTech-front/dechinus.git
cd dechinus
make
sudo make install
```
*The installer automatically deploys the default configuration to `~/.config/dewm/` for the original user.*

## Configuration
Dechinus is configured via **Xresources** syntax in `~/.config/dewm/dewmrc`.

### Layout Symbols
*   `i` — **Iconify** (Floating/Manual)
*   `t` — **Tile** (Classic Master/Stack)
*   `b` — **BStack** (Bottom Stack)
*   `m` — **Monocle** (Full Screen Focus)
*   `f` — **Floating** (Freeform Overlap)

## Optimizations & Performance
Dechinus is a sharpened version of its core logic:
*   **Code Purge:** All redundant logic and legacy macros have been stripped away for a cleaner, faster execution path.
*   **Refined Event Handling:** Replaced global monitor lookups with explicit pointer passing, significantly speeding up window management on multi-screen setups.
*   **Memory Integrity:** Refactored internal string handling to ensure long-term stability and zero compiler warnings.

## Contributing
Help us sharpen the spines.
1. Fork the repo.
2. Keep it minimal (< 2k lines).
3. Submit a PR.

## License
**MIT License** — See [LICENSE](./LICENSE)
