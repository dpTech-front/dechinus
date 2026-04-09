# Dechinus WM
![Captain Dechinus](assets/captain-dechinus.svg)
> **Stay Sharp. Stay Lean.** A self-sustained, reparenting window manager for X11.

## The Identity: Self-Sustained Minimalism
**Dechinus (`dewm`)** is designed as a single, high-performance organism. It follows the Suckless philosophy of being a standalone binary while incorporating the Unix efficiency of being scriptable. It is the only window manager that provides **Reparenting (Titlebars)** and **Tiling** in a single C file under 2,000 lines without requiring external daemons to function.

## Core Philosophy: The Unified Urchin
*   **Single-Binary Focus:** No external key-daemons or complex IPC sockets. Everything you need to manage windows is contained within the `dewm` binary, keeping memory usage at the absolute minimum.
*   **The Command Spines:** While self-sustained, Dechinus exposes X11 Atoms that allow it to be controlled via external scripts. It is a "Scriptable Monolith"—efficiency without the overhead of modularity.
*   **Chrome-less Tiling:** Spines (titlebars) retract during tiling to maximize work space. Only a sharp, configurable border remains.
*   **Elegant Floating:** Spines emerge when a window floats, providing beautiful, reparented decorations and mouse-driven widgets.
*   **Panel Synergy:** Explicitly respects third-party panels through modern **_NET_WM_STRUT_PARTIAL** support. It doesn't try to be a panel; it provides the space for one.

## Efficiency Flex
*   **Memory:** < 2MB RSS.
*   **Code:** ~1,900 lines of C.
*   **Speed:** Direct X11 event handling with zero IPC latency.
*   **Independence:** No `Xresources` (XRDB) required; features a native C configuration parser.

## Features
*   **Hybrid Tiling/Floating:** Mathematical precision meets desktop elegance.
*   **Smart Multi-Monitor Logic:** Tag swapping between screens to prevent duplicate views and "ghost" tags.
*   **XDG Base Directory Compliant:** Keeps your `$HOME` clean.
*   **Intelligent Installer:** Automatically deploys `dewmrc` to the correct user directory.

## Installation
```bash
make
sudo make install
```
*Add `exec dewm` to your `.xinitrc`.*

## Contributing
We value "Suckless" efficiency. Every new feature must be implemented with the smallest possible footprint.
1. Fork the repo.
2. Keep the logic internal and the code lean.
3. Submit a PR.

## License
**MIT License** — See [LICENSE](./LICENSE)

***

### Why this is the right path:
If you split it into many components, you lose the **"Sea Urchin"** metaphor (one shell, one organism). By keeping the keys internal but adding the "Command Spines" (Atoms), you get:
1.  **Ease of use:** It works perfectly as soon as you install it.
2.  **Advanced power:** You can still write a bash script to change layouts or automate your desktop.
3.  **Maximum "Suckless" points:** You prove that you don't need 50,000 lines of code or 5 different processes to have a modern, scriptable desktop.

**Does this "Self-Sustained but Scriptable" approach fit your vision better?**
