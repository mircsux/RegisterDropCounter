void RelayoutHistory(HWND h);
void RelayoutAbout(HWND h);

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
             {rdc::FormatWhen(e.at), rdc::HistoryLabel(e), std::to_wstring(sum.counted),
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
    return;
  }
  if (!found) gHistSel = idx[0];
  ListView_SetItemState(lv, found ? selectRow : 0, LVIS_SELECTED | LVIS_FOCUSED,
                        LVIS_SELECTED | LVIS_FOCUSED);
  FillDetailList(h, gHistory[gHistSel]);
}

void LayoutHistory(HWND h) {
  MakeStatic(h, IDC_H_DATELBL, L"Search by date (YYYY-MM-DD)", 12, 12, 220, 22, SS_LEFT);
  MakeEditText(h, IDC_H_DATE, 240, 10, 130, 24);
  MakeBtn(h, IDC_H_TODAY, L"Today", 380, 10, 70, 24);
  MakeBtn(h, IDC_H_ALL, L"All dates", 456, 10, 90, 24);
  MakeStatic(h, IDC_H_STAMP, rdc::FormatSheetDate().c_str(), 560, 12, 300, 22, SS_RIGHT);
  SendMessageW(GetDlgItem(h, IDC_H_STAMP), WM_SETFONT, (WPARAM)gFontBold, TRUE);

  HWND list = MakeLv(h, IDC_H_LIST, 12, 42, 850, 180);
  InsertCol(list, 0, L"When", 210);
  InsertCol(list, 1, L"Action", 200);
  InsertCol(list, 2, L"Counted", 80);
  InsertCol(list, 3, L"Amount", 120);
  InsertCol(list, 4, L"Drop", 120);

  MakeBtn(h, IDC_H_RESTORE, L"Restore to Counter", 12, 230, 160, 28);
  MakeBtn(h, IDC_H_CLEAR, L"Clear history", 180, 230, 120, 28);
  MakeBtn(h, IDC_H_CLOSE, L"Close", 310, 230, 80, 28);
  MakeStatic(h, IDC_H_STATUS, L"", 400, 234, 460, 22, SS_LEFT);

  MakeStatic(h, IDC_H_DETLBL, L"Registers in this snapshot - click a row for denom counts", 12, 268,
             500, 20, SS_LEFT);
  HWND det = MakeLv(h, IDC_H_DETAIL, 12, 290, 850, 160);
  InsertCol(det, 0, L"Reg", 50);
  InsertCol(det, 1, L"Amount", 130);
  InsertCol(det, 2, L"Drop", 130);
  InsertCol(det, 3, L"Left", 130);
  InsertCol(det, 4, L"Status", 160);

  MakeStatic(h, IDC_H_DENLBL, L"Denomination counts", 12, 458, 300, 20, SS_LEFT);
  HWND den = MakeLv(h, IDC_H_DENOM, 12, 480, 850, 200);
  InsertCol(den, 0, L"Denom", 120);
  InsertCol(den, 1, L"Count", 90);
  InsertCol(den, 2, L"Drop", 90);
  InsertCol(den, 3, L"Left", 90);
  RelayoutHistory(h);
}

void RelayoutHistory(HWND h) {
  RECT rc{};
  GetClientRect(h, &rc);
  const int W = rc.right;
  const int H = rc.bottom;
  if (W < 200 || H < 160) return;
  const int pad = 12;
  const int topH = 28;
  auto set = [&](int id, HFONT f) {
    HWND c = GetDlgItem(h, id);
    if (c) SendMessageW(c, WM_SETFONT, (WPARAM)f, TRUE);
  };
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

  const int y1 = pad + topH + 8;
  const int body = H - y1 - pad;
  const int listH = (std::max)(80, body * 28 / 100);
  const int btnY = y1 + listH + 8;
  const int btnH = 28;
  const int detY = btnY + btnH + 8;
  const int detH = (std::max)(80, body * 24 / 100);
  const int denLblY = detY + detH + 6;
  const int denY = denLblY + 20;
  const int denH = (std::max)(80, H - denY - pad);

  HDWP dwp = BeginDeferWindowPos(20);
  if (!dwp) return;
  Place(&dwp, h, IDC_H_DATELBL, pad, pad, 220, topH);
  Place(&dwp, h, IDC_H_DATE, pad + 224, pad, 130, topH);
  Place(&dwp, h, IDC_H_TODAY, pad + 362, pad, 70, topH);
  Place(&dwp, h, IDC_H_ALL, pad + 438, pad, 90, topH);
  Place(&dwp, h, IDC_H_STAMP, W - pad - 300, pad, 300, topH);
  Place(&dwp, h, IDC_H_LIST, pad, y1, W - pad * 2, listH);
  Place(&dwp, h, IDC_H_RESTORE, pad, btnY, 160, btnH);
  Place(&dwp, h, IDC_H_CLEAR, pad + 168, btnY, 120, btnH);
  Place(&dwp, h, IDC_H_CLOSE, pad + 296, btnY, 80, btnH);
  Place(&dwp, h, IDC_H_STATUS, pad + 384, btnY + 4, W - pad - 384, btnH);
  Place(&dwp, h, IDC_H_DETLBL, pad, detY - 22, W - pad * 2, 20);
  Place(&dwp, h, IDC_H_DETAIL, pad, detY, W - pad * 2, detH);
  Place(&dwp, h, IDC_H_DENLBL, pad, denLblY, 300, 18);
  Place(&dwp, h, IDC_H_DENOM, pad, denY, W - pad * 2, denH);
  EndDeferWindowPos(dwp);

  const int listParts[] = {210, 200, 80, 120, 120};
  const int detParts[] = {50, 130, 130, 130, 160};
  const int denParts[] = {120, 90, 90, 90};
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
           rdc::FormatWhen(e.at).c_str(), rdc::HistoryLabel(e).c_str());
  if (MessageBoxW(owner, ask, L"Restore snapshot", MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return;
  gBase = e.base;
  for (int r = 0; r < rdc::kRegisterCount; ++r) gRegs[r] = e.registers[r];
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
      FillRect(dc, &rc, Brush(kSheet));
      return 1;
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = (HDC)w;
      SetBkColor(dc, kSheet);
      SetTextColor(dc, RGB(26, 36, 46));
      int id = GetDlgCtrlID((HWND)l);
      if (id == IDC_H_STAMP) {
        SetTextColor(dc, kNavy);
        SetBkColor(dc, kSheet);
      }
      return (LRESULT)Brush(kSheet);
    }
    case WM_NOTIFY: {
      auto* hdr = (NMHDR*)l;
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
      m->ptMinTrackSize.x = 640;
      m->ptMinTrackSize.y = 480;
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
                          rc.left + 40, rc.top + 40, 890, 740, owner, nullptr, gInst, nullptr);
  ShowWindow(gHist, SW_SHOW);
  UpdateWindow(gHist);
}

constexpr wchar_t kAboutClass[] = L"RegisterDropCounterAboutWnd";
HWND gAbout = nullptr;
constexpr int IDC_A_CREDIT = 9101;
constexpr int IDC_A_TEXT = 9102;
constexpr int IDC_A_CLOSE = 9103;
constexpr int IDC_A_LOGBTN = 9104;
constexpr int IDC_A_LOG = 9105;

const wchar_t kAboutBody[] =
    L"Count a drawer\r\n"
    L"1. Set the Register Base ($100, $200, $300, $400, or $500).\r\n"
    L"2. Open a register (R1-R10).\r\n"
    L"3. Type counts in the yellow cells, pennies through $100, including $2 bills.\r\n"
    L"4. Tab or Enter moves to the next field. Shift+Tab or Shift+Enter goes back.\r\n"
    L"5. Amount, Drop, and Left fill in as you type. You do not enter those columns.\r\n"
    L"6. The Left total turns green when it equals the register base. Red means that drawer is still off base.\r\n"
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
    L"Clear and Clear All save the sheet first. Open History to search by date, look at old register counts, or restore a snapshot.\r\n"
    L"\r\n"
    L"Sample\r\n"
    L"Load sample fills the 2026-08-10 workbook example so you can check the drop math.";

const wchar_t kChangelogBody[] =
    L"2026-09-09  Windows, phone, and web app\r\n"
    L"- Visual Studio 2022 C++ app (RegisterDropCounter.sln) with the same drop math as the spreadsheet.\r\n"
    L"- Works on Android phones in the browser: register chips, large yellow count cells, Tab through counts.\r\n"
    L"- History: Clear and Clear All save a snapshot. Search by date, restore, or clear the tab.\r\n"
    L"- $2 bills count, drop, and copy in every register and cash-log table.\r\n"
    L"- R1-R10 copy one register. Copy Cash Log, Copy EOD, and Copy Reset copy that table only.\r\n"
    L"- About window with how-to. Changelog is under the Show changelog button.\r\n"
    L"- Integer-cent engine. The old Excel 5-pennies / MROUND bug is gone.\r\n"
    L"\r\n"
    L"2026-08-10  Excel workbook\r\n"
    L"- RegisterDropCounter-08102026.xlsx.\r\n"
    L"- Drop and Left columns show counts (integers), not dollar amounts. Amount and the Total row stay currency.\r\n"
    L"\r\n"
    L"2024-07-21  Excel\r\n"
    L"- Reduced filesize from 13 MB to under 1 MB by removing the 3D spider on the ChangeLog sheet.\r\n"
    L"\r\n"
    L"2024-07-20  Excel drop fixes\r\n"
    L"- Math drops quarters, nickels, and dimes before coin rolls.\r\n"
    L"- A penny roll still drops before 50 loose pennies. Nobody wants to count out 50 pennies.\r\n"
    L"- Outline around roll columns for visibility.\r\n"
    L"- CopyHack no longer pastes into the ChangeLog. Pastebin must stay internally labelled Sheet2 and hidden. Do not move those sheets; it breaks the copy macros.\r\n"
    L"- Narrowed the 5 pennies bug to MROUND on pennies (fixed later in this app).\r\n"
    L"\r\n"
    L"2024-07-08  Excel\r\n"
    L"- Added coin rolls to the Drop and Left columns.\r\n"
    L"\r\n"
    L"2024-07-06  Excel cash log\r\n"
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
      SetTextColor(dc, kNavy);
      return (LRESULT)Brush(kSheet);
    }
    case WM_CTLCOLOREDIT: {
      HDC dc = (HDC)w;
      SetBkColor(dc, RGB(255, 255, 255));
      SetTextColor(dc, RGB(26, 36, 46));
      return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
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
  const int yBtn = 44;
  int yText = 42;
  int textH;
  int logY = 0, logH = 0;
  if (gAboutLogOpen) {
    const int remain = yClose - yBtn - btnH - 16;
    textH = (std::max)(80, remain * 42 / 100);
    logY = yBtn + btnH + 8;
    logH = (std::max)(80, yClose - 12 - logY);
  } else {
    textH = (std::max)(80, yBtn - yText);
    // how-to fills down to the button
    textH = (std::max)(80, yClose - yText - btnH - 20);
  }
  HDWP dwp = BeginDeferWindowPos(8);
  if (!dwp) return;
  Place(&dwp, h, IDC_A_CREDIT, pad, 12, W - pad * 2, 24);
  Place(&dwp, h, IDC_A_TEXT, pad, yText, W - pad * 2, textH);
  Place(&dwp, h, IDC_A_LOGBTN, pad, yText + textH + 8, 140, btnH);
  if (gAboutLogOpen)
    Place(&dwp, h, IDC_A_LOG, pad, yText + textH + 8 + btnH + 8, W - pad * 2,
          (std::max)(60, yClose - (yText + textH + 8 + btnH + 16)));
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
  ShowWindow(gAbout, SW_SHOW);
  UpdateWindow(gAbout);
}

void Layout(HWND h) {
  MakeStatic(h, IDC_TITLE, L"Register Drop Counter", 16, 12, 280, 24, SS_LEFT);
  SendMessageW(GetDlgItem(h, IDC_TITLE), WM_SETFONT, (WPARAM)gFontBold, TRUE);
  MakeStatic(h, IDC_BASELBL, L"Register Base", 400, 14, 100, 20, SS_RIGHT);
  HWND combo = CreateWindowW(L"COMBOBOX", L"",
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 508, 10, 110,
                             200, h, (HMENU)IDC_BASE, gInst, nullptr);
  SendMessageW(combo, WM_SETFONT, (WPARAM)gFont, TRUE);
  for (int b : rdc::kBaseOptions) {
    wchar_t buf[16];
    swprintf(buf, 16, L"$%d.00", b);
    int idx = (int)SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)buf);
    if (b == gBase) SendMessageW(combo, CB_SETCURSEL, idx, 0);
  }
  MakeBtn(h, IDC_SAMPLE, L"Load sample", 630, 10, 100, 26);
  MakeBtn(h, IDC_CLEAR, L"Clear All", 738, 10, 90, 26);
  MakeBtn(h, IDC_HISTORY, L"History", 836, 10, 80, 26);
  MakeBtn(h, IDC_ABOUT, L"About", 924, 10, 72, 26);
  MakeStatic(h, IDC_TODAY, rdc::FormatSheetDate().c_str(), 1004, 14, 370, 20, SS_RIGHT);
  SendMessageW(GetDlgItem(h, IDC_TODAY), WM_SETFONT, (WPARAM)gFontBold, TRUE);

  HWND tabs = CreateWindowExW(0, WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                              12, 48, 500, 26, h, (HMENU)IDC_TABS, gInst, nullptr);
  SendMessageW(tabs, WM_SETFONT, (WPARAM)gFont, TRUE);
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    wchar_t lab[16];
    swprintf(lab, 16, L"Register %d", i + 1);
    TCITEMW it{};
    it.mask = TCIF_TEXT;
    it.pszText = lab;
    TabCtrl_InsertItem(tabs, i, &it);
  }
  MakeBtn(h, IDC_CLEAR_REG, L"Clear", 520, 48, 64, 26);
  MakeBtn(h, IDC_COPY_R, L"Copy R1", 590, 48, 80, 26);

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
      rdc::LoadHistory(HistoryPath(), &gHistory);
      if (!LoadState()) rdc::LoadSample(gRegs);
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
      return 0;
    case WM_ERASEBKGND: {
      HDC dc = (HDC)w;
      RECT rc;
      GetClientRect(h, &rc);
      RECT band{0, 0, rc.right, gHeaderH};
      FillRect(dc, &band, Brush(kNavy));
      RECT rest{0, gHeaderH, rc.right, rc.bottom};
      FillRect(dc, &rest, Brush(kSheet));
      return 1;
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = (HDC)w;
      int id = GetDlgCtrlID((HWND)l);
      if (id == IDC_TITLE || id == IDC_BASELBL || id == IDC_TODAY) {
        SetBkColor(dc, kNavy);
        SetTextColor(dc, RGB(255, 255, 255));
        return (LRESULT)Brush(kNavy);
      }
      if (id == IDC_LOGTITLE) {
        SetBkColor(dc, kSheet);
        SetTextColor(dc, kNavy);
        return (LRESULT)Brush(kSheet);
      }
      if (id >= IDC_HDR0 && id < IDC_HDR0 + 6) {
        SetBkColor(dc, kNavyMid);
        SetTextColor(dc, RGB(255, 255, 255));
        return (LRESULT)Brush(kNavyMid);
      }
      if (id == IDC_TOTAL_LEFT) {
        auto r = rdc::ComputeRegister(gRegs[gActive], gBase);
        COLORREF bg = (!r.hasCount) ? kSheet : (r.balanced ? kOk : kBad);
        SetBkMode(dc, OPAQUE);
        SetBkColor(dc, bg);
        SetTextColor(dc, (!r.hasCount) ? RGB(92, 107, 122)
                                      : (r.balanced ? RGB(0, 97, 0) : RGB(156, 0, 6)));
        return (LRESULT)Brush(bg);
      }
      if (id >= IDC_DROP0 && id < IDC_DROP0 + rdc::DenomCount) {
        auto r = rdc::ComputeRegister(gRegs[gActive], gBase);
        const int drop = r.drop.n[id - IDC_DROP0];
        COLORREF bg = drop > 0 ? kDropHit : kComputed;
        SetBkColor(dc, bg);
        SetTextColor(dc, drop > 0 ? RGB(0, 97, 0) : RGB(26, 36, 46));
        return (LRESULT)Brush(bg);
      }
      if ((id >= IDC_AMT0 && id < IDC_AMT0 + rdc::DenomCount) ||
          (id >= IDC_DENOM0 && id < IDC_DENOM0 + rdc::DenomCount) ||
          (id >= IDC_LEFT0 && id < IDC_LEFT0 + rdc::DenomCount) ||
          id == IDC_TOTAL_AMT || id == IDC_TOTAL_DROP) {
        SetBkColor(dc, kComputed);
        SetTextColor(dc, RGB(26, 36, 46));
        return (LRESULT)Brush(kComputed);
      }
      SetBkColor(dc, kSheet);
      SetTextColor(dc, RGB(26, 36, 46));
      return (LRESULT)Brush(kSheet);
    }
    case WM_CTLCOLOREDIT: {
      HDC dc = (HDC)w;
      SetBkColor(dc, kInput);
      SetTextColor(dc, RGB(26, 36, 46));
      return (LRESULT)Brush(kInput);
    }
    case WM_NOTIFY: {
      auto* hdr = (NMHDR*)l;
      if (hdr->idFrom == IDC_TABS && hdr->code == TCN_SELCHANGE) {
        ReadCountsFromEdits(h);
        SaveState();
        gActive = TabCtrl_GetCurSel(hdr->hwndFrom);
        LoadCountsIntoEdits(h);
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
        rdc::LoadSample(gRegs);
        gBase = 400;
        SendMessageW(GetDlgItem(h, IDC_BASE), CB_SETCURSEL, 3, 0);
        LoadCountsIntoEdits(h);
        SaveState();
      } else if (id == IDC_CLEAR) {
        ReadCountsFromEdits(h);
        if (MessageBoxW(h, L"Clear all ten registers?", L"Register Drop Counter",
                        MB_OKCANCEL | MB_ICONWARNING) == IDOK) {
          SnapshotBeforeClear(rdc::HistAll, -1);
          for (int i = 0; i < rdc::kRegisterCount; ++i) gRegs[i] = rdc::Counts{};
          LoadCountsIntoEdits(h);
          SaveState();
          if (gHist && IsWindow(gHist)) RefreshHistoryUi(gHist);
        }
      } else if (id == IDC_CLEAR_REG) {
        ReadCountsFromEdits(h);
        SnapshotBeforeClear(rdc::HistRegister, gActive);
        gRegs[gActive] = rdc::Counts{};
        LoadCountsIntoEdits(h);
        SaveState();
        if (gHist && IsWindow(gHist)) RefreshHistoryUi(gHist);
      } else if (id == IDC_HISTORY) {
        ShowHistory(h);
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
      SaveState();
      PersistHistory();
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(h, msg, w, l);
}

}  // namespace

bool EnterAsTab(HWND root, const MSG& msg) {
  if (!root || !IsWindow(root)) return false;
  if (msg.hwnd != root && !IsChild(root, msg.hwnd)) return false;
  const bool enter = msg.wParam == VK_RETURN;
  const bool tab = msg.wParam == VK_TAB;
  if (!enter && !tab) return false;
  if (msg.message == WM_CHAR) return true;
  if (msg.message != WM_KEYDOWN) return false;
  const bool back = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
  HWND focus = GetFocus();
  const int id = focus ? GetDlgCtrlID(focus) : 0;
  if (root == gMain && id >= IDC_COUNT0 && id < IDC_COUNT0 + rdc::DenomCount) {
    MoveCountFocus(root, id, back);
    return true;
  }
  FocusNextTab(root, back);
  return true;
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int show) {
  SetProcessDPIAware();
  gInst = inst;
  INITCOMMONCONTROLSEX icc{sizeof(icc),
                           ICC_WIN95_CLASSES | ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES};
  InitCommonControlsEx(&icc);

  gFont = CreateFontW(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY,
                      DEFAULT_PITCH, L"Segoe UI");
  gFontBold = CreateFontW(-16, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0,
                          CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
  gMono = CreateFontW(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY,
                      DEFAULT_PITCH, L"Consolas");

  WNDCLASSEXW wc{sizeof(wc)};
  wc.lpfnWndProc = WndProc;
  wc.hInstance = inst;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = Brush(kSheet);
  wc.lpszClassName = kClass;
  wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  RegisterClassExW(&wc);

  gMain = CreateWindowExW(0, kClass, L"Register Drop Counter",
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1400,
                          820, nullptr, nullptr, inst, nullptr);
  ShowWindow(gMain, show);
  UpdateWindow(gMain);

  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0)) {
    HWND root = nullptr;
    if (gHist && IsWindow(gHist) &&
        (msg.hwnd == gHist || IsChild(gHist, msg.hwnd)))
      root = gHist;
    else if (gAbout && IsWindow(gAbout) &&
             (msg.hwnd == gAbout || IsChild(gAbout, msg.hwnd)))
      root = gAbout;
    else if (gMain && (msg.hwnd == gMain || IsChild(gMain, msg.hwnd)))
      root = gMain;

    if (root && EnterAsTab(root, msg)) continue;
    if (root && IsDialogMessageW(root, &msg)) continue;
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  return (int)msg.wParam;
}
