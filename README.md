# Register Drop Counter

A counting tool for cash drawers. Open `RegisterDropCounter.sln` in Visual Studio 2022.

Version **2.33.0** (2026-09-23). Designed by Ronald Robbins Jr and SuperGrok.

## Open in Visual Studio

1. Install **Visual Studio 2022** with the **Desktop development with C++** workload.
2. Double-click `RegisterDropCounter.sln`.
3. Set the toolbar to **Release | x64**.
4. Press **F5**.

See `HOW_TO_OPEN.txt` for the short version. No NuGet packages, no MFC.

To check drop math against the 2026-08-10 workbook sample, build and run **DropEngineTest**.

## Using it

- Yellow **Count** cells are the only inputs. Tab, Enter, and arrow keys move through them. On a phone, use the number pad.
- Click a till name (defaults **R1-R10**) to rename it.
- **Drop** is how many of each denom to pull so the drawer resets to the register base.
- **Left** turns green when it equals the register base, red when it does not.
- Drop order: $100, $50, $20, $10, $5, $2, $1, then quarters, dimes, nickels, rolls, pennies.
- **$2 bills** count, drop, and copy like the other bills.
- **R1-R10** buttons copy one register row as tab-separated values for the national cash log.
- **Copy Cash Log**, **Copy EOD**, and **Copy Reset** copy that whole table only.
- **Clear** and **Clear All** save a snapshot first. **Undo clear** brings back the last drawer. **History** searches by date, shows old counts, and restores.
- **Drop slip** prints an 80 mm Star TSC100 receipt (42 columns, 72 mm print width) for the safe (date, till, drop by denom, bag number, initials).
- **About** has the how-to. **Show changelog** lists Excel history, drop fixes, Windows, and the Android phone layout. Today's date is on the main window.
- **Options** can sync history to OneDrive. Save, Load, and Sync now use `RegisterDropCounter.history` in your OneDrive folder.

Register base is $100 / $200 / $300 / $400 / $500. Counts save next to the .exe as `RegisterDropCounter.state`. History saves as `RegisterDropCounter.history`. Window position and size save as `RegisterDropCounter.window`.

## Files

| File | Role |
| --- | --- |
| `RegisterDropCounter.sln` | Open this in Visual Studio |
| `RegisterDropCounter/DropEngine.h` | Integer-cent drop math |
| `RegisterDropCounter/Version.h` | Product version |
| `RegisterDropCounter/App.cpp` | Win32 UI, clipboard, History, About |
| `DropEngineTest/DropEngineTest.cpp` | Workbook sample tests |

## Other builds (same drop math)

- **Web** — `web/index.html` in any browser. See `web/HOW_TO_OPEN.txt`.
- **Windows (C++ Win32)** — this folder. Open `RegisterDropCounter.sln`.
- **Android (Kotlin)** — `android/` folder in Android Studio. See `android/HOW_TO_OPEN.txt`.
- **iPhone (SwiftUI)** — `ios/RegisterDropCounter.xcodeproj`. See `ios/HOW_TO_OPEN.txt`.
- **Web (HTML/JS)** — `web/index.html` in any browser. See `web/HOW_TO_OPEN.txt`.
- **ISO C23 console** — `c23/RegisterDropCounter-C23.sln` or `c23/Makefile`. No C++.
