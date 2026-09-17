# Register Drop Counter — web

Browser build of the same drop math as Windows, Android, iPhone, and C23.

Version **2.15.0** (2026-09-17). Designed by Ronald Robbins Jr and SuperGrok.

## Open

1. Open `index.html` in Chrome, Edge, Safari, or Firefox.
2. Or serve the folder: `npx serve .` then visit the printed URL.

No build step. Counts save in this browser.

## Using it

- Yellow **Count** cells are the only inputs.
- Click a till tab to switch. Double-click (or Rename) to change R1–R10 names.
- **Drop** is how many of each denom to pull so the drawer resets to the register base.
- **Left** turns green when it equals the base, red when it does not.
- Copy Cash Log / EOD / Reset copy the 10-till table as tab-separated values.
- Clear saves a snapshot first. Undo clear and History restore old counts.
- Drop slip opens a 42-column Star 80 mm layout for print.

## Files

| File | Role |
| --- | --- |
| `index.html` | Page shell |
| `styles.css` | Layout and colors |
| `app.js` | Drop engine, history, clipboard, slip |
