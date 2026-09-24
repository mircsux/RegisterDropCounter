# Register Drop Counter — Web

A counting tool for cash drawers. Open `index.html` in any browser.

Version **2.32.0** (2026-09-23). Designed by Ronald Robbins Jr and SuperGrok.

## Open

1. Unzip.
2. Double-click `index.html`.
3. Optional: put this folder on a web host so phones can open the same counter.

No build step. `index.html`, `rdc.css`, and `rdc.js` are the whole app.

## Using it

- Yellow **Count** cells are the only inputs. Tab, Enter, and arrow keys move through them. On a phone, use the number pad.
- Click a till name (defaults **R1–R10**) to rename it.
- **Drop** is how many of each denom to pull so the drawer resets to the register base.
- **Left** turns green when it equals the register base.
- Drop order: $100, $50, $20, $10, $5, $2, $1, then quarters, dimes, nickels, rolls, pennies.
- **Drop slip** prints an 80 mm Star TSC100 receipt (42 columns).
- **Options** has Dark mode. The setting saves in this browser.

## Files

| File | Role |
| --- | --- |
| `index.html` | Open this |
| `rdc.js` | Integer-cent drop math + UI |
| `rdc.css` | Light and dark sheet |
| `HOW_TO_OPEN.txt` | Setup steps |

## Other builds (same drop math)

- **Windows (C++ Win32)** — `RegisterDropCounter.sln`
- **Android (Kotlin)** — Android Studio project
- **iPhone (SwiftUI)** — Xcode project
- **ISO C23 console** — `RegisterDropCounter-C23.sln` or `make`
