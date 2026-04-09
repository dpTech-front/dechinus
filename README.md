# Dechinus WM
![Captain Dechinus](assets/captain-dechinus.svg)
> **Stay Sharp. Stay Lean.** A self-sustained, scriptable window manager for X11.

## The Ecosystem
To keep Dechinus minimal and efficient, the project is divided into three distinct parts:

*   **`dewm` [The Manager]:** The core window manager binary. It handles tiling logic, reparenting frames, and input events.
*   **`dewmrc` [The DNA]:** The configuration file (`~/.config/dewm/dewmrc`). It defines your static settings like colors, layouts, and initial rules.
*   **`dewmc` [The Controller]:** A lightweight CLI utility used to send real-time commands to the manager from scripts, panels, or the terminal.

## The Identity: Self-Sustained Minimalism
Dechinus bridges the gap between the "Manual" world (keyboard shortcuts) and the "Automated" world (shell scripts). It is a **Self-Sustained Monolith**—it handles its own keys internally for maximum speed, but exposes its "Command Spines" to the outside world for ultimate flexibility.

## Core Philosophy
*   **Dynamic Fusion:** Elegant reparented titlebars for floating windows; sharp, chrome-less borders for tiling.
*   **The 2K Flex:** A complete engine with multi-monitor support and scriptable IPC in ~1,900 lines of C.
*   **Unix Synergy:** Full **_NET_WM_STRUT_PARTIAL** support to respect third-party panels like Polybar or Tint2.
*   **Zero-Overhead Scriptability:** Get the power of a modular WM with the footprint of a standalone binary.

## Why `dewmc`?
While `dewm` handles your keyboard shortcuts internally, the `dewmc` utility allows the WM to "hear" the rest of your system:
*   **Automate:** Write a "Movie Mode" script that switches tags and sets layouts with one command.
*   **Integrate:** Make your status bar clickable by having it trigger `dewmc layout tile`.
*   **Override:** Change gaps or toggle titlebars on the fly without editing your `dewmrc`.

## Installation
```bash
make
sudo make install
```
*The installer automatically deploys the `dewmrc` template to `~/.config/dewm/`.*

## Usage & Control
*   **Configure:** Edit `~/.config/dewm/dewmrc`.
*   **Control:** Use the CLI for dynamic changes:
    *   `dewmc view 2` (Switch to tag 2)
    *   `dewmc layout monocle` (Change layout)
    *   `dewmc set gap 10` (Change gaps live)

## Contributing
We value "Suckless" efficiency. Keep the core binary small, the logic sharp, and the organism independent.
1. Fork the repo.
2. Submit a PR.

## License
**MIT License** — See [LICENSE](./LICENSE)

---
