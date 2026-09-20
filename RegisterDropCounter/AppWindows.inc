void RelayoutHistory(HWND h);
void RelayoutAbout(HWND h);
void RelayoutOptions(HWND h);

int CurrentHistIndex() {
  auto idx = rdc::FilterByDate(gHistory, gHistFilter);
  if (gHistSel < 0) return -1;
  for (int i : idx)
    if (i == gHistSel) return gHistSel;
  return idx.empty() ? -1 : idx[0];
}

void FillDenomList(HWND h, const rdc::HistoryEntry& e, int reg) {
  HWND lv = GetDlgItem(h, IDC_H_DENOM);
  ListView_DeleteAllItems(lv);
  auto r = rdc::ComputeRegister(e.registers[reg], e.base);
  for (int d = 0; d < rdc::DenomCount; ++d) {
    std::wstring lab = rdc::kLabels[d];
    if (rdc::kRollLetter[d][0]) lab = std::wstring(rdc::kRollLetter[d]) + L" " + lab;
    AddLvRow(lv, d,
             {lab, std::to_wstring(r.counts.n[d]), std::to_wstring(r.drop.n[d]),
              std::to_wstring(r.left.n[d])});
  }
  wchar_t cap[48];
  swprintf(cap, 48, L"Counts for R%d", reg + 1);
  SetDlgItemTextW(h, IDC_H_DENLBL, cap);
}

void FillDetailList(HWND h, const rdc::HistoryEntry& e) {
  HWND lv = GetDlgItem(h, IDC_H_DETAIL);
  ListView_DeleteAllItems(lv);
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    auto r = rdc::ComputeRegister(e.registers[i], e.base);
    wchar_t lab[8];
    swprintf(lab, 8, L"R%d", i + 1);
    const wchar_t* st = !r.hasCount ? L"Empty" : (r.balanced ? L"Balanced" : L"Off base");
    std::wstring status = st;
    if (e.kind == rdc::HistRegister && e.registerIndex == i) status += L"  cleared";
    AddLvRow(lv, i,
             {lab, rdc::Money(r.amountCents), rdc::Money(r.dropCents), rdc::Money(r.leftCents),
              status});
  }
  if (gHistDetailReg < 0 || gHistDetailReg >= rdc::kRegisterCount) gHistDetailReg = 0;
  ListView_SetItemState(lv, gHistDetailReg, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
  FillDenomList(h, e, gHistDetailReg);
}

void RefreshHistoryUi(HWND h) {
  HWND lv = GetDlgItem(h, IDC_H_LIST);
  ListView_DeleteAllItems(lv);
  auto idx = rdc::FilterByDate(gHistory, gHistFilter);
  int row = 0;
  int selectRow = 0;
  bool found = false;
  for (int i : idx) {
    const auto& e = gHistory[i];
    auto sum = rdc::Summarize(e);
    AddLvRow(lv, row,
             {rdc::FormatWhen(e.at), rdc::HistoryLabel(e, gNames), std::to_wstring(sum.counted),
              rdc::Money(sum.amountCents), rdc::Money(sum.dropCents)});
    LVITEMW it{};
    it.mask = LVIF_PARAM;
    it.iItem = row;
    it.lParam = i;
    ListView_SetItem(lv, &it);
    if (i == gHistSel) {
      selectRow = row;
      found = true;
    }
    ++row;
  }
  wchar_t st[80];
  if (gHistFilter.empty())
    swprintf(st, 80, L"%d saved", (int)gHistory.size());
  else
    swprintf(st, 80, L"%d on %s", (int)idx.size(), gHistFilter.c_str());
  SetDlgItemTextW(h, IDC_H_STATUS, st);
  SetDlgItemTextW(h, IDC_H_STAMP, rdc::FormatSheetDate().c_str());

  if (idx.empty()) {
    gHistSel = -1;
    ListView_DeleteAllItems(GetDlgItem(h, IDC_H_DETAIL));
    ListView_DeleteAllItems(GetDlgItem(h, IDC_H_DENOM));
    SetDlgItemTextW(h, IDC_H_DENLBL, L"Counts for selected register");
    return;
  }
  if (!found) gHistSel = idx[0];
  ListView_SetItemState(lv, found ? selectRow : 0, LVIS_SELECTED | LVIS_FOCUSED,
                        LVIS_SELECTED | LVIS_FOCUSED);
  FillDetailList(h, gHistory[gHistSel]);
}

void LayoutHistory(HWND h) {
  MakeStatic(h, IDC_H_TITLE, L"History", 16, 10, 200, 24, SS_LEFT);
  SendMessageW(GetDlgItem(h, IDC_H_TITLE), WM_SETFONT, (WPARAM)gFontBold, TRUE);
  MakeStatic(h, IDC_H_STAMP, rdc::FormatSheetDate().c_str(), 400, 12, 300, 22, SS_RIGHT);
  SendMessageW(GetDlgItem(h, IDC_H_STAMP), WM_SETFONT, (WPARAM)gFontBold, TRUE);

  MakeStatic(h, IDC_H_DATELBL, L"Date", 16, 58, 40, 22, SS_LEFT);
  MakeEditText(h, IDC_H_DATE, 60, 54, 140, 28);
  MakeBtn(h, IDC_H_TODAY, L"Today", 208, 54, 72, 28);
  MakeBtn(h, IDC_H_ALL, L"All dates", 288, 54, 92, 28);

  HWND list = MakeLv(h, IDC_H_LIST, 16, 92, 850, 160);
  InsertCol(list, 0, L"When", 210);
  InsertCol(list, 1, L"Action", 200);
  InsertCol(list, 2, L"Counted", 80);
  InsertCol(list, 3, L"Amount", 120);
  InsertCol(list, 4, L"Drop", 120);

  MakeBtn(h, IDC_H_RESTORE, L"Restore to Counter", 16, 262, 160, 28);
  MakeBtn(h, IDC_H_CLEAR, L"Clear history", 184, 262, 120, 28);
  MakeBtn(h, IDC_H_CLOSE, L"Close", 312, 262, 80, 28);
  MakeStatic(h, IDC_H_STATUS, L"", 400, 266, 300, 22, SS_RIGHT);

  MakeStatic(h, IDC_H_DETLBL, L"Registers in this snapshot", 16, 300, 400, 18, SS_LEFT);
  HWND det = MakeLv(h, IDC_H_DETAIL, 16, 320, 420, 160);
  InsertCol(det, 0, L"Reg", 50);
  InsertCol(det, 1, L"Amount", 90);
  InsertCol(det, 2, L"Drop", 90);
  InsertCol(det, 3, L"Left", 90);
  InsertCol(det, 4, L"Status", 100);

  MakeStatic(h, IDC_H_DENLBL, L"Counts for selected register", 450, 300, 400, 18, SS_LEFT);
  HWND den = MakeLv(h, IDC_H_DENOM, 450, 320, 420, 200);
  InsertCol(den, 0, L"Denom", 100);
  InsertCol(den, 1, L"Count", 80);
  InsertCol(den, 2, L"Drop", 80);
  InsertCol(den, 3, L"Left", 80);
  RelayoutHistory(h);
}

void RelayoutHistory(HWND h) {
  RECT rc{};
  GetClientRect(h, &rc);
  const int W = rc.right;
  const int H = rc.bottom;
  if (W < 240 || H < 180) return;

  const int pad = 16;
  const int gap = 10;
  const int headerH = 44;
  const int btnH = 28;
  const int searchH = 28;
  const int lblH = 18;
  auto set = [&](int id, HFONT f) {
    HWND c = GetDlgItem(h, id);
    if (c) SendMessageW(c, WM_SETFONT, (WPARAM)f, TRUE);
  };
  set(IDC_H_TITLE, gFontBold);
  set(IDC_H_DATELBL, gFont);
  set(IDC_H_DATE, gMono);
  set(IDC_H_TODAY, gFont);
  set(IDC_H_ALL, gFont);
  set(IDC_H_STAMP, gFontBold);
  set(IDC_H_RESTORE, gFont);
  set(IDC_H_CLEAR, gFont);
  set(IDC_H_CLOSE, gFont);
  set(IDC_H_STATUS, gFont);
  set(IDC_H_DETLBL, gFont);
  set(IDC_H_DENLBL, gFont);
  set(IDC_H_LIST, gMono);
  set(IDC_H_DETAIL, gMono);
  set(IDC_H_DENOM, gMono);

  const int ySearch = headerH + 12;
  const int yList = ySearch + searchH + gap;
  const bool sideBySide = W >= 900;
  const int belowSearch = H - pad - yList;
  const int chrome = gap + btnH + gap + lblH + 4;
  int listH;
  int panelH;
  if (sideBySide) {
    panelH = rdc::MaxI(160, belowSearch * 46 / 100);
    listH = belowSearch - chrome - panelH;
    if (listH < 110) {
      listH = 110;
      panelH = belowSearch - chrome - listH;
    }
    if (panelH < 140) {
      panelH = 140;
      listH = belowSearch - chrome - panelH;
      if (listH < 90) listH = 90;
    }
  } else {
    const int stackedChrome = gap + btnH + gap + (lblH + 4) * 2 + gap;
    listH = belowSearch * 28 / 100;
    if (listH < 90) listH = 90;
    if (listH > 180) listH = 180;
    int rest = belowSearch - listH - stackedChrome;
    int each = rest / 2;
    if (each < 90) {
      each = 90;
      listH = belowSearch - stackedChrome - each * 2;
      if (listH < 80) listH = 80;
    }
    panelH = each;
  }

  const int yBtn = yList + listH + gap;
  const int yPanels = yBtn + btnH + gap;
  const int innerW = W - pad * 2;

  HDWP dwp = BeginDeferWindowPos(20);
  if (!dwp) return;
  Place(&dwp, h, IDC_H_TITLE, pad, 10, 220, 24);
  Place(&dwp, h, IDC_H_STAMP, W - pad - 340, 12, 340, 22);
  Place(&dwp, h, IDC_H_DATELBL, pad, ySearch + 5, 40, 20);
  Place(&dwp, h, IDC_H_DATE, pad + 48, ySearch, 140, searchH);
  Place(&dwp, h, IDC_H_TODAY, pad + 196, ySearch, 72, searchH);
  Place(&dwp, h, IDC_H_ALL, pad + 276, ySearch, 92, searchH);
  Place(&dwp, h, IDC_H_LIST, pad, yList, innerW, listH);

  Place(&dwp, h, IDC_H_RESTORE, pad, yBtn, 160, btnH);
  Place(&dwp, h, IDC_H_CLEAR, pad + 168, yBtn, 120, btnH);
  Place(&dwp, h, IDC_H_CLOSE, pad + 296, yBtn, 80, btnH);
  const int statusX = pad + 388;
  const int statusW = rdc::MaxI(80, W - pad - statusX);
  Place(&dwp, h, IDC_H_STATUS, statusX, yBtn + 4, statusW, 20);

  if (sideBySide) {
    const int colGap = 12;
    const int colW = (innerW - colGap) / 2;
    const int xR = pad + colW + colGap;
    Place(&dwp, h, IDC_H_DETLBL, pad, yPanels, colW, lblH);
    Place(&dwp, h, IDC_H_DETAIL, pad, yPanels + lblH + 4, colW, panelH);
    Place(&dwp, h, IDC_H_DENLBL, xR, yPanels, colW, lblH);
    Place(&dwp, h, IDC_H_DENOM, xR, yPanels + lblH + 4, colW, panelH);
  } else {
    Place(&dwp, h, IDC_H_DETLBL, pad, yPanels, innerW, lblH);
    Place(&dwp, h, IDC_H_DETAIL, pad, yPanels + lblH + 4, innerW, panelH);
    const int yDen = yPanels + lblH + 4 + panelH + gap;
    Place(&dwp, h, IDC_H_DENLBL, pad, yDen, innerW, lblH);
    Place(&dwp, h, IDC_H_DENOM, pad, yDen + lblH + 4, innerW, panelH);
  }
  EndDeferWindowPos(dwp);

  const int listParts[] = {210, 200, 80, 120, 120};
  const int detParts[] = {48, 90, 90, 90, 110};
  const int denParts[] = {110, 80, 80, 80};
  SizeLvCols(GetDlgItem(h, IDC_H_LIST), listParts, 5);
  SizeLvCols(GetDlgItem(h, IDC_H_DETAIL), detParts, 5);
  SizeLvCols(GetDlgItem(h, IDC_H_DENOM), denParts, 4);
}

void RestoreSelected(HWND owner) {
  const int i = CurrentHistIndex();
  if (i < 0) {
    MessageBoxW(owner, L"Select a snapshot first.", L"History", MB_OK | MB_ICONINFORMATION);
    return;
  }
  const auto& e = gHistory[i];
  wchar_t ask[160];
  swprintf(ask, 160, L"Replace live counts with this snapshot?\n\n%s\n%s",
           rdc::FormatWhen(e.at).c_str(), rdc::HistoryLabel(e, gNames).c_str());
  if (MessageBoxW(owner, ask, L"Restore snapshot", MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return;
  gBase = e.base;
  for (int r = 0; r < rdc::kRegisterCount; ++r) gRegs[r] = e.registers[r];
  gSampleScratch = false;
  HWND combo = GetDlgItem(gMain, IDC_BASE);
  for (int b = 0; b < 5; ++b)
    if (rdc::kBaseOptions[b] == gBase) SendMessageW(combo, CB_SETCURSEL, b, 0);
  LoadCountsIntoEdits(gMain);
  SaveState();
  Status(gMain, L"Restored snapshot to the Counter.");
  ShowWindow(gMain, SW_SHOW);
  SetForegroundWindow(gMain);
}

void ClearHistoryTab(HWND owner) {
  if (gHistory.empty()) return;
  if (MessageBoxW(owner, L"Clear the History tab? This cannot be undone.", L"Clear history",
                  MB_OKCANCEL | MB_ICONWARNING) != IDOK)
    return;
  gHistory.clear();
  gHistSel = -1;
  PersistHistory();
  RefreshHistoryUi(owner);
}

int HistIndexFromList(HWND lv) {
  int row = ListView_GetNextItem(lv, -1, LVNI_SELECTED);
  if (row < 0) return -1;
  LVITEMW it{};
  it.mask = LVIF_PARAM;
  it.iItem = row;
  ListView_GetItem(lv, &it);
  return (int)it.lParam;
}

LRESULT CALLBACK HistProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
    case WM_CREATE:
      LayoutHistory(h);
      SetDlgItemTextW(h, IDC_H_DATE, gHistFilter.c_str());
      RefreshHistoryUi(h);
      return 0;
    case WM_ERASEBKGND: {
      HDC dc = (HDC)w;
      RECT rc;
      GetClientRect(h, &rc);
      RECT band{0, 0, rc.right, 44};
      FillRect(dc, &band, Brush(kNavy));
      RECT rest{0, 44, rc.right, rc.bottom};
      FillRect(dc, &rest, Brush(kSheet));
      return 1;
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = (HDC)w;
      int id = GetDlgCtrlID((HWND)l);
      if (id == IDC_H_TITLE || id == IDC_H_STAMP) {
        SetBkColor(dc, kNavy);
        SetTextColor(dc, kOnNavy);
        return (LRESULT)Brush(kNavy);
      }
      SetBkColor(dc, kSheet);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kSheet);
    }
    case WM_CTLCOLOREDIT: {
      HDC dc = (HDC)w;
      SetBkColor(dc, kPaper);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kPaper);
    }
    case WM_DRAWITEM:
      DrawThemedItem((DRAWITEMSTRUCT*)l);
      return TRUE;
    case WM_NOTIFY: {
      auto* hdr = (NMHDR*)l;
      if (hdr->code == NM_CUSTOMDRAW &&
          (hdr->idFrom == IDC_H_LIST || hdr->idFrom == IDC_H_DETAIL ||
           hdr->idFrom == IDC_H_DENOM)) {
        return LvCustomDraw((NMLVCUSTOMDRAW*)l);
      }
      if (hdr->idFrom == IDC_H_LIST &&
          (hdr->code == LVN_ITEMCHANGED || hdr->code == NM_CLICK || hdr->code == NM_DBLCLK)) {
        int i = HistIndexFromList(hdr->hwndFrom);
        if (i >= 0 && i < (int)gHistory.size()) {
          gHistSel = i;
          gHistDetailReg = (gHistory[i].kind == rdc::HistRegister && gHistory[i].registerIndex >= 0)
                               ? gHistory[i].registerIndex
                               : 0;
          FillDetailList(h, gHistory[i]);
          if (hdr->code == NM_DBLCLK) RestoreSelected(h);
        }
      } else if (hdr->idFrom == IDC_H_DETAIL &&
                 (hdr->code == LVN_ITEMCHANGED || hdr->code == NM_CLICK)) {
        int row = ListView_GetNextItem(hdr->hwndFrom, -1, LVNI_SELECTED);
        if (row >= 0 && gHistSel >= 0 && gHistSel < (int)gHistory.size()) {
          gHistDetailReg = row;
          FillDenomList(h, gHistory[gHistSel], row);
        }
      }
      break;
    }
    case WM_COMMAND: {
      int id = LOWORD(w);
      int code = HIWORD(w);
      if (id == IDC_H_TODAY) {
        gHistFilter = rdc::TodayDateKey();
        SetDlgItemTextW(h, IDC_H_DATE, gHistFilter.c_str());
        RefreshHistoryUi(h);
      } else if (id == IDC_H_ALL) {
        gHistFilter.clear();
        SetDlgItemTextW(h, IDC_H_DATE, L"");
        RefreshHistoryUi(h);
      } else if (id == IDC_H_DATE && code == EN_CHANGE) {
        wchar_t buf[32];
        GetDlgItemTextW(h, IDC_H_DATE, buf, 32);
        gHistFilter = buf;
        RefreshHistoryUi(h);
      } else if (id == IDC_H_RESTORE) {
        RestoreSelected(h);
      } else if (id == IDC_H_CLEAR) {
        ClearHistoryTab(h);
      } else if (id == IDC_H_CLOSE) {
        DestroyWindow(h);
      }
      return 0;
    }
    case WM_SIZE:
      if (w != SIZE_MINIMIZED) RelayoutHistory(h);
      return 0;
    case WM_GETMINMAXINFO: {
      auto* m = (MINMAXINFO*)l;
      m->ptMinTrackSize.x = 760;
      m->ptMinTrackSize.y = 560;
      return 0;
    }
    case WM_CLOSE:
      DestroyWindow(h);
      return 0;
    case WM_DESTROY:
      gHist = nullptr;
      return 0;
  }
  return DefWindowProcW(h, msg, w, l);
}

void ShowHistory(HWND owner) {
  if (gHist && IsWindow(gHist)) {
    ShowWindow(gHist, SW_SHOW);
    SetForegroundWindow(gHist);
    RefreshHistoryUi(gHist);
    return;
  }
  static bool registered = false;
  if (!registered) {
    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = HistProc;
    wc.hInstance = gInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = Brush(kSheet);
    wc.lpszClassName = kHistClass;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);
    registered = true;
  }
  RECT rc;
  GetWindowRect(owner, &rc);
  gHist = CreateWindowExW(WS_EX_APPWINDOW, kHistClass, L"History",
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                          rc.left + 40, rc.top + 40, 1040, 720, owner, nullptr, gInst, nullptr);
  ApplyWinDark(gHist);
  ThemeChildEdits(gHist);
  ShowWindow(gHist, SW_SHOW);
  UpdateWindow(gHist);
}

constexpr wchar_t kAboutClass[] = L"RegisterDropCounterAboutWnd";
constexpr int IDC_A_CREDIT = 9101;
constexpr int IDC_A_TEXT = 9102;
constexpr int IDC_A_CLOSE = 9103;
constexpr int IDC_A_LOGBTN = 9104;
constexpr int IDC_A_LOG = 9105;
constexpr int IDC_A_VER = 9106;

const wchar_t kAboutBody[] =
    L"Count a drawer\r\n"
    L"1. Set the Register Base ($100, $200, $300, $400, or $500).\r\n"
    L"2. Open a register (R1-R10). Click the till name to rename it.\r\n"
    L"3. Type counts in the yellow cells, pennies through $100, including $2 bills.\r\n"
    L"4. Tab, Enter, or the arrow keys move to the next field. After $100 it wraps back to pennies on the same till. Shift+Tab or Shift+Enter goes back.\r\n"
    L"5. Amount, Drop, and Left fill in as you type. You do not enter those columns.\r\n"
    L"6. The Left total turns green when it equals the register base. Red means that drawer is still off base.\r\n"
    L"7. Drop slip prints an 80 mm Star TSC100 receipt: date, till, drop total, left in drawer, drop by denom, bag number, and initials.\r\n"
    L"8. Undo clear brings back the last drawer you cleared, without opening History.\r\n"
    L"\r\n"
    L"The drop\r\n"
    L"Drop is how many of each denomination to pull so the drawer resets to the base.\r\n"
    L"Order: $100, $50, $20, $10, $5, $2, $1, then quarters, dimes, nickels, rolls, pennies.\r\n"
    L"Loose coins drop before coin rolls. A penny roll still drops before 50 loose pennies.\r\n"
    L"\r\n"
    L"Cash log\r\n"
    L"Copy Cash Log, Copy EOD, and Copy Reset copy every cell in that table only.\r\n"
    L"The R1-R10 buttons copy one register's row as tab-separated values for the national cash log.\r\n"
    L"Deposit Tab is what you drop. EOD Drawer Count is the full drawer as counted. Drawer Reset Count is what stays after the drop.\r\n"
    L"\r\n"
    L"History\r\n"
    L"Clear and Clear All save the sheet first. Undo clear brings back the last one. Open History to search by date, look at old register counts, or restore a snapshot.\r\n"
    L"\r\n"
    L"Sample\r\n"
    L"File > Load Sample Drops fills every till with random test counts (0-100 on coins and bills, 0-10 on rolls). Each till's drop stays under $6,000. Those test fills do not go into History.\r\n"
    L"File > Stats for Nerds charts drop trends from History.\r\n"
    L"File > History, Options, and About. They are not header buttons.";

const wchar_t kChangelogBody[] =
    L"v2.31.0  2026-09-19  Visual Studio build\r\n"
    L"- Windows desktop app compiles again in Visual Studio 2022 after the dark-mode paint path.\r\n"
    L"\r\n"
    L"v2.30.0  2026-09-19  Windows 10 dark mode\r\n"
    L"- Dark mode paints the title bar, buttons, till tabs, cash log, History lists, and count cells. It is a night theme, not a dimmed navy sheet.\r\n"
    L"\r\n"
    L"v2.29.0  2026-09-19  Options and About under File\r\n"
    L"- Options and About are File menu items. Header keeps File, Clear all, Undo, and History.\r\n"
    L"\r\n"
    L"v2.28.0  2026-09-19  Stats for Nerds on Windows\r\n"
    L"- File > Stats for Nerds charts drop trends from History on the desktop app.\r\n"
    L"\r\n"
    L"v2.27.0  2026-09-19  Current release\r\n"
    L"- Web, Windows, iPhone, Android, and C23 all stamp as v2.27.0.\r\n"
    L"\r\n"
    L"v2.26.0  2026-09-19  Night theme\r\n"
    L"- Dark mode is its own look: carbon sheet, teal chrome, amber count cells.\r\n"
    L"\r\n"
    L"v2.25.0  2026-09-19  Stats for Nerds\r\n"
    L"- File > Stats for Nerds on the web app charts drop trends from History.\r\n"
    L"\r\n"
    L"v2.24.0  2026-09-19  Dark mode on desktop\r\n"
    L"- Count cells stay gold with dark numbers. Amount, drop, and left columns stay readable.\r\n"
    L"\r\n"
    L"v2.23.0  2026-09-19  Counting bar, downloads on About\r\n"
    L"- Header keeps File, Clear all, and Undo. Web, Android, iPhone, Windows, and C23 downloads live on the About tab of the web app.\r\n"
    L"\r\n"
    L"v2.22.0  2026-09-18  Tab wraps on the same till\r\n"
    L"- Tab, Enter, or arrow keys after $100 wrap back to pennies on that till. They no longer jump to the next register.\r\n"
    L"\r\n"
    L"v2.21.0  2026-09-17  Current release\r\n"
    L"- Web, Windows, iPhone, Android, and C23 all stamp as v2.21.0.\r\n"
    L"\r\n"
    L"v2.20.0  2026-09-17  Sample drops stay off history\r\n"
    L"- Load Sample Drops is for testing only. Clearing those tills does not write a History snapshot.\r\n"
    L"\r\n"
    L"v2.19.0  2026-09-17  Sample drop cap\r\n"
    L"- Load Sample Drops still uses random 0-100 coins/bills and 0-10 rolls, but each till's drop stays under $6,000.\r\n"
    L"\r\n"
    L"v2.18.0  2026-09-17  Sample drops\r\n"
    L"- File > Load Sample Drops fills every till with random test counts (0-100 on coins and bills, 0-10 on rolls).\r\n"
    L"\r\n"
    L"v2.17.0  2026-09-17  Drop slip totals\r\n"
    L"- Drop slip shows Drop total and Left in drawer at the top of the receipt, then again after the denom list.\r\n"
    L"\r\n"
    L"v2.16.0  2026-09-17  Web app\r\n"
    L"- Download Web unzip and open index.html in any browser. Same drop math, dark mode, history, and Star TSC100 drop slip.\r\n"
    L"\r\n"
    L"v2.15.0  2026-09-17  Dark mode polish\r\n"
    L"- Dark mode now darkens count, amount, and cash-log cells. Gold count cells stay easy to find.\r\n"
    L"\r\n"
    L"v2.14.1  2026-09-17  Visual Studio build\r\n"
    L"- Windows project compiles in Visual Studio 2022: History till selection and the till-name editor.\r\n"
    L"\r\n"
    L"v2.14.0  2026-09-16  Dark mode\r\n"
    L"- Options: Dark mode. Navy headers and yellow count cells stay; the sheet goes dark.\r\n"
    L"\r\n"
    L"v2.13.0  2026-09-16  Star TSC100 drop slip\r\n"
    L"- Drop slip prints as an 80 mm / 42-column receipt for a Star TSC100 / TSP100 (72 mm print width, 9 pt mono, 3 mm side margins, 11 mm cutter gap).\r\n"
    L"\r\n"
    L"v2.12.0  2026-09-15  Android app\r\n"
    L"- Native Kotlin + Jetpack Compose Android app with the same drop math, yellow counts, number pad, undo, drop slip, named tills, and history.\r\n"
    L"\r\n"
    L"v2.11.0  2026-09-15  ISO C23 implementation\r\n"
    L"- Hosted C23 library and console app: same drop math, $2 bills, undo, named tills, drop slip, and cash-log TSV. No C++.\r\n"
    L"- Visual Studio solution RegisterDropCounter-C23.sln (/std:clatest) and a gcc Makefile.\r\n"
    L"\r\n"
    L"v2.10.0  2026-09-15  iPhone app\r\n"
    L"- Native SwiftUI iPhone app with the same drop math, yellow counts, number pad, undo, drop slip, named tills, and history.\r\n"
    L"- Add the web counter to an iPhone home screen from Safari (Share, Add to Home Screen).\r\n"
    L"\r\n"
    L"v2.9.0  2026-09-15  Named tills\r\n"
    L"- Click a till name (defaults R1-R10) to rename it. Names save with the sheet.\r\n"
    L"\r\n"
    L"v2.8.0  2026-09-15  Undo clear and drop slip\r\n"
    L"- Undo clear brings back the last drawer you cleared, without opening History.\r\n"
    L"- Drop slip prints a one-page bag slip: date, register, drop by denom, total, bag number, and initials.\r\n"
    L"\r\n"
    L"v2.7.1  2026-09-13  Phone app start\r\n"
    L"- Phone layout opens again after the number pad was added.\r\n"
    L"\r\n"
    L"v2.7.0  2026-09-13  Phone pad and arrow keys\r\n"
    L"- Phone and tablet get a number pad (0-9, Clear, Enter) so you can count without the system keyboard covering the yellow cells.\r\n"
    L"- Arrow keys move through count cells the same way as Tab and Enter.\r\n"
    L"\r\n"
    L"v2.6.2  2026-09-13  Roll outline\r\n"
    L"- Black box around the P/N/D/Q coin-roll rows, same as the Excel sheet.\r\n"
    L"\r\n"
    L"v2.6.1  2026-09-13  Windows build fix\r\n"
    L"- Visual Studio compile no longer fails on the OneDrive folder picker.\r\n"
    L"\r\n"
    L"v2.6.0  2026-09-12  OneDrive history sync\r\n"
    L"- Options menu: turn Sync history to OneDrive on or off.\r\n"
    L"- Save, Load, and Sync now use RegisterDropCounter.history in your OneDrive folder so PCs stay together.\r\n"
    L"\r\n"
    L"v2.5.1  2026-09-12  History window layout\r\n"
    L"- History window no longer overlaps. Navy header, date search on its own row, and registers beside denomination counts.\r\n"
    L"\r\n"
    L"v2.5.0  2026-09-12  Remember window\r\n"
    L"- Windows app restores its last screen position, size, and maximized state when you open it again.\r\n"
    L"\r\n"
    L"v2.4.0  2026-09-12  Version number\r\n"
    L"- Visible product version on the header, About tab, and Windows About window.\r\n"
    L"- The version is bumped whenever the counter is updated.\r\n"
    L"\r\n"
    L"v2.3.0  2026-09-09  Enter, resize, GitHub\r\n"
    L"- Enter moves through count cells the same as Tab. Shift+Enter goes back. After $100 it opens the next register.\r\n"
    L"- Windows app reflows the header, grid, cash log, History, and About when you resize the window.\r\n"
    L"- Project is on GitHub at mircsux/RegisterDropCounter.\r\n"
    L"\r\n"
    L"v2.2.0  2026-09-09  About, polish, changelog\r\n"
    L"- About window credits Ronald Robbins Jr and SuperGrok, with how-to instructions.\r\n"
    L"- Show changelog button opens a scrollable list of Excel history, drop fixes, Windows, and the phone layout.\r\n"
    L"- Sunken borders. Unicode arrows and dashes replaced with plain ASCII.\r\n"
    L"- Bold count numbers taken back out.\r\n"
    L"\r\n"
    L"v2.1.0  2026-09-09  History, $2, copy buttons\r\n"
    L"- History: Clear and Clear All save a snapshot. Search by date, restore, or clear the tab.\r\n"
    L"- $2 bills count, drop, and copy in every register and cash-log table.\r\n"
    L"- R1-R10 copy one register. Copy Cash Log, Copy EOD, and Copy Reset copy that table only.\r\n"
    L"- Tab walks the yellow count cells.\r\n"
    L"\r\n"
    L"v2.0.0  2026-09-09  Windows, phone, and web app\r\n"
    L"- Visual Studio 2022 C++ app (RegisterDropCounter.sln) with the same drop math as the spreadsheet.\r\n"
    L"- Works on Android phones in the browser: register chips, large yellow count cells.\r\n"
    L"- Integer-cent engine. The old Excel 5-pennies / MROUND bug is gone.\r\n"
    L"\r\n"
    L"v1.2.0  2026-08-10  Excel workbook\r\n"
    L"- RegisterDropCounter-08102026.xlsx.\r\n"
    L"- Drop and Left columns show counts (integers), not dollar amounts. Amount and the Total row stay currency.\r\n"
    L"\r\n"
    L"v1.1.3  2024-07-21  Excel\r\n"
    L"- Reduced filesize from 13 MB to under 1 MB by removing the 3D spider on the ChangeLog sheet.\r\n"
    L"\r\n"
    L"v1.1.2  2024-07-20  Excel drop fixes\r\n"
    L"- Math drops quarters, nickels, and dimes before coin rolls.\r\n"
    L"- A penny roll still drops before 50 loose pennies. Nobody wants to count out 50 pennies.\r\n"
    L"- Outline around roll columns for visibility.\r\n"
    L"- CopyHack no longer pastes into the ChangeLog. Pastebin must stay internally labelled Sheet2 and hidden. Do not move those sheets; it breaks the copy macros.\r\n"
    L"- Narrowed the 5 pennies bug to MROUND on pennies (fixed later in this app).\r\n"
    L"\r\n"
    L"v1.1.1  2024-07-08  Excel\r\n"
    L"- Added coin rolls to the Drop and Left columns.\r\n"
    L"\r\n"
    L"v1.1.0  2024-07-06  Excel cash log\r\n"
    L"- Register Reset Total turns red when it does not equal the Register Base. Over/under is exactly the drop error.\r\n"
    L"- Dropdown for Register Base so it cannot be set to a junk value.\r\n"
    L"- EOD and Reset copy buttons. Excel cannot copy text-only, so a hidden sheet recopy hack pastes into the national cash log without breaking its format.\r\n"
    L"- Column total routines updated to avoid counting errors.\r\n";

LRESULT CALLBACK AboutProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
    case WM_CREATE: {
      MakeStatic(h, IDC_A_CREDIT, L"Designed by Ronald Robbins Jr and SuperGrok", 16, 12, 590, 24,
                 SS_LEFT);
      SendMessageW(GetDlgItem(h, IDC_A_CREDIT), WM_SETFONT, (WPARAM)gFontBold, TRUE);
      wchar_t ver[80];
      swprintf(ver, 80, L"Version %s", rdc::VersionStamp());
      MakeStatic(h, IDC_A_VER, ver, 16, 36, 590, 20, SS_LEFT);
      SendMessageW(GetDlgItem(h, IDC_A_VER), WM_SETFONT, (WPARAM)gMono, TRUE);
      HWND ed = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", kAboutBody,
                                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | ES_MULTILINE |
                                    ES_READONLY | ES_AUTOVSCROLL,
                                16, 42, 590, 250, h, (HMENU)IDC_A_TEXT, gInst, nullptr);
      SendMessageW(ed, WM_SETFONT, (WPARAM)gFont, TRUE);
      MakeBtn(h, IDC_A_LOGBTN, L"Show changelog", 16, 300, 140, 28);
      HWND log = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", kChangelogBody,
                                 WS_CHILD | WS_VSCROLL | WS_TABSTOP | ES_MULTILINE | ES_READONLY |
                                     ES_AUTOVSCROLL,
                                 16, 336, 590, 220, h, (HMENU)IDC_A_LOG, gInst, nullptr);
      SendMessageW(log, WM_SETFONT, (WPARAM)gMono, TRUE);
      MakeBtn(h, IDC_A_CLOSE, L"Close", 266, 566, 90, 28);
      RelayoutAbout(h);
      return 0;
    }
    case WM_ERASEBKGND: {
      HDC dc = (HDC)w;
      RECT rc;
      GetClientRect(h, &rc);
      FillRect(dc, &rc, Brush(kSheet));
      return 1;
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = (HDC)w;
      SetBkColor(dc, kSheet);
      SetTextColor(dc, kNavyFg);
      return (LRESULT)Brush(kSheet);
    }
    case WM_CTLCOLOREDIT: {
      HDC dc = (HDC)w;
      SetBkColor(dc, kPaper);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kPaper);
    }
    case WM_DRAWITEM:
      DrawThemedItem((DRAWITEMSTRUCT*)l);
      return TRUE;
    case WM_COMMAND:
      if (LOWORD(w) == IDC_A_CLOSE) {
        DestroyWindow(h);
      } else if (LOWORD(w) == IDC_A_LOGBTN) {
        HWND log = GetDlgItem(h, IDC_A_LOG);
        const bool vis = IsWindowVisible(log) != FALSE;
        ShowWindow(log, vis ? SW_HIDE : SW_SHOW);
        gAboutLogOpen = !vis;
        SetDlgItemTextW(h, IDC_A_LOGBTN, vis ? L"Show changelog" : L"Hide changelog");
        RelayoutAbout(h);
      }
      return 0;
    case WM_SIZE:
      if (w != SIZE_MINIMIZED) RelayoutAbout(h);
      return 0;
    case WM_GETMINMAXINFO: {
      auto* m = (MINMAXINFO*)l;
      m->ptMinTrackSize.x = 480;
      m->ptMinTrackSize.y = 400;
      return 0;
    }
    case WM_CLOSE:
      DestroyWindow(h);
      return 0;
    case WM_DESTROY:
      gAbout = nullptr;
      gAboutLogOpen = false;
      return 0;
  }
  return DefWindowProcW(h, msg, w, l);
}

void RelayoutAbout(HWND h) {
  RECT rc{};
  GetClientRect(h, &rc);
  const int W = rc.right;
  const int H = rc.bottom;
  if (W < 200 || H < 160) return;
  const int pad = 16;
  const int btnH = 28;
  const int yClose = H - pad - btnH;
  const int yText = 62;
  int textH;
  if (gAboutLogOpen) {
    const int remain = yClose - yText - btnH - 16;
    textH = rdc::MaxI(80, remain * 42 / 100);
  } else {
    textH = rdc::MaxI(80, yClose - yText - btnH - 20);
  }
  HDWP dwp = BeginDeferWindowPos(8);
  if (!dwp) return;
  Place(&dwp, h, IDC_A_CREDIT, pad, 12, W - pad * 2, 24);
  Place(&dwp, h, IDC_A_VER, pad, 36, W - pad * 2, 20);
  Place(&dwp, h, IDC_A_TEXT, pad, yText, W - pad * 2, textH);
  Place(&dwp, h, IDC_A_LOGBTN, pad, yText + textH + 8, 140, btnH);
  if (gAboutLogOpen)
    Place(&dwp, h, IDC_A_LOG, pad, yText + textH + 8 + btnH + 8, W - pad * 2,
          rdc::MaxI(60, yClose - (yText + textH + 8 + btnH + 16)));
  Place(&dwp, h, IDC_A_CLOSE, (W - 90) / 2, yClose, 90, btnH);
  EndDeferWindowPos(dwp);
}

void ShowAbout(HWND owner) {
  if (gAbout && IsWindow(gAbout)) {
    ShowWindow(gAbout, SW_SHOW);
    SetForegroundWindow(gAbout);
    return;
  }
  static bool registered = false;
  if (!registered) {
    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = AboutProc;
    wc.hInstance = gInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = Brush(kSheet);
    wc.lpszClassName = kAboutClass;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);
    registered = true;
  }
  RECT rc;
  GetWindowRect(owner, &rc);
  gAbout = CreateWindowExW(WS_EX_APPWINDOW, kAboutClass, L"About",
                           WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, rc.left + 80,
                           rc.top + 40, 640, 660, owner, nullptr, gInst, nullptr);
  wchar_t aboutTitle[64];
  swprintf(aboutTitle, 64, L"About  %s", rdc::kAppVersion);
  SetWindowTextW(gAbout, aboutTitle);
  ApplyWinDark(gAbout);
  ThemeChildEdits(gAbout);
  ShowWindow(gAbout, SW_SHOW);
  UpdateWindow(gAbout);
}

constexpr wchar_t kStatsClass[] = L"RegisterDropCounterStatsWnd";
constexpr int IDC_N_CLOSE = 9301;

void DrawTxt(HDC dc, RECT r, const wchar_t* t, COLORREF c, HFONT f, UINT fmt) {
  HFONT old = (HFONT)SelectObject(dc, f);
  SetBkMode(dc, TRANSPARENT);
  SetTextColor(dc, c);
  DrawTextW(dc, t, -1, &r, fmt | DT_NOPREFIX);
  SelectObject(dc, old);
}

void FillRound(HDC dc, RECT r, COLORREF fill, COLORREF edge) {
  HBRUSH b = CreateSolidBrush(fill);
  HPEN p = CreatePen(PS_SOLID, 1, edge);
  HGDIOBJ ob = SelectObject(dc, b);
  HGDIOBJ op = SelectObject(dc, p);
  RoundRect(dc, r.left, r.top, r.right, r.bottom, 12, 12);
  SelectObject(dc, ob);
  SelectObject(dc, op);
  DeleteObject(b);
  DeleteObject(p);
}

void DrawBars(HDC dc, RECT box, const std::vector<rdc::ChartItem>& items, COLORREF fill) {
  if (items.empty()) {
    DrawTxt(dc, box, L"No data yet", kMuted, gFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    return;
  }
  int maxv = 1;
  for (const auto& it : items)
    if (it.cents > maxv) maxv = it.cents;
  const int n = (int)items.size();
  const int padL = 8, padR = 8, padB = 22, padT = 6;
  const int innerW = rdc::MaxI(10, box.right - box.left - padL - padR);
  const int innerH = rdc::MaxI(10, box.bottom - box.top - padB - padT);
  const int slot = innerW / n;
  const int bw = rdc::MaxI(4, slot - 6);
  HBRUSH b = CreateSolidBrush(fill);
  for (int i = 0; i < n; ++i) {
    const int bh = (int)((long long)innerH * items[i].cents / maxv);
    RECT r;
    r.left = box.left + padL + i * slot + (slot - bw) / 2;
    r.right = r.left + bw;
    r.bottom = box.bottom - padB;
    r.top = r.bottom - bh;
    FillRect(dc, &r, b);
    RECT lab{r.left - 4, box.bottom - padB + 2, r.right + 4, box.bottom};
    DrawTxt(dc, lab, items[i].label.c_str(), kMuted, gFont, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
  }
  DeleteObject(b);
}

void DrawLine(HDC dc, RECT box, const std::vector<rdc::ChartItem>& items, COLORREF stroke) {
  if (items.empty()) {
    DrawTxt(dc, box, L"No data yet", kMuted, gFont, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    return;
  }
  int maxv = 1;
  for (const auto& it : items)
    if (it.cents > maxv) maxv = it.cents;
  const int n = (int)items.size();
  const int padL = 10, padR = 10, padB = 22, padT = 8;
  const int innerW = rdc::MaxI(10, box.right - box.left - padL - padR);
  const int innerH = rdc::MaxI(10, box.bottom - box.top - padB - padT);
  std::vector<POINT> pts(n);
  for (int i = 0; i < n; ++i) {
    const int x = box.left + padL + (n == 1 ? innerW / 2 : i * innerW / (n - 1));
    const int y = box.top + padT + innerH - (int)((long long)innerH * items[i].cents / maxv);
    pts[i] = {x, y};
  }
  HPEN pen = CreatePen(PS_SOLID, 2, stroke);
  HGDIOBJ op = SelectObject(dc, pen);
  Polyline(dc, pts.data(), n);
  HBRUSH dot = CreateSolidBrush(stroke);
  HGDIOBJ ob = SelectObject(dc, dot);
  SelectObject(dc, GetStockObject(NULL_PEN));
  for (int i = 0; i < n; ++i) Ellipse(dc, pts[i].x - 3, pts[i].y - 3, pts[i].x + 4, pts[i].y + 4);
  SelectObject(dc, ob);
  SelectObject(dc, op);
  DeleteObject(pen);
  DeleteObject(dot);
  const int step = rdc::MaxI(1, n / 6);
  for (int i = 0; i < n; ++i) {
    if (i != 0 && i != n - 1 && (i % step) != 0) continue;
    RECT lab{pts[i].x - 28, box.bottom - padB + 2, pts[i].x + 28, box.bottom};
    DrawTxt(dc, lab, items[i].label.c_str(), kMuted, gFont, DT_CENTER | DT_SINGLELINE);
  }
}

void PaintStats(HWND h, HDC dc) {
  RECT rc{};
  GetClientRect(h, &rc);
  const int W = rc.right;
  const int H = rc.bottom;
  FillRect(dc, &rc, Brush(kSheet));
  RECT band{0, 0, W, 92};
  FillRect(dc, &band, Brush(kNavy));
  RECT t{20, 14, W - 20, 44};
  DrawTxt(dc, t, L"Stats for Nerds", RGB(255, 255, 255), gFontBold, DT_LEFT | DT_SINGLELINE);
  RECT sub{20, 46, W - 20, 84};
  DrawTxt(dc, sub,
          L"Drop is the cash pulled so each drawer resets to base. Sample fills stay out of History, so they stay off these charts.",
          RGB(220, 240, 236), gFont, DT_LEFT | DT_WORDBREAK);

  auto stats = rdc::BuildNerdStats(gHistory, gNames);
  const int pad = 16;
  const int closeH = 32;
  int y = 104;

  if (stats.clearCount == 0) {
    RECT empty{pad, y, W - pad, H - pad - closeH - 16};
    FillRound(dc, empty, kPaper, RGB(GetRValue(kMuted), GetGValue(kMuted), GetBValue(kMuted)));
    InflateRect(&empty, -16, -16);
    DrawTxt(dc, empty,
            L"History is empty. Count a till, then Clear. Each real clear becomes a point on the charts.",
            kMuted, gFont, DT_LEFT | DT_WORDBREAK);
    return;
  }

  const int tileH = 70;
  const int gap = 10;
  const int tileW = (W - pad * 2 - gap * 3) / 4;
  wchar_t tiles[4][80];
  swprintf(tiles[0], 80, L"Drop in History\n%s", rdc::Money(stats.totalDropCents).c_str());
  swprintf(tiles[1], 80, L"Clears\n%d", stats.clearCount);
  swprintf(tiles[2], 80, L"Average drop\n%s", rdc::Money(stats.avgDropCents).c_str());
  swprintf(tiles[3], 80, L"This week\n%s", rdc::Money(stats.weekDropCents).c_str());
  for (int i = 0; i < 4; ++i) {
    RECT tile{pad + i * (tileW + gap), y, pad + i * (tileW + gap) + tileW, y + tileH};
    FillRound(dc, tile, kPaper, RGB(GetRValue(kMuted), GetGValue(kMuted), GetBValue(kMuted)));
    RECT cap{tile.left + 10, tile.top + 8, tile.right - 10, tile.top + 28};
    RECT val{tile.left + 10, tile.top + 30, tile.right - 10, tile.bottom - 8};
    wchar_t* nl = wcschr(tiles[i], L'\n');
    if (nl) {
      *nl = 0;
      DrawTxt(dc, cap, tiles[i], kMuted, gFont, DT_LEFT | DT_SINGLELINE);
      DrawTxt(dc, val, nl + 1, kNavyFg, gFontBold, DT_LEFT | DT_SINGLELINE);
    }
  }
  y += tileH + 12;

  if (!stats.insights.empty()) {
    const int insH = 22 + (int)stats.insights.size() * 20;
    RECT box{pad, y, W - pad, y + insH};
    FillRound(dc, box, kPaper, RGB(GetRValue(kMuted), GetGValue(kMuted), GetBValue(kMuted)));
    RECT hdg{box.left + 12, box.top + 6, box.right - 12, box.top + 24};
    DrawTxt(dc, hdg, L"What the drops say", kNavyFg, gFontBold, DT_LEFT | DT_SINGLELINE);
    int iy = box.top + 26;
    for (const auto& line : stats.insights) {
      RECT lr{box.left + 12, iy, box.right - 12, iy + 20};
      std::wstring bullet = L"- ";
      bullet += line;
      DrawTxt(dc, lr, bullet.c_str(), kInk, gFont, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
      iy += 20;
    }
    y += insH + 12;
  }

  const int chartBottom = H - pad - closeH - 12;
  const int chartH = rdc::MaxI(80, (chartBottom - y - gap) / 2);
  const int chartW = (W - pad * 2 - gap) / 2;
  struct ChartBox {
    const wchar_t* title;
    const std::vector<rdc::ChartItem>* items;
    bool line;
    COLORREF color;
  };
  ChartBox charts[4] = {
      {L"Drop by day", &stats.byDay, true, kNavyMid},
      {L"Drop by till", &stats.byTill, false, kNavy},
      {L"Weekday mix", &stats.byWeekday, false, kNavyMid},
      {L"What you pull", &stats.byDenom, false, kInput},
  };
  for (int i = 0; i < 4; ++i) {
    const int col = i % 2;
    const int row = i / 2;
    RECT card{pad + col * (chartW + gap), y + row * (chartH + gap),
              pad + col * (chartW + gap) + chartW, y + row * (chartH + gap) + chartH};
    FillRound(dc, card, kPaper, RGB(GetRValue(kMuted), GetGValue(kMuted), GetBValue(kMuted)));
    RECT title{card.left + 12, card.top + 8, card.right - 12, card.top + 28};
    DrawTxt(dc, title, charts[i].title, kNavyFg, gFontBold, DT_LEFT | DT_SINGLELINE);
    RECT plot{card.left + 8, card.top + 30, card.right - 8, card.bottom - 8};
    if (charts[i].line)
      DrawLine(dc, plot, *charts[i].items, charts[i].color);
    else
      DrawBars(dc, plot, *charts[i].items, charts[i].color);
  }
}

LRESULT CALLBACK StatsProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
    case WM_CREATE:
      MakeBtn(h, IDC_N_CLOSE, L"Close", 20, 20, 90, 28);
      return 0;
    case WM_ERASEBKGND:
      return 1;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC dc = BeginPaint(h, &ps);
      RECT rc;
      GetClientRect(h, &rc);
      HDC mem = CreateCompatibleDC(dc);
      HBITMAP bm = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
      HGDIOBJ old = SelectObject(mem, bm);
      PaintStats(h, mem);
      BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
      SelectObject(mem, old);
      DeleteObject(bm);
      DeleteDC(mem);
      EndPaint(h, &ps);
      return 0;
    }
    case WM_SIZE: {
      RECT rc;
      GetClientRect(h, &rc);
      HWND btn = GetDlgItem(h, IDC_N_CLOSE);
      if (btn)
        ::SetWindowPos(btn, nullptr, (rc.right - 90) / 2, rc.bottom - 16 - 28, 90, 28, SWP_NOZORDER);
      InvalidateRect(h, nullptr, FALSE);
      return 0;
    }
    case WM_DRAWITEM:
      DrawThemedItem((DRAWITEMSTRUCT*)l);
      return TRUE;
    case WM_COMMAND:
      if (LOWORD(w) == IDC_N_CLOSE) DestroyWindow(h);
      return 0;
    case WM_GETMINMAXINFO: {
      auto* m = (MINMAXINFO*)l;
      m->ptMinTrackSize.x = 720;
      m->ptMinTrackSize.y = 560;
      return 0;
    }
    case WM_CLOSE:
      DestroyWindow(h);
      return 0;
    case WM_DESTROY:
      gStats = nullptr;
      return 0;
  }
  return DefWindowProcW(h, msg, w, l);
}

void ShowStats(HWND owner) {
  if (gStats && IsWindow(gStats)) {
    InvalidateRect(gStats, nullptr, FALSE);
    ShowWindow(gStats, SW_SHOW);
    SetForegroundWindow(gStats);
    return;
  }
  static bool registered = false;
  if (!registered) {
    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = StatsProc;
    wc.hInstance = gInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = Brush(kSheet);
    wc.lpszClassName = kStatsClass;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);
    registered = true;
  }
  RECT rc;
  GetWindowRect(owner, &rc);
  gStats = CreateWindowExW(WS_EX_APPWINDOW, kStatsClass, L"Stats for Nerds",
                           WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, rc.left + 40, rc.top + 30, 1000,
                           740, owner, nullptr, gInst, nullptr);
  wchar_t title[80];
  swprintf(title, 80, L"Stats for Nerds  %s", rdc::kAppVersion);
  SetWindowTextW(gStats, title);
  ApplyWinDark(gStats);
  ShowWindow(gStats, SW_SHOW);
  UpdateWindow(gStats);
}

constexpr wchar_t kOptClass[] = L"RegisterDropCounterOptionsWnd";
constexpr int IDC_O_TITLE = 9201;
constexpr int IDC_O_SYNC = 9202;
constexpr int IDC_O_HELP = 9203;
constexpr int IDC_O_PATHLBL = 9204;
constexpr int IDC_O_PATH = 9205;
constexpr int IDC_O_BROWSE = 9206;
constexpr int IDC_O_SAVE = 9207;
constexpr int IDC_O_LOAD = 9208;
constexpr int IDC_O_SYNCNOW = 9209;
constexpr int IDC_O_STATUS = 9210;
constexpr int IDC_O_CLOSE = 9211;
constexpr int IDC_O_DARK = 9212;

const wchar_t kOptHelp[] =
    L"When this is on, Clear and Clear All also write snapshots to a file in your "
    L"OneDrive folder. OneDrive copies that file to your other PCs.\r\n\r\n"
    L"On another PC, turn the same option on and click Sync now to pull those snapshots "
    L"in. Load reads OneDrive into this PC. Save writes this PC's history out.\r\n\r\n"
    L"The file is RegisterDropCounter.history inside the folder below. Leave the folder "
    L"blank to use OneDrive\\RegisterDropCounter automatically.";

void RefreshOptionsUi(HWND h) {
  SendMessageW(GetDlgItem(h, IDC_O_DARK), BM_SETCHECK,
               gOptions.darkMode ? BST_CHECKED : BST_UNCHECKED, 0);
  SendMessageW(GetDlgItem(h, IDC_O_SYNC), BM_SETCHECK,
               gOptions.syncOneDrive ? BST_CHECKED : BST_UNCHECKED, 0);
  std::wstring folder = rdc::ResolvedSyncFolder(gOptions);
  if (folder.empty()) folder = gOptions.folder;
  SetDlgItemTextW(h, IDC_O_PATH, folder.c_str());
  std::wstring st;
  if (!gOptions.syncOneDrive) {
    st = L"OneDrive sync is off. History stays on this PC.";
  } else if (rdc::ResolvedSyncFolder(gOptions).empty()) {
    st = L"OneDrive folder not found. Choose a folder, or install the OneDrive app.";
  } else {
    st = L"Sync is on. File: RegisterDropCounter.history";
  }
  SetDlgItemTextW(h, IDC_O_STATUS, st.c_str());
}

bool BrowseForFolder(HWND owner, std::wstring* out) {
  BROWSEINFOW bi{};
  bi.hwndOwner = owner;
  bi.lpszTitle = L"Choose OneDrive folder for history";
  bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
  LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
  if (!pidl) return false;
  wchar_t path[MAX_PATH];
  const BOOL ok = SHGetPathFromIDListW(pidl, path);
  CoTaskMemFree(pidl);
  if (!ok || !path[0]) return false;
  *out = path;
  return true;
}

void RelayoutOptions(HWND h) {
  RECT rc{};
  GetClientRect(h, &rc);
  const int W = rc.right;
  const int H = rc.bottom;
  if (W < 240 || H < 180) return;
  const int pad = 16;
  const int btnH = 28;
  HDWP dwp = BeginDeferWindowPos(13);
  if (!dwp) return;
  Place(&dwp, h, IDC_O_TITLE, pad, 10, W - pad * 2, 24);
  Place(&dwp, h, IDC_O_DARK, pad, 52, W - pad * 2, 24);
  Place(&dwp, h, IDC_O_SYNC, pad, 80, W - pad * 2, 24);
  Place(&dwp, h, IDC_O_HELP, pad, 112, W - pad * 2, 116);
  Place(&dwp, h, IDC_O_PATHLBL, pad, 236, 80, 22);
  Place(&dwp, h, IDC_O_PATH, pad + 84, 232, W - pad * 2 - 84 - 128, 28);
  Place(&dwp, h, IDC_O_BROWSE, W - pad - 120, 232, 120, 28);
  const int yAct = 276;
  Place(&dwp, h, IDC_O_SAVE, pad, yAct, 150, btnH);
  Place(&dwp, h, IDC_O_LOAD, pad + 158, yAct, 160, btnH);
  Place(&dwp, h, IDC_O_SYNCNOW, pad + 326, yAct, 100, btnH);
  Place(&dwp, h, IDC_O_STATUS, pad, yAct + 40, W - pad * 2, 40);
  Place(&dwp, h, IDC_O_CLOSE, (W - 90) / 2, H - pad - btnH, 90, btnH);
  EndDeferWindowPos(dwp);
}

LRESULT CALLBACK OptProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
    case WM_CREATE: {
      MakeStatic(h, IDC_O_TITLE, L"Options", 16, 10, 400, 24, SS_LEFT);
      SendMessageW(GetDlgItem(h, IDC_O_TITLE), WM_SETFONT, (WPARAM)gFontBold, TRUE);
      HWND dark = CreateWindowW(L"BUTTON", L"Dark mode",
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 16, 52, 400,
                                24, h, (HMENU)(INT_PTR)IDC_O_DARK, gInst, nullptr);
      SendMessageW(dark, WM_SETFONT, (WPARAM)gFont, TRUE);
      Untheme(dark);
      HWND chk = CreateWindowW(L"BUTTON", L"Sync history to OneDrive",
                               WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 16, 80, 400,
                               24, h, (HMENU)(INT_PTR)IDC_O_SYNC, gInst, nullptr);
      SendMessageW(chk, WM_SETFONT, (WPARAM)gFont, TRUE);
      Untheme(chk);
      HWND help = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", kOptHelp,
                                  WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | ES_MULTILINE |
                                      ES_READONLY | ES_AUTOVSCROLL,
                                  16, 112, 590, 116, h, (HMENU)(INT_PTR)IDC_O_HELP, gInst, nullptr);
      SendMessageW(help, WM_SETFONT, (WPARAM)gFont, TRUE);
      MakeStatic(h, IDC_O_PATHLBL, L"Folder", 16, 236, 80, 22, SS_LEFT);
      MakeEditText(h, IDC_O_PATH, 100, 232, 380, 28);
      MakeBtn(h, IDC_O_BROWSE, L"Choose folder", 490, 232, 120, 28);
      MakeBtn(h, IDC_O_SAVE, L"Save to OneDrive", 16, 276, 150, 28);
      MakeBtn(h, IDC_O_LOAD, L"Load from OneDrive", 174, 276, 160, 28);
      MakeBtn(h, IDC_O_SYNCNOW, L"Sync now", 342, 276, 100, 28);
      MakeStatic(h, IDC_O_STATUS, L"", 16, 316, 590, 40, SS_LEFT);
      MakeBtn(h, IDC_O_CLOSE, L"Close", 266, 380, 90, 28);
      RefreshOptionsUi(h);
      RelayoutOptions(h);
      return 0;
    }
    case WM_ERASEBKGND: {
      HDC dc = (HDC)w;
      RECT rc;
      GetClientRect(h, &rc);
      RECT band{0, 0, rc.right, 44};
      FillRect(dc, &band, Brush(kNavy));
      RECT rest{0, 44, rc.right, rc.bottom};
      FillRect(dc, &rest, Brush(kSheet));
      return 1;
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = (HDC)w;
      int id = GetDlgCtrlID((HWND)l);
      if (id == IDC_O_TITLE) {
        SetBkColor(dc, kNavy);
        SetTextColor(dc, kOnNavy);
        return (LRESULT)Brush(kNavy);
      }
      SetBkColor(dc, kSheet);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kSheet);
    }
    case WM_CTLCOLOREDIT: {
      HDC dc = (HDC)w;
      SetBkColor(dc, kPaper);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kPaper);
    }
    case WM_CTLCOLORBTN: {
      HDC dc = (HDC)w;
      SetBkColor(dc, kSheet);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kSheet);
    }
    case WM_DRAWITEM:
      DrawThemedItem((DRAWITEMSTRUCT*)l);
      return TRUE;
    case WM_COMMAND: {
      int id = LOWORD(w);
      int code = HIWORD(w);
      if (id == IDC_O_DARK) {
        gOptions.darkMode = SendMessageW((HWND)l, BM_GETCHECK, 0, 0) == BST_CHECKED;
        PersistOptions();
        ApplyTheme();
      } else if (id == IDC_O_SYNC) {
        gOptions.syncOneDrive = SendMessageW((HWND)l, BM_GETCHECK, 0, 0) == BST_CHECKED;
        PersistOptions();
        if (gOptions.syncOneDrive) {
          std::wstring msg = SyncHistoryNow();
          SetDlgItemTextW(h, IDC_O_STATUS, msg.c_str());
          if (gHist && IsWindow(gHist)) RefreshHistoryUi(gHist);
          if (gStats && IsWindow(gStats)) InvalidateRect(gStats, nullptr, FALSE);
        } else {
          RefreshOptionsUi(h);
        }
      } else if (id == IDC_O_BROWSE) {
        std::wstring dir;
        if (BrowseForFolder(h, &dir)) {
          gOptions.folder = dir;
          PersistOptions();
          RefreshOptionsUi(h);
        }
      } else if (id == IDC_O_PATH && code == EN_KILLFOCUS) {
        wchar_t buf[MAX_PATH];
        GetDlgItemTextW(h, IDC_O_PATH, buf, MAX_PATH);
        gOptions.folder = buf;
        PersistOptions();
      } else if (id == IDC_O_SAVE) {
        if (!gOptions.syncOneDrive) {
          SetDlgItemTextW(h, IDC_O_STATUS, L"Turn OneDrive sync on first.");
        } else if (SaveHistoryToCloud()) {
          SetDlgItemTextW(h, IDC_O_STATUS, L"Saved history to OneDrive.");
        } else {
          SetDlgItemTextW(h, IDC_O_STATUS, L"Could not save. Check the folder.");
        }
      } else if (id == IDC_O_LOAD) {
        if (!gOptions.syncOneDrive) {
          SetDlgItemTextW(h, IDC_O_STATUS, L"Turn OneDrive sync on first.");
        } else if (LoadHistoryFromCloud()) {
          wchar_t msg[80];
          swprintf(msg, 80, L"Loaded and merged %d snapshot(s).", (int)gHistory.size());
          SetDlgItemTextW(h, IDC_O_STATUS, msg);
          if (gHist && IsWindow(gHist)) RefreshHistoryUi(gHist);
          if (gStats && IsWindow(gStats)) InvalidateRect(gStats, nullptr, FALSE);
        } else {
          SetDlgItemTextW(h, IDC_O_STATUS, L"No history file in that OneDrive folder yet.");
        }
      } else if (id == IDC_O_SYNCNOW) {
        std::wstring msg = SyncHistoryNow();
        SetDlgItemTextW(h, IDC_O_STATUS, msg.c_str());
        if (gHist && IsWindow(gHist)) RefreshHistoryUi(gHist);
      } else if (id == IDC_O_CLOSE) {
        DestroyWindow(h);
      }
      return 0;
    }
    case WM_SIZE:
      if (w != SIZE_MINIMIZED) RelayoutOptions(h);
      return 0;
    case WM_GETMINMAXINFO: {
      auto* m = (MINMAXINFO*)l;
      m->ptMinTrackSize.x = 560;
      m->ptMinTrackSize.y = 420;
      return 0;
    }
    case WM_CLOSE:
      DestroyWindow(h);
      return 0;
    case WM_DESTROY:
      gOpt = nullptr;
      return 0;
  }
  return DefWindowProcW(h, msg, w, l);
}

void ShowOptions(HWND owner) {
  if (gOpt && IsWindow(gOpt)) {
    ShowWindow(gOpt, SW_SHOW);
    SetForegroundWindow(gOpt);
    RefreshOptionsUi(gOpt);
    return;
  }
  static bool registered = false;
  if (!registered) {
    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = OptProc;
    wc.hInstance = gInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = Brush(kSheet);
    wc.lpszClassName = kOptClass;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);
    registered = true;
  }
  RECT rc;
  GetWindowRect(owner, &rc);
  gOpt = CreateWindowExW(WS_EX_APPWINDOW, kOptClass, L"Options",
                         WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, rc.left + 60, rc.top + 60, 680, 460,
                         owner, nullptr, gInst, nullptr);
  ApplyWinDark(gOpt);
  ThemeChildEdits(gOpt);
  ShowWindow(gOpt, SW_SHOW);
  UpdateWindow(gOpt);
}

void EnableUndoBtn(HWND h);

void UndoLastClear(HWND h) {
  if (gHistory.empty()) {
    Status(h, L"Nothing to undo. History is empty.");
    return;
  }
  const rdc::HistoryEntry& e = gHistory[0];
  if (e.kind == rdc::HistRegister && e.registerIndex >= 0 &&
      e.registerIndex < rdc::kRegisterCount) {
    gRegs[e.registerIndex] = e.registers[e.registerIndex];
    gActive = e.registerIndex;
    TabCtrl_SetCurSel(GetDlgItem(h, IDC_TABS), gActive);
  } else {
    gBase = e.base;
    for (int i = 0; i < rdc::kRegisterCount; ++i) gRegs[i] = e.registers[i];
    HWND combo = GetDlgItem(h, IDC_BASE);
    for (int b = 0; b < 5; ++b)
      if (rdc::kBaseOptions[b] == gBase) SendMessageW(combo, CB_SETCURSEL, b, 0);
  }
  gSampleScratch = false;
  LoadCountsIntoEdits(h);
  SaveState();
  EnableUndoBtn(h);
  wchar_t msg[80];
  if (e.kind == rdc::HistRegister)
    swprintf(msg, 80, L"Brought back %s.", gNames[e.registerIndex].c_str());
  else
    swprintf(msg, 80, L"Brought back all registers from Clear All.");
  Status(h, msg);
}

std::wstring BuildSlipText(int reg, const wchar_t* bag, const wchar_t* initials) {
  if (reg < 0 || reg >= rdc::kRegisterCount) reg = 0;
  return rdc::BuildReceiptSlip(gNames[reg].c_str(), gBase, rdc::ComputeRegister(gRegs[reg], gBase),
                               bag, initials);
}

void PrintSlipText(HWND owner, const std::wstring& body) {
  PRINTDLGW pd{};
  pd.lStructSize = sizeof(pd);
  pd.hwndOwner = owner;
  pd.Flags = PD_RETURNDC | PD_NOSELECTION | PD_NOPAGENUMS;
  if (!PrintDlgW(&pd) || !pd.hDC) return;

  if (pd.hDevMode) {
    DEVMODEW* dm = static_cast<DEVMODEW*>(GlobalLock(pd.hDevMode));
    if (dm) {
      dm->dmFields |= DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERWIDTH | DM_PAPERLENGTH;
      dm->dmOrientation = DMORIENT_PORTRAIT;
      dm->dmPaperSize = DMPAPER_USER;
      dm->dmPaperWidth = 800;    // 80.0 mm, tenths of a millimeter
      dm->dmPaperLength = 2000;  // 200 mm; Star driver auto-cuts
      HDC reset = ResetDCW(pd.hDC, dm);
      if (reset) pd.hDC = reset;
      GlobalUnlock(pd.hDevMode);
    }
  }

  DOCINFOW di{};
  di.cbSize = sizeof(di);
  di.lpszDocName = L"Register Drop Slip";
  if (StartDocW(pd.hDC, &di) > 0) {
    StartPage(pd.hDC);
    const int dpiX = GetDeviceCaps(pd.hDC, LOGPIXELSX);
    const int dpiY = GetDeviceCaps(pd.hDC, LOGPIXELSY);
    const int horz = GetDeviceCaps(pd.hDC, HORZRES);
    const int vert = GetDeviceCaps(pd.hDC, VERTRES);
    const int mm72 = MulDiv(720, dpiX, 254);
    const int mm3 = MulDiv(30, dpiX, 254);
    const int mm3y = MulDiv(30, dpiY, 254);
    const int mm11 = MulDiv(110, dpiY, 254);
    int contentW = mm72;
    if (contentW > horz) contentW = horz;
    int left = 0;
    if (horz > mm72 + mm3) {
      left = mm3;
      contentW = mm72;
    }
    int top = (vert > mm72) ? mm3y : 0;
    int bottom = vert - mm11;
    if (bottom <= top + 40) bottom = vert;

    HFONT font = nullptr;
    HGDIOBJ old = nullptr;
    for (int pt = 9; pt >= 6; --pt) {
      if (font) {
        SelectObject(pd.hDC, old);
        DeleteObject(font);
      }
      font = CreateFontW(-MulDiv(pt, dpiY, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                         DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                         NONANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
      old = SelectObject(pd.hDC, font);
      SIZE sz{};
      GetTextExtentPoint32W(pd.hDC, L"012345678901234567890123456789012345678901",
                            rdc::kReceiptCols, &sz);
      if (sz.cx <= contentW) break;
    }

    SetBkMode(pd.hDC, TRANSPARENT);
    SetTextColor(pd.hDC, RGB(0, 0, 0));
    SetTextAlign(pd.hDC, TA_LEFT | TA_TOP);
    TEXTMETRICW tm{};
    GetTextMetricsW(pd.hDC, &tm);
    const int lineH = tm.tmHeight + tm.tmExternalLeading;
    int y = top;
    size_t start = 0;
    while (start <= body.size() && y + lineH <= bottom) {
      size_t end = body.find(L'\n', start);
      if (end == std::wstring::npos) end = body.size();
      std::wstring line = body.substr(start, end - start);
      if (!line.empty() && line.back() == L'\r') line.pop_back();
      if ((int)line.size() > rdc::kReceiptCols) line.resize((size_t)rdc::kReceiptCols);
      if (!line.empty()) TextOutW(pd.hDC, left, y, line.c_str(), (int)line.size());
      y += lineH;
      if (end == body.size()) break;
      start = end + 1;
    }
    if (old) SelectObject(pd.hDC, old);
    if (font) DeleteObject(font);
    EndPage(pd.hDC);
    EndDoc(pd.hDC);
  }
  DeleteDC(pd.hDC);
  if (pd.hDevMode) GlobalFree(pd.hDevMode);
  if (pd.hDevNames) GlobalFree(pd.hDevNames);
}

constexpr wchar_t kSlipClass[] = L"RegisterDropCounterSlipWnd";
constexpr int IDC_S_TITLE = 9401;
constexpr int IDC_S_META = 9402;
constexpr int IDC_S_BAGLBL = 9403;
constexpr int IDC_S_BAG = 9404;
constexpr int IDC_S_INITLBL = 9405;
constexpr int IDC_S_INIT = 9406;
constexpr int IDC_S_BODY = 9407;
constexpr int IDC_S_PRINT = 9408;
constexpr int IDC_S_COPY = 9409;
constexpr int IDC_S_CLOSE = 9410;

void RefreshSlipBody(HWND h) {
  wchar_t bag[64]{};
  wchar_t initials[32]{};
  GetDlgItemTextW(h, IDC_S_BAG, bag, 64);
  GetDlgItemTextW(h, IDC_S_INIT, initials, 32);
  gSlipBag = bag;
  gSlipInitials = initials;
  SetDlgItemTextW(h, IDC_S_BODY, BuildSlipText(gSlipReg, bag, initials).c_str());
}

LRESULT CALLBACK SlipProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
    case WM_CREATE: {
      wchar_t title[40];
      swprintf(title, 40, L"Drop slip  R%d", gSlipReg + 1);
      MakeStatic(h, IDC_S_TITLE, title, 16, 10, 400, 24, SS_LEFT);
      SendMessageW(GetDlgItem(h, IDC_S_TITLE), WM_SETFONT, (WPARAM)gFontBold, TRUE);
      wchar_t meta[96];
      swprintf(meta, 96, L"%s   Star TSC100  80 mm / 42 col", rdc::FormatSheetDate().c_str());
      MakeStatic(h, IDC_S_META, meta, 16, 40, 500, 20, SS_LEFT);
      MakeStatic(h, IDC_S_BAGLBL, L"Bag / seal #", 16, 72, 100, 22, SS_LEFT);
      MakeEditText(h, IDC_S_BAG, 120, 68, 200, 28);
      SetDlgItemTextW(h, IDC_S_BAG, gSlipBag.c_str());
      MakeStatic(h, IDC_S_INITLBL, L"Initials", 340, 72, 70, 22, SS_LEFT);
      MakeEditText(h, IDC_S_INIT, 414, 68, 120, 28);
      SetDlgItemTextW(h, IDC_S_INIT, gSlipInitials.c_str());
      HWND body = CreateWindowExW(
          WS_EX_CLIENTEDGE, L"EDIT", L"",
          WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | ES_MULTILINE | ES_READONLY |
              ES_AUTOVSCROLL,
          16, 108, 518, 260, h, (HMENU)(INT_PTR)IDC_S_BODY, gInst, nullptr);
      SendMessageW(body, WM_SETFONT, (WPARAM)gMono, TRUE);
      MakeBtn(h, IDC_S_PRINT, L"Print", 16, 380, 100, 28);
      MakeBtn(h, IDC_S_COPY, L"Copy", 124, 380, 90, 28);
      MakeBtn(h, IDC_S_CLOSE, L"Close", 444, 380, 90, 28);
      RefreshSlipBody(h);
      return 0;
    }
    case WM_ERASEBKGND: {
      HDC dc = (HDC)w;
      RECT rc;
      GetClientRect(h, &rc);
      RECT band{0, 0, rc.right, 44};
      FillRect(dc, &band, Brush(kNavy));
      RECT rest{0, 44, rc.right, rc.bottom};
      FillRect(dc, &rest, Brush(kSheet));
      return 1;
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = (HDC)w;
      int id = GetDlgCtrlID((HWND)l);
      if (id == IDC_S_TITLE) {
        SetBkColor(dc, kNavy);
        SetTextColor(dc, kOnNavy);
        return (LRESULT)Brush(kNavy);
      }
      SetBkColor(dc, kSheet);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kSheet);
    }
    case WM_CTLCOLOREDIT: {
      HDC dc = (HDC)w;
      SetBkColor(dc, kPaper);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kPaper);
    }
    case WM_DRAWITEM:
      DrawThemedItem((DRAWITEMSTRUCT*)l);
      return TRUE;
    case WM_COMMAND: {
      int id = LOWORD(w);
      int code = HIWORD(w);
      if (id == IDC_S_CLOSE) {
        PersistSlipFields();
        DestroyWindow(h);
        return 0;
      }
      if (id == IDC_S_COPY) {
        RefreshSlipBody(h);
        CopyText(BuildSlipText(gSlipReg, gSlipBag.c_str(), gSlipInitials.c_str()));
        return 0;
      }
      if (id == IDC_S_PRINT) {
        RefreshSlipBody(h);
        PersistSlipFields();
        PrintSlipText(h, BuildSlipText(gSlipReg, gSlipBag.c_str(), gSlipInitials.c_str()));
        return 0;
      }
      if (code == EN_CHANGE && (id == IDC_S_BAG || id == IDC_S_INIT)) {
        RefreshSlipBody(h);
        return 0;
      }
      break;
    }
    case WM_SIZE:
      if (w != SIZE_MINIMIZED) {
        RECT rc;
        GetClientRect(h, &rc);
        const int W = rc.right;
        const int H = rc.bottom;
        HDWP dwp = BeginDeferWindowPos(10);
        if (dwp) {
          Place(&dwp, h, IDC_S_TITLE, 16, 10, W - 32, 24);
          Place(&dwp, h, IDC_S_META, 16, 40, W - 32, 20);
          Place(&dwp, h, IDC_S_BAGLBL, 16, 72, 100, 22);
          Place(&dwp, h, IDC_S_BAG, 120, 68, rdc::MaxI(120, W / 2 - 140), 28);
          Place(&dwp, h, IDC_S_INITLBL, W / 2 + 8, 72, 70, 22);
          Place(&dwp, h, IDC_S_INIT, W / 2 + 82, 68, rdc::MaxI(80, W / 2 - 100), 28);
          Place(&dwp, h, IDC_S_BODY, 16, 108, W - 32, rdc::MaxI(80, H - 160));
          Place(&dwp, h, IDC_S_PRINT, 16, H - 44, 100, 28);
          Place(&dwp, h, IDC_S_COPY, 124, H - 44, 90, 28);
          Place(&dwp, h, IDC_S_CLOSE, W - 106, H - 44, 90, 28);
          EndDeferWindowPos(dwp);
        }
      }
      return 0;
    case WM_CLOSE:
      PersistSlipFields();
      DestroyWindow(h);
      return 0;
    case WM_DESTROY:
      gSlip = nullptr;
      return 0;
  }
  return DefWindowProcW(h, msg, w, l);
}

void ShowSlip(HWND owner, int reg) {
  gSlipReg = reg;
  if (gSlip && IsWindow(gSlip)) {
    wchar_t title[40];
    swprintf(title, 40, L"Drop slip  R%d", gSlipReg + 1);
    SetWindowTextW(gSlip, title);
    SetDlgItemTextW(gSlip, IDC_S_TITLE, title);
    RefreshSlipBody(gSlip);
    ShowWindow(gSlip, SW_SHOW);
    SetForegroundWindow(gSlip);
    return;
  }
  static bool registered = false;
  if (!registered) {
    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = SlipProc;
    wc.hInstance = gInst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = Brush(kSheet);
    wc.lpszClassName = kSlipClass;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClassExW(&wc);
    registered = true;
  }
  RECT rc;
  GetWindowRect(owner, &rc);
  wchar_t title[40];
  swprintf(title, 40, L"Drop slip  R%d", gSlipReg + 1);
  gSlip = CreateWindowExW(WS_EX_APPWINDOW, kSlipClass, title, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                          rc.left + 50, rc.top + 50, 560, 470, owner, nullptr, gInst, nullptr);
  ApplyWinDark(gSlip);
  ThemeChildEdits(gSlip);
  ShowWindow(gSlip, SW_SHOW);
  UpdateWindow(gSlip);
}

void ApplyTabNames(HWND h) {
  HWND tabs = GetDlgItem(h, IDC_TABS);
  if (!tabs) return;
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    wchar_t buf[24];
    swprintf(buf, 24, L"%s", gNames[i].c_str());
    TCITEMW it{};
    it.mask = TCIF_TEXT;
    it.pszText = buf;
    TabCtrl_SetItem(tabs, i, &it);
  }
}

void HideTillEdit(HWND h) {
  HWND e = GetDlgItem(h, IDC_TAB_EDIT);
  if (e) ShowWindow(e, SW_HIDE);
}

void CommitTillEdit(HWND h) {
  HWND e = GetDlgItem(h, IDC_TAB_EDIT);
  if (!e || !IsWindowVisible(e)) return;
  wchar_t buf[32]{};
  GetWindowTextW(e, buf, 32);
  ShowWindow(e, SW_HIDE);
  gNames[gActive] = SanitizeTillName(buf, gActive);
  PersistTillNames();
  ApplyTabNames(h);
}

void CancelTillEdit(HWND h) { HideTillEdit(h); }

void StartTillEdit(HWND h, int i) {
  HWND tabs = GetDlgItem(h, IDC_TABS);
  HWND e = GetDlgItem(h, IDC_TAB_EDIT);
  if (!tabs || !e || i < 0 || i >= rdc::kRegisterCount) return;
  RECT rc{};
  if (!TabCtrl_GetItemRect(tabs, i, &rc)) return;
  MapWindowPoints(tabs, h, (POINT*)&rc, 2);
  int tw = rc.right - rc.left;
  int th = rc.bottom - rc.top;
  if (tw < 40) tw = 40;
  if (th < 20) th = 20;
  ::SetWindowPos(e, HWND_TOP, rc.left, rc.top, tw, th, SWP_SHOWWINDOW);
  SetWindowTextW(e, gNames[i].c_str());
  SetFocus(e);
  SendMessageW(e, EM_SETSEL, 0, -1);
}

void Layout(HWND h) {
  MakeStatic(h, IDC_TITLE, L"Register Drop Counter", 16, 12, 280, 24, SS_LEFT);
  SendMessageW(GetDlgItem(h, IDC_TITLE), WM_SETFONT, (WPARAM)gFontBold, TRUE);
  MakeStatic(h, IDC_BASELBL, L"Register Base", 400, 14, 100, 20, SS_RIGHT);
  HWND combo = CreateWindowW(L"COMBOBOX", L"",
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST |
                                 CBS_OWNERDRAWFIXED | CBS_HASSTRINGS,
                             508, 10, 110, 200, h, (HMENU)IDC_BASE, gInst, nullptr);
  SendMessageW(combo, WM_SETFONT, (WPARAM)gFont, TRUE);
  Untheme(combo);
  for (int b : rdc::kBaseOptions) {
    wchar_t buf[16];
    swprintf(buf, 16, L"$%d.00", b);
    int idx = (int)SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)buf);
    if (b == gBase) SendMessageW(combo, CB_SETCURSEL, idx, 0);
  }
  SendMessageW(combo, CB_SETITEMHEIGHT, (WPARAM)-1, 22);
  SendMessageW(combo, CB_SETITEMHEIGHT, 0, 22);
  MakeBtn(h, IDC_CLEAR, L"Clear All", 630, 10, 90, 26);
  MakeBtn(h, IDC_UNDO, L"Undo clear", 836, 10, 92, 26);
  MakeStatic(h, IDC_TODAY, rdc::FormatSheetDate().c_str(), 1004, 14, 370, 20, SS_RIGHT);
  SendMessageW(GetDlgItem(h, IDC_TODAY), WM_SETFONT, (WPARAM)gFontBold, TRUE);

  HWND tabs = CreateWindowExW(0, WC_TABCONTROLW, L"",
                              WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_OWNERDRAWFIXED |
                                  TCS_TABS | TCS_FIXEDWIDTH | TCS_FOCUSNEVER,
                              12, 48, 500, 26, h, (HMENU)IDC_TABS, gInst, nullptr);
  SendMessageW(tabs, WM_SETFONT, (WPARAM)gFont, TRUE);
  Untheme(tabs);
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    wchar_t lab[16];
    swprintf(lab, 16, L"R%d", i + 1);
    TCITEMW it{};
    it.mask = TCIF_TEXT;
    it.pszText = lab;
    TabCtrl_InsertItem(tabs, i, &it);
  }
  TabCtrl_SetItemSize(tabs, 50, 24);
  MakeBtn(h, IDC_SLIP, L"Drop slip", 520, 48, 76, 26);
  MakeBtn(h, IDC_CLEAR_REG, L"Clear", 604, 48, 64, 26);
  MakeBtn(h, IDC_COPY_R, L"Copy R1", 676, 48, 80, 26);
  HWND tillEdit = MakeEditText(h, IDC_TAB_EDIT, 0, 0, 80, 24);
  ShowWindow(tillEdit, SW_HIDE);

  const int x0 = 16, y0 = 92, rowH = 24;
  const wchar_t* hdrs[] = {L"", L"Count", L"Denom", L"Amount", L"Drop", L"Left"};
  const int hx[] = {16, 56, 136, 216, 316, 396};
  const int hw[] = {36, 70, 70, 90, 70, 70};
  for (int i = 0; i < 6; ++i)
    MakeStatic(h, IDC_HDR0 + i, hdrs[i], hx[i], 76, hw[i], 16);

  for (int i = 0; i < rdc::DenomCount; ++i) {
    int y = y0 + i * rowH;
    MakeStatic(h, IDC_BADGE0 + i, rdc::kRollLetter[i], x0, y, 36, 22);
    MakeEdit(h, IDC_COUNT0 + i, 56, y, 70, 22);
    MakeStatic(h, IDC_DENOM0 + i, rdc::kLabels[i], 136, y, 70, 22);
    MakeStatic(h, IDC_AMT0 + i, L"$0.00", 216, y, 90, 22, SS_RIGHT);
    MakeStatic(h, IDC_DROP0 + i, L"0", 316, y, 70, 22);
    MakeStatic(h, IDC_LEFT0 + i, L"0", 396, y, 70, 22);
    SendMessageW(GetDlgItem(h, IDC_AMT0 + i), WM_SETFONT, (WPARAM)gMono, TRUE);
    SendMessageW(GetDlgItem(h, IDC_DROP0 + i), WM_SETFONT, (WPARAM)gMono, TRUE);
    SendMessageW(GetDlgItem(h, IDC_LEFT0 + i), WM_SETFONT, (WPARAM)gMono, TRUE);
    SendMessageW(GetDlgItem(h, IDC_DENOM0 + i), WM_SETFONT, (WPARAM)gMono, TRUE);
  }
  int ty = y0 + rdc::DenomCount * rowH + 6;
  MakeStatic(h, IDC_TOTAL_LBL, L"Total", 56, ty, 150, 22, SS_LEFT);
  MakeStatic(h, IDC_TOTAL_AMT, L"$0.00", 216, ty, 90, 22, SS_RIGHT);
  MakeStatic(h, IDC_TOTAL_DROP, L"$0.00", 316, ty, 70, 22);
  MakeStatic(h, IDC_TOTAL_LEFT, L"$0.00", 396, ty, 90, 22);
  SendMessageW(GetDlgItem(h, IDC_TOTAL_AMT), WM_SETFONT, (WPARAM)gFontBold, TRUE);
  SendMessageW(GetDlgItem(h, IDC_TOTAL_DROP), WM_SETFONT, (WPARAM)gFontBold, TRUE);
  SendMessageW(GetDlgItem(h, IDC_TOTAL_LEFT), WM_SETFONT, (WPARAM)gFontBold, TRUE);

  int cx = 690, cy = 48;
  MakeStatic(h, IDC_LOGTITLE, L"Cash Log Information", cx, cy, 175, 24, SS_LEFT);
  SendMessageW(GetDlgItem(h, IDC_LOGTITLE), WM_SETFONT, (WPARAM)gFontBold, TRUE);
  MakeBtn(h, IDC_COPY_DEP, L"Copy Cash Log", cx + 178, cy, 118, 24);
  MakeBtn(h, IDC_COPY_EOD, L"Copy EOD", cx + 300, cy, 90, 24);
  MakeBtn(h, IDC_COPY_RST, L"Copy Reset", cx + 394, cy, 96, 24);

  MakeStatic(h, IDC_DEP_LBL, L"Deposit Tab", cx, cy + 30, 120, 20, SS_LEFT);
  MakeBtn(h, IDC_COPY_TOT, L"Copy Totals", cx + 130, cy + 28, 100, 22);
  MakeLv(h, IDC_LV_DEP, cx, cy + 52, 620, 168);
  MakeStatic(h, IDC_EOD_LBL, L"EOD Drawer Count  -  click R# to copy that row", cx, cy + 226, 400, 20, SS_LEFT);
  MakeLv(h, IDC_LV_EOD, cx, cy + 248, 620, 168);
  MakeStatic(h, IDC_RST_LBL, L"Drawer Reset Count  -  click R# to copy that row", cx, cy + 422, 420, 20, SS_LEFT);
  MakeLv(h, IDC_LV_RST, cx, cy + 444, 620, 168);

  auto setup = [](HWND lv) {
    InsertCol(lv, 0, L"", 48);
    InsertCol(lv, 1, L"$1", 40);
    InsertCol(lv, 2, L"$2", 36);
    InsertCol(lv, 3, L"$5", 40);
    InsertCol(lv, 4, L"$10", 44);
    InsertCol(lv, 5, L"$20", 48);
    InsertCol(lv, 6, L"$50", 44);
    InsertCol(lv, 7, L"$100", 52);
    InsertCol(lv, 8, L"Coin", 56);
    InsertCol(lv, 9, L"Total", 80);
  };
  setup(GetDlgItem(h, IDC_LV_DEP));
  setup(GetDlgItem(h, IDC_LV_EOD));
  setup(GetDlgItem(h, IDC_LV_RST));

  MakeStatic(h, IDC_STATUS, L"", 16, 700, 1200, 22, SS_LEFT);
  Relayout(h);
}

void DoCopy(HWND h, int id) {
  ReadCountsFromEdits(h);
  if (id == IDC_COPY_R) {
    auto r = rdc::ComputeRegister(gRegs[gActive], gBase);
    CopyText(rdc::OneDepositTsv(rdc::DepositRow(r)));
    wchar_t msg[64];
    swprintf(msg, 64, L"Copied Register %d deposit row (tab-separated).", gActive + 1);
    Status(h, msg);
    return;
  }
  auto dep = Collect(DepFn);
  auto eod = Collect(EodFn);
  auto rst = Collect(RstFn);
  if (id == IDC_COPY_DEP) {
    CopyText(rdc::AreaTsv(dep));
    Status(h, L"Copied Cash Log (deposit table cells).");
  } else if (id == IDC_COPY_TOT) {
    CopyText(rdc::TotalsTsv(dep));
    Status(h, L"Copied deposit totals row.");
  } else if (id == IDC_COPY_EOD) {
    CopyText(rdc::AreaTsv(eod));
    Status(h, L"Copied EOD drawer table cells.");
  } else {
    CopyText(rdc::AreaTsv(rst));
    Status(h, L"Copied drawer reset table cells.");
  }
}

LRESULT CALLBACK WndProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
    case WM_CREATE:
      Layout(h);
      rdc::LoadOptions(OptionsPath(), &gOptions);
      ApplyTheme();
      rdc::LoadHistory(HistoryPath(), &gHistory);
      LoadSlipFields();
      LoadTillNames();
      ApplyTabNames(h);
      if (gOptions.syncOneDrive) LoadHistoryFromCloud();
      if (!LoadState()) {
        for (int i = 0; i < rdc::kRegisterCount; ++i) gRegs[i] = rdc::Counts{};
      }
      {
        HWND combo = GetDlgItem(h, IDC_BASE);
        for (int i = 0; i < 5; ++i)
          if (rdc::kBaseOptions[i] == gBase) SendMessageW(combo, CB_SETCURSEL, i, 0);
      }
      LoadCountsIntoEdits(h);
      Relayout(h);
      return 0;
    case WM_SIZE:
      if (w != SIZE_MINIMIZED) Relayout(h);
      if (w == SIZE_MAXIMIZED) SaveWindowPlacement(h);
      return 0;
    case WM_EXITSIZEMOVE:
      SaveWindowPlacement(h);
      return 0;
    case WM_ERASEBKGND: {
      HDC dc = (HDC)w;
      RECT rc;
      GetClientRect(h, &rc);
      RECT band{0, 0, rc.right, gHeaderH};
      FillRect(dc, &band, Brush(kNavy));
      RECT rest{0, gHeaderH, rc.right, rc.bottom};
      FillRect(dc, &rest, Brush(kSheet));
      if (gRollBox.right > gRollBox.left) {
        HPEN pen = CreatePen(PS_SOLID, 2, kGrid);
        HGDIOBJ oldPen = SelectObject(dc, pen);
        HGDIOBJ oldBr = SelectObject(dc, GetStockObject(HOLLOW_BRUSH));
        Rectangle(dc, gRollBox.left, gRollBox.top, gRollBox.right, gRollBox.bottom);
        SelectObject(dc, oldPen);
        SelectObject(dc, oldBr);
        DeleteObject(pen);
      }
      return 1;
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = (HDC)w;
      int id = GetDlgCtrlID((HWND)l);
      if (id == IDC_TITLE || id == IDC_BASELBL || id == IDC_TODAY) {
        SetBkColor(dc, kNavy);
        SetTextColor(dc, kOnNavy);
        return (LRESULT)Brush(kNavy);
      }
      if (id == IDC_LOGTITLE) {
        SetBkColor(dc, kSheet);
        SetTextColor(dc, kNavyFg);
        return (LRESULT)Brush(kSheet);
      }
      if (id >= IDC_HDR0 && id < IDC_HDR0 + 6) {
        SetBkColor(dc, kNavyMid);
        SetTextColor(dc, kOnNavy);
        return (LRESULT)Brush(kNavyMid);
      }
      if (id == IDC_TOTAL_LEFT) {
        auto r = rdc::ComputeRegister(gRegs[gActive], gBase);
        COLORREF bg = (!r.hasCount) ? kSheet : (r.balanced ? kOk : kBad);
        SetBkMode(dc, OPAQUE);
        SetBkColor(dc, bg);
        SetTextColor(dc, (!r.hasCount) ? kMuted
                                      : (r.balanced ? kOkInk : kBadInk));
        return (LRESULT)Brush(bg);
      }
      if (id >= IDC_DROP0 && id < IDC_DROP0 + rdc::DenomCount) {
        auto r = rdc::ComputeRegister(gRegs[gActive], gBase);
        const int drop = r.drop.n[id - IDC_DROP0];
        COLORREF bg = drop > 0 ? kDropHit : kComputed;
        SetBkColor(dc, bg);
        SetTextColor(dc, drop > 0 ? kOkInk : kInk);
        return (LRESULT)Brush(bg);
      }
      if ((id >= IDC_AMT0 && id < IDC_AMT0 + rdc::DenomCount) ||
          (id >= IDC_DENOM0 && id < IDC_DENOM0 + rdc::DenomCount) ||
          (id >= IDC_LEFT0 && id < IDC_LEFT0 + rdc::DenomCount) ||
          id == IDC_TOTAL_AMT || id == IDC_TOTAL_DROP) {
        SetBkColor(dc, kComputed);
        SetTextColor(dc, kInk);
        return (LRESULT)Brush(kComputed);
      }
      SetBkColor(dc, kSheet);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kSheet);
    }
    case WM_CTLCOLOREDIT: {
      HDC dc = (HDC)w;
      int id = GetDlgCtrlID((HWND)l);
      if (id == IDC_TAB_EDIT) {
        SetBkColor(dc, kPaper);
        SetTextColor(dc, kInk);
        return (LRESULT)Brush(kPaper);
      }
      SetBkColor(dc, kInput);
      SetTextColor(dc, kCellInk);
      return (LRESULT)Brush(kInput);
    }
    case WM_CTLCOLORLISTBOX: {
      HDC dc = (HDC)w;
      SetBkColor(dc, kPaper);
      SetTextColor(dc, kInk);
      return (LRESULT)Brush(kPaper);
    }
    case WM_DRAWITEM:
      DrawThemedItem((DRAWITEMSTRUCT*)l);
      return TRUE;
    case WM_MEASUREITEM: {
      auto* mi = (MEASUREITEMSTRUCT*)l;
      if (mi->CtlType == ODT_COMBOBOX) {
        mi->itemHeight = 22;
        return TRUE;
      }
      break;
    }
    case WM_NOTIFY: {
      auto* hdr = (NMHDR*)l;
      if (hdr->code == NM_CUSTOMDRAW &&
          (hdr->idFrom == IDC_LV_DEP || hdr->idFrom == IDC_LV_EOD || hdr->idFrom == IDC_LV_RST)) {
        return LvCustomDraw((NMLVCUSTOMDRAW*)l);
      }
      if (hdr->idFrom == IDC_TABS && hdr->code == TCN_SELCHANGE) {
        CommitTillEdit(h);
        ReadCountsFromEdits(h);
        SaveState();
        gActive = TabCtrl_GetCurSel(hdr->hwndFrom);
        gTabClick = gActive;
        LoadCountsIntoEdits(h);
      } else if (hdr->idFrom == IDC_TABS && hdr->code == NM_CLICK) {
        TCHITTESTINFO ht{};
        POINT pt{};
        GetCursorPos(&pt);
        ScreenToClient(hdr->hwndFrom, &pt);
        ht.pt = pt;
        const int hit = TabCtrl_HitTest(hdr->hwndFrom, &ht);
        if (hit >= 0 && hit == gActive && hit == gTabClick) StartTillEdit(h, hit);
        gTabClick = hit;
      } else if ((hdr->idFrom == IDC_LV_DEP || hdr->idFrom == IDC_LV_EOD ||
                  hdr->idFrom == IDC_LV_RST) &&
                 (hdr->code == NM_CLICK || hdr->code == NM_DBLCLK)) {
        auto* item = (NMITEMACTIVATE*)l;
        const int i = item->iItem;
        if (i >= 0 && i < rdc::kRegisterCount) {
          ReadCountsFromEdits(h);
          auto computed = rdc::ComputeRegister(gRegs[i], gBase);
          rdc::LogRow row;
          const wchar_t* kind = L"deposit";
          if (hdr->idFrom == IDC_LV_DEP) {
            row = rdc::DepositRow(computed);
          } else if (hdr->idFrom == IDC_LV_EOD) {
            row = rdc::EodRow(computed);
            kind = L"EOD";
          } else {
            row = rdc::ResetRow(computed, gBase);
            kind = L"reset";
          }
          CopyText(rdc::OneRowTsv(row));
          wchar_t msg[80];
          swprintf(msg, 80, L"Copied R%d %s row (tab-separated).", i + 1, kind);
          Status(h, msg);
        }
      }
      break;
    }
    case WM_COMMAND: {
      int id = LOWORD(w);
      int code = HIWORD(w);
      if (id == IDC_BASE && code == CBN_SELCHANGE) {
        int sel = (int)SendMessageW((HWND)l, CB_GETCURSEL, 0, 0);
        if (sel >= 0 && sel < 5) gBase = rdc::kBaseOptions[sel];
        RefreshComputed(h);
        SaveState();
      } else if (id == IDC_SAMPLE) {
        rdc::LoadSampleDrops(gRegs, gBase);
        gSampleScratch = true;
        LoadCountsIntoEdits(h);
        SaveState();
      } else if (id == IDC_STATS) {
        ShowStats(h);
      } else if (id == IDC_CLEAR) {
        ReadCountsFromEdits(h);
        if (MessageBoxW(h, L"Clear all ten registers?", L"Register Drop Counter",
                        MB_OKCANCEL | MB_ICONWARNING) == IDOK) {
          SnapshotBeforeClear(rdc::HistAll, -1);
          for (int i = 0; i < rdc::kRegisterCount; ++i) gRegs[i] = rdc::Counts{};
          LoadCountsIntoEdits(h);
          SaveState();
          EnableUndoBtn(h);
          if (gHist && IsWindow(gHist)) RefreshHistoryUi(gHist);
          if (gStats && IsWindow(gStats)) InvalidateRect(gStats, nullptr, FALSE);
        }
      } else if (id == IDC_CLEAR_REG) {
        ReadCountsFromEdits(h);
        SnapshotBeforeClear(rdc::HistRegister, gActive);
        gRegs[gActive] = rdc::Counts{};
        LoadCountsIntoEdits(h);
        SaveState();
        EnableUndoBtn(h);
        if (gHist && IsWindow(gHist)) RefreshHistoryUi(gHist);
      } else if (id == IDC_UNDO) {
        UndoLastClear(h);
      } else if (id == IDC_TAB_EDIT && code == EN_KILLFOCUS) {
        CommitTillEdit(h);
      } else if (id == IDC_SLIP) {
        ReadCountsFromEdits(h);
        ShowSlip(h, gActive);
      } else if (id == IDC_HISTORY) {
        ShowHistory(h);
      } else if (id == IDC_OPTIONS) {
        ShowOptions(h);
      } else if (id == IDC_ABOUT) {
        ShowAbout(h);
      } else if (id == IDC_COPY_DEP || id == IDC_COPY_EOD || id == IDC_COPY_RST ||
                 id == IDC_COPY_TOT || id == IDC_COPY_R) {
        DoCopy(h, id);
      } else if (code == EN_CHANGE && !gRefreshing && id >= IDC_COUNT0 &&
                 id < IDC_COUNT0 + rdc::DenomCount) {
        ReadCountsFromEdits(h);
        RefreshComputed(h);
        SaveState();
      }
      return 0;
    }
    case WM_GETMINMAXINFO: {
      auto* m = (MINMAXINFO*)l;
      m->ptMinTrackSize.x = 1000;
      m->ptMinTrackSize.y = 600;
      return 0;
    }
    case WM_DESTROY:
      SaveWindowPlacement(h);
      SaveState();
      PersistHistory();
      PersistOptions();
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(h, msg, w, l);
}

}  // namespace

bool EnterAsTab(HWND root, const MSG& msg) {
  if (!root || !IsWindow(root)) return false;
  if (msg.hwnd != root && !IsChild(root, msg.hwnd)) return false;
  HWND focus = GetFocus();
  const int id = focus ? GetDlgCtrlID(focus) : 0;
  const bool enter = msg.wParam == VK_RETURN;
  const bool tab = msg.wParam == VK_TAB;
  if (root == gMain && id == IDC_TAB_EDIT) {
    if (msg.message == WM_CHAR && (enter || tab || msg.wParam == VK_ESCAPE)) return true;
    if (msg.message != WM_KEYDOWN) return false;
    if (enter || tab) {
      CommitTillEdit(root);
      return true;
    }
    if (msg.wParam == VK_ESCAPE) {
      CancelTillEdit(root);
      return true;
    }
    return false;
  }
  const bool fwdArrow = msg.wParam == VK_RIGHT || msg.wParam == VK_DOWN;
  const bool backArrow = msg.wParam == VK_LEFT || msg.wParam == VK_UP;
  const bool onCount =
      root == gMain && id >= IDC_COUNT0 && id < IDC_COUNT0 + rdc::DenomCount;
  if (onCount) {
    if (!enter && !tab && !fwdArrow && !backArrow) return false;
  } else if (!enter && !tab) {
    return false;
  }
  if (msg.message == WM_CHAR) return true;
  if (msg.message != WM_KEYDOWN) return false;
  const bool back =
      backArrow || ((enter || tab) && (GetKeyState(VK_SHIFT) & 0x8000) != 0);
  if (onCount) {
    MoveCountFocus(root, id, back);
    return true;
  }
  FocusNextTab(root, back);
  return true;
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int show) {
  SetProcessDPIAware();
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  gInst = inst;
  AllowUxDark(LoadLibraryW(L"uxtheme.dll"));
  INITCOMMONCONTROLSEX icc{sizeof(icc),
                           ICC_WIN95_CLASSES | ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES};
  InitCommonControlsEx(&icc);

  RecreateFonts();

  WNDCLASSEXW wc{sizeof(wc)};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = inst;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = Brush(kSheet);
  wc.lpszClassName = kClass;
  wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  RegisterClassExW(&wc);

  HMENU fileMenu = CreatePopupMenu();
  AppendMenuW(fileMenu, MF_STRING, IDC_SAMPLE, L"Load Sample Drops");
  AppendMenuW(fileMenu, MF_STRING, IDC_HISTORY, L"History");
  AppendMenuW(fileMenu, MF_STRING, IDC_STATS, L"Stats for Nerds");
  AppendMenuW(fileMenu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(fileMenu, MF_STRING, IDC_OPTIONS, L"Options");
  AppendMenuW(fileMenu, MF_STRING, IDC_ABOUT, L"About");
  HMENU bar = CreateMenu();
  AppendMenuW(bar, MF_POPUP, (UINT_PTR)fileMenu, L"&File");

  gMain = CreateWindowExW(0, kClass, L"Register Drop Counter",
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1400,
                          820, nullptr, bar, inst, nullptr);
  wchar_t mainTitle[80];
  swprintf(mainTitle, 80, L"Register Drop Counter  %s", rdc::kAppVersion);
  SetWindowTextW(gMain, mainTitle);

  WINDOWPLACEMENT wp{};
  if (LoadWindowPlacement(&wp)) {
    SetWindowPlacement(gMain, &wp);
    ShowWindow(gMain, wp.showCmd);
  } else {
    ShowWindow(gMain, show);
  }
  UpdateWindow(gMain);

  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0)) {
    HWND root = nullptr;
    if (gHist && IsWindow(gHist) &&
        (msg.hwnd == gHist || IsChild(gHist, msg.hwnd)))
      root = gHist;
    else if (gOpt && IsWindow(gOpt) &&
             (msg.hwnd == gOpt || IsChild(gOpt, msg.hwnd)))
      root = gOpt;
    else if (gAbout && IsWindow(gAbout) &&
             (msg.hwnd == gAbout || IsChild(gAbout, msg.hwnd)))
      root = gAbout;
    else if (gStats && IsWindow(gStats) &&
             (msg.hwnd == gStats || IsChild(gStats, msg.hwnd)))
      root = gStats;
    else if (gMain && (msg.hwnd == gMain || IsChild(gMain, msg.hwnd)))
      root = gMain;

    if (root && EnterAsTab(root, msg)) continue;
    if (root && IsDialogMessageW(root, &msg)) continue;
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  CoUninitialize();
  return (int)msg.wParam;
}
