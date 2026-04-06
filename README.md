# Dechinus WM

> Sharp, minimal, and fast — a tiling window manager for X11.

![Captain Dechinus](assets/captain-dechinus.svg)

---

## Overview

**Dechinus WM (`dewm`)** is a lightweight tiling window manager for X11, forked from Echinus and modernized for current Linux workflows.

It focuses on **active maintenance**, **clean defaults**, and a **consistent configuration experience** while staying true to the simplicity of classic tiling WMs.

---

## Features

* **XDG Base Directory Compliant**
  Uses `~/.config/dewm/` instead of cluttering the home directory.

* **Lightweight & Fast**
  Minimal dependencies with efficient runtime performance.

* **Highly Configurable**
  Customize layouts, gaps, titlebars, and keybindings via Xresources.

* **Smart Installation**
  Automatically detects the invoking user and deploys configuration safely — avoiding common root install issues.

---

## Installation

### Dependencies

* `X11`
* `Xft`
* `pkg-config`
* *(Optional)* `libxrandr` (for multi-monitor setups)

### Build & Install

```bash
git clone https://github.com/dpTech-front/dechinus.git
cd dechinus

make
sudo make install
```

---

## Automated Environment Setup

Unlike traditional window managers, **Dechinus includes an intelligent installer**.

Running `sudo make install` will:

1. Install the `dewm` binary to `/usr/local/bin`
2. Detect the original user (via `SUDO_USER`)
3. Deploy default configuration and assets to:

```bash
~/.config/dewm/
```

> No manual copying required — your environment is ready immediately.

---

## Running Dechinus

Add this to your `.xinitrc`:

```bash
exec dewm
```

Or select **dewm** from your display manager.

---

## Configuration

Dechinus uses **Xresources** for configuration.

Edit:

```bash
~/.config/dewm/dewmrc
```

---

### Core Settings

| Setting              | Description                              | Example |
| -------------------- | ---------------------------------------- | ------- |
| `Dechinus*deflayout` | Default layout (`i`, `f`, `t`, `b`, `m`) | `t`     |
| `Dechinus*gap`       | Window gaps (pixels)                     | `5`     |
| `Dechinus*mwfact`    | Master area size ratio                   | `0.55`  |
| `Dechinus*nmaster`   | Number of master windows                 | `1`     |
| `Dechinus*sloppy`    | Focus mode (0–3)                         | `1`     |
| `Dechinus*modkey`    | Modifier key (`A`, `W`, `S`, `C`)        | `W`     |

---

### Titlebar Settings

* `Dechinus*decoratetiled` — Enable titlebars in tiled mode (`1` or `0`)
* `Dechinus*titlelayout` — Titlebar format
  Example: `"N  IMC"` (Name, Iconify, Maximize, Close)

---

## Why Dechinus?

Dechinus modernizes the Echinus philosophy by:

* Adopting **XDG standards**
* Eliminating fragile manual setup steps
* Providing a **clean, predictable user experience**

It keeps the spirit of classic tiling WMs — but removes the friction.

---

## Contributing

Contributions are welcome.

* Improve layouts
* Fix bugs
* Refine defaults
* Submit pull requests

---

## License

**MIT License** — see [LICENSE](./LICENSE)
