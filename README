# Dechinus WM 
> Sharp, minimal, and fast — a tiling window manager for X11.

![Captain Dechinus](assets/captain-dechinus.svg)

## Overview

**Dechinus WM (dewm)** is a lightweight tiling window manager for X11, forked from Echinus.

It preserves the original simplicity and speed while introducing active maintenance,
improved defaults, and a cleaner, more consistent configuration experience.

## Features

* Lightweight and fast with minimal dependencies
* Configurable layouts, gaps, titlebars, and modkeys
* Customizable panels and tagbars
* User configuration stored in `~/.config/dechinus`

## Installation

Dependencies: `X11`, `Xft`, `pkg-config`. Optional for multihead setups: `libxrandr`.

```bash
# Clone the repository
git clone https://github.com/dpTech-front/dechinus.git
cd dechinus

# Build & install
make
sudo make install

# Setup initial config
mkdir -p ~/.config/dechinus
cp -r CONFDIR ~/.config/dechinus
```

> Adjust `~/.config/dechinus/dewmrc` to set your layouts, gaps, and master window size.

## Running

To start Dechinus WM, add the following to your `.xinitrc`:

```bash
exec dewm
```

Alternatively, select **dewm** from your display manager.

## Configuration

### Main Settings

| Setting              | Description                                                                  | Example |
| -------------------- | ---------------------------------------------------------------------------- | ------- |
| `Dechinus*deflayout` | Default layout: i=ifloating, f=floating, t=tiled, b=bottomstack, m=maximized | `t`     |
| `Dechinus*gap`       | Gap between windows (pixels)                                                 | `5`     |
| `Dechinus*mwfact`    | Fraction of master area                                                      | `0.55`  |
| `Dechinus*nmaster`   | Number of master clients                                                     | `1`     |
| `Dechinus*sloppy`    | Focus behavior: 0=click, 1=sloppy floating, 2=sloppy all, 3=sloppy+raise     | `1`     |
| `Dechinus*modkey`    | Modifier key: A=Alt, W=Win, S=Shift, C=Ctrl                                  | `W`     |

### Title Settings

* `Dechinus*decoratetiled` – Show titlebars in tiled mode (1/0)
* `Dechinus*titlelayout` – Titlebar format (`"N  IMC"` for tags, icons, master, client)

Full options are in `config.h` and `dechinusrc`.

## Panels & Tagbars

* Fully configurable for status, time, CPU, etc.
* Supports custom scripts and widgets.

## Why Dechinus?

Dechinus aims to keep the simplicity of classic tiling window managers
while remaining actively maintained and improving usability over time.

It avoids unnecessary complexity while refining defaults and consistency.

## Contributing

Contributions are welcome! Fork, tweak layouts, fix bugs, and submit pull requests.

## License

**MIT License** — See [LICENSE](./LICENSE) for full details.
