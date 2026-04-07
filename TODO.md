This is the consolidated Master TODO List for Dechinus WM (dewm), containing all technical requirements, snippets, and milestones from Audits 1 through 8. All checkboxes have been removed, but all detailed data and structural progress markers have been retained.

---

### 📝 DECHINUS WM CONSOLIDATED MASTER TODO LIST

#### 🟢 PHASE 0: THE MODERN BASELINE (v0.1.0)
1.  **Branding Overhaul:** Complete rename from Echinus to Dechinus/dewm across all source files, headers, and config files.
2.  **Binary & Config Renaming:** Set primary binary output to `dewm` and default configuration filename to `dewmrc`.
3.  **XDG Base Directory Compliance:** Relocate default configuration search path from `~/.echinus` to `$HOME/.config/dewm/dewmrc`.
4.  **Macro Cleanup:** Delete "Must Die" macros; replace with explicit `Monitor *m` pointers across the entire source for performance and clarity.
5.  **EWMH Sanitization:** Refactor `atomnames` into a 1D pointer list to eliminate compiler warnings.
6.  **Stability Optimization:** Implement a hard fallback to `BlackPixel` in `getcolor` to prevent WM crashes when the user provides invalid hex codes.
7.  **Smart Installation Logic:** Refactor Makefile to detect `$SUDO_USER` to ensure configuration and assets are installed to the actual user's home directory.

#### 🏗️ PHASE 1: ARCHITECTURE & INDEPENDENCE (v0.2.0)
8.  **Native Parser (The Suckless Parser):** Rip out `<X11/Xresource.h>` and all `Xrm` library dependencies to remove XRDB as a requirement.
9.  **Native C Reader Implementation:** Build a lightweight `FILE*` reader in `parse.c` using `fgets` and `sscanf` to parse `key: value` syntax directly.
10. **Parser Fallback Logic:** If `~/.config/dewm/dewmrc` is missing, the parser must default to internal hard-coded values.
11. **Internal Key-Value Struct:** Update `getresource()` to pull values from an internal struct rather than querying Xresources.
12. **Variable Support:** Implement support for a `modkey` variable in the config to be set once and referenced for all bindings.
13. **RegEx Pre-compilation Logic:** Integrate a POSIX RegEx subsystem into the parser to optimize window rule matching (`applyrules`) during initialization.
14. **Binary Unification:** Perform a final audit to ensure all remaining "echinus" strings in manual pages, error logs, and comments are replaced.

#### 🧠 PHASE 2: HARVESTED LOGIC & WINDOW MANAGEMENT (v0.2.0)
15. **Multi-Monitor Tag Swapping:** Adapt `view()` logic so that attempting to view a tag already active on another monitor swaps the tags between the two monitors to prevent duplicate views.
16. **Tag Inheritance Logic:** Implement `memcpy` of tags for transient windows (dialogs/pickers) in `manage()` so they automatically spawn on their parent's workspace.
    *   *Snippet:* `memcpy(c->tags, cm->seltags, ntags * sizeof(cm->seltags[0]));`
17. **Precision Resize (PAspect):** Implement `PAspect` ratio handling in `resize()` and `updatesizehints()` to properly support video players (mpv/mplayer) and eliminate black-bar gaps.
18. **Layered Stacking Hierarchy:** Refactor `restack()` to enforce a strict 4-layer visual hierarchy: `Floating Windows > Docks/Bastards > Tiled Windows > Desktop`.
19. **Stack Focus Preservation:** Hard-code `attachaside` logic so new windows attach as slaves at the end of the stack, keeping the current Master window in focus.
20. **Reparent Safety Implementation:** Integrate `reparentnotify()` to detect if a window is stolen/moved by an external process and call `unmanage()` to avoid orphaned frames or ghost pointers.

#### 📐 PHASE 3: EWMH & MATHEMATICAL PRECISION (v0.2.0)
21. **Precision Struts:** Refine `updategeom()` to calculate the Working Area (`wa`) using absolute `min/max` bounds across all monitors to prevent panel/window overlap.
22. **Strut Partial Support:** Integrate modern `_NET_WM_STRUT_PARTIAL` support for better compatibility with panels like Polybar, Tint2, and Lemonbar.
    *   *Snippet:* `state = (unsigned long*)getatom(c->win, atom[StrutPartial], &n);`
23. **EWMH Communication:** Ensure the WM emits `_NET_CLIENT_LIST`, `_NET_ACTIVE_WINDOW`, and `_NET_CURRENT_DESKTOP` updates on every window or tag change.
24. **Hard-Tiling Math Refactor:** Rewrite `tile()` and `bstack()` in `dechinus.c` using pure integer division to eliminate 1px rounding gaps without extra overhead.
25. **Configurable Window Gaps:** Update tiling calculations to respect `options.gap` while maintaining strict pixel-perfect integer alignment.

#### 🎨 PHASE 4: UI OPTIMIZATION (v0.2.0)
26. **Dynamic Titlebar Mapping:** Update `updateframe()` to map and draw titlebars **ONLY** if `isfloating == True`. 
27. **Tiled Aesthetic Enforcement:** Force Titlebar Height (`c->th`) to `0` for all tiled windows; tiled windows must remain chrome-less to maximize space.
28. **Titlebar Element Rendering:** Implement the flexible `drawelement` system (N, I, M, C, T) specifically for floating window titlebar rendering.
29. **Execution Bypass:** Update `draw.c` so tiled windows completely skip the `drawclient` code path to save CPU cycles.
30. **Border Consistency:** Ensure tiled windows maintain a configurable border (e.g., 1px) even when the titlebar is removed.

#### 🧹 PHASE 5: MAINTENANCE & ECOSYSTEM (v0.2.0)
31. **Integrated Tooling Modernization:** Adapt the harvested `tests/ewmhpanel.c` utility and its Makefile to work with the modernized `dechinus.h` headers.
32. **Makefile Hardening:** Ensure robust directory creation and permission handling for `~/.config/dewm/` across different Linux distributions.
33. **Final Branch Pruning:** Once logic extraction is verified, delete audited branches: `gamaral`, `abs`, `bububu`, `recursive`, `reparent`, `testing`, `tint2`, and `title`.
34. **Release Tagging:** Perform a final rebase of the master branch and tag as `v0.2.0-stable`.
