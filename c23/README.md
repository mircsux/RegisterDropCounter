# Register Drop Counter — ISO C23

Version **2.21.0** (2026-09-17). Designed by Ronald Robbins Jr and SuperGrok.

Hosted C23 library and console app. Same integer-cent drop as the Excel sheet,
the Windows app, and the iPhone app. **No C++.**

- Drop order: $100, $50, $20, $10, $5, **$2**, $1, then coins, then rolls, pennies.
- Yellow-cell idea: you only type **counts**. Amount, Drop, and Left are computed.
- Clear writes history. **Undo** brings the drawer back. Till names default to R1–R10.
- Drop slip is an 80 mm / 42-column Star TSC100 receipt (`slip` command).
- Files: `RegisterDropCounter.state`, `.names`, `.history`, `.slip` (same layouts as Windows).

Build with `-std=c23` or `-std=c2x` / MSVC `/std:clatest`. `make test` checks the
2026-08-10 workbook sample.
