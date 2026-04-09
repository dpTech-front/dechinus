# Dechinus WM
![Captain Dechinus](assets/captain-dechinus.svg)
> **Stay Sharp. Stay Lean.** The minimalist sea urchin of window managers.

## The Identity: A Specialized Organism
**Dechinus (`dewm`)** is designed like its namesake sea urchin: a compact, efficient organism protected by sharp "spines" (reparenting frames). It is the next evolution of the minimalist philosophy—sharpening the heritage of its ancestors into a modern tool that masters the balance between tiling logic and floating elegance.

Dechinus is a **Self-Sustained Monolith**. It handles its own keyboard shortcuts internally for maximum speed and zero latency, but it exposes its "Command Spines" to the outside world via **`dewmc`**, allowing for ultimate Unix-style scriptability without the overhead of complex background daemons.

## The Ecosystem: The Triangle of Power
To keep Dechinus minimal and efficient, the project is organized into three distinct parts:

*   **`dewm` [The Manager]:** The core window manager binary. A high-performance engine (~1,900 SLOC) that handles tiling logic, reparenting frames, and input events.
*   **`dewmrc` [The DNA]:** The configuration file (`~/.config/dewm/dewmrc`). It defines your static settings like colors, layouts, and initial window rules.
*   **`dewmc` [The Controller]:** A lightweight CLI utility used to send real-time commands to the manager from shell scripts, panels, or the terminal.

## Core Philosophy: The Sharp Edge
*   **Dynamic Fusion:** Dechinus refuses to choose between tiling and floating.
    *   **Elegant Floating:** When a window is untiled, its "spines" emerge. It gains a beautiful, reparented frame with a titlebar and mouse-driven widgets (Close, Max, Min).
    *   **Chrome-less Tiling:** When tiled, the spines retract. Titlebars vanish to maximize workspace, leaving only a sharp, configurable border.
*   **Reparenting Minimalism:** Titlebars and frames are implemented with surgical precision. This isn't "bloat"—it is usability engineered into a footprint small enough to fit in your head.
*   **Panel Synergy (Unix Philosophy):** Dechinus focuses exclusively on window management. With full **_NET_WM_STRUT_PARTIAL** support, it perfectly respects the work area of external panels like **Polybar**, **Lemonbar**, or **Tint2**.
*   **Scriptable Spines:** While Dechinus handles keys internally, the `dewmc` utility allows it to "hear" the rest of your system. You can automate your workflow with shell scripts or make your status bar clickable.

## Features
*   **The 2K Flex:** A complete, reparenting engine with multi-monitor support (XRandR) and IPC scriptability in ~1,900 lines of C.
*   **Independence:** No `Xresources` (XRDB) required. Features a native C configuration parser for zero-overhead startup.
*   **Smart Multi-Monitor Logic:** Intelligent tag swapping between screens to prevent duplicate views and "ghost" tags.
*   **XDG Base Directory Compliant:** Configuration stays organized in `~/.config/dewm/`.
*   **Zero-Overhead Modularity:** Get the scriptable power of `bspwm` with the standalone footprint and simplicity of `dwm`.

## Installation

### Dependencies
*   `X11` (libX11)
*   `Xft` (for anti-aliased fonts)
*   `pkg-config`
*   `libxrandr` (required for multi-monitor setups)

### Build & Install
```bash
make
sudo make install
```
*The smart installer detects the `$SUDO_USER` and deploys the `dewmrc` configuration safely to `~/.config/dewm/`.*

## Contributing
Help us keep the code sharp and the footprint small.
1. Fork the repo.
2. Follow the "Sea Urchin" philosophy (Minimalism + Usability).
3. Submit a PR.

## License
**MIT License** — See [LICENSE](./LICENSE)
