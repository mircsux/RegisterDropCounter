#define UNICODE
#define _UNICODE
#define NOMINMAX
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define _WIN32_IE 0x0A00
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <objbase.h>
#include <dwmapi.h>
#include <uxtheme.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef SetWindowPos
#undef SetWindowPos
#endif
#include <algorithm>
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

#include "DropEngine.h"
#include "History.h"
#include "OneDrive.h"
#include "Stats.h"
#include "Version.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(linker, \
                "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' \
version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

constexpr wchar_t kClass[] = L"RegisterDropCounterWnd";
constexpr int IDC_TABS = 101;
constexpr int IDC_BASE = 102;
constexpr int IDC_SAMPLE = 103;
constexpr int IDC_CLEAR = 104;
constexpr int IDC_COPY_DEP = 105;
constexpr int IDC_COPY_EOD = 106;
constexpr int IDC_COPY_RST = 107;
constexpr int IDC_COPY_TOT = 108;
constexpr int IDC_CLEAR_REG = 109;
constexpr int IDC_COPY_R = 110;
constexpr int IDC_TITLE = 111;
constexpr int IDC_BASELBL = 112;
constexpr int IDC_HISTORY = 113;
constexpr int IDC_TODAY = 114;
constexpr int IDC_ABOUT = 115;
constexpr int IDC_OPTIONS = 116;
constexpr int IDC_UNDO = 117;
constexpr int IDC_SLIP = 118;
constexpr int IDC_TAB_EDIT = 119;
constexpr int IDC_STATS = 120;
constexpr int IDC_COUNT0 = 200;
constexpr int IDC_BADGE0 = 250;
constexpr int IDC_AMT0 = 300;
constexpr int IDC_DENOM0 = 350;
constexpr int IDC_DROP0 = 400;
constexpr int IDC_LEFT0 = 500;
constexpr int IDC_TOTAL_AMT = 600;
constexpr int IDC_TOTAL_DROP = 601;
constexpr int IDC_TOTAL_LEFT = 602;
constexpr int IDC_TOTAL_LBL = 603;
constexpr int IDC_HDR0 = 610;
constexpr int IDC_LV_DEP = 700;
constexpr int IDC_LV_EOD = 701;
constexpr int IDC_LV_RST = 702;
constexpr int IDC_STATUS = 800;
constexpr int IDC_LOGTITLE = 801;
constexpr int IDC_DEP_LBL = 802;
constexpr int IDC_EOD_LBL = 803;
constexpr int IDC_RST_LBL = 804;

constexpr int IDC_H_DATE = 9001;
constexpr int IDC_H_TODAY = 9002;
constexpr int IDC_H_ALL = 9003;
constexpr int IDC_H_LIST = 9004;
constexpr int IDC_H_DETAIL = 9005;
constexpr int IDC_H_DENOM = 9006;
constexpr int IDC_H_RESTORE = 9007;
constexpr int IDC_H_CLEAR = 9008;
constexpr int IDC_H_CLOSE = 9009;
constexpr int IDC_H_STATUS = 9010;
constexpr int IDC_H_STAMP = 9011;
constexpr int IDC_H_DATELBL = 9012;
constexpr int IDC_H_DETLBL = 9013;
constexpr int IDC_H_DENLBL = 9014;
constexpr int IDC_H_TITLE = 9015;

constexpr wchar_t kHistClass[] = L"RegisterDropCounterHistoryWnd";

HINSTANCE gInst = nullptr;
HWND gMain = nullptr;
HWND gHist = nullptr;
HWND gSlip = nullptr;
HWND gOpt = nullptr;
HWND gAbout = nullptr;
HWND gStats = nullptr;
HFONT gFont = nullptr;
HFONT gFontBold = nullptr;
HFONT gMono = nullptr;
WNDPROC gOldEditProc = nullptr;
int gHeaderH = 44;
int gFontPx = 0;
bool gFontDark = false;
bool gAboutLogOpen = false;
rdc::Counts gRegs[rdc::kRegisterCount]{};
int gBase = 400;
int gActive = 0;
bool gSampleScratch = false;
bool gRefreshing = false;
std::vector<rdc::HistoryEntry> gHistory;
rdc::Options gOptions;
std::wstring gHistFilter;
int gHistSel = -1;
int gHistDetailReg = 0;
std::wstring gSlipBag;
std::wstring gSlipInitials;
int gSlipReg = 0;
std::wstring gNames[rdc::kRegisterCount];
int gTabClick = -1;
RECT gRollBox{};

COLORREF kNavy = RGB(31, 78, 121);
COLORREF kNavyMid = RGB(46, 117, 182);
COLORREF kNavyDeep = RGB(22, 58, 95);
COLORREF kOnNavy = RGB(247, 251, 255);
COLORREF kInput = RGB(255, 244, 194);
COLORREF kComputed = RGB(248, 228, 212);
COLORREF kOk = RGB(198, 239, 206);
COLORREF kBad = RGB(255, 199, 206);
COLORREF kDropHit = RGB(214, 234, 223);
COLORREF kCellInk = RGB(26, 36, 46);
COLORREF kOkInk = RGB(0, 97, 0);
COLORREF kBadInk = RGB(156, 0, 6);
COLORREF kSheet = RGB(238, 241, 244);
COLORREF kPaper = RGB(255, 255, 255);
COLORREF kInk = RGB(26, 36, 46);
COLORREF kNavyFg = RGB(31, 78, 121);
COLORREF kMuted = RGB(92, 107, 122);
COLORREF kGrid = RGB(197, 208, 219);

HBRUSH gBrNavy = nullptr;
HBRUSH gBrMid = nullptr;
HBRUSH gBrDeep = nullptr;
HBRUSH gBrIn = nullptr;
HBRUSH gBrPeach = nullptr;
HBRUSH gBrOk = nullptr;
HBRUSH gBrBad = nullptr;
HBRUSH gBrHit = nullptr;
HBRUSH gBrSheet = nullptr;
HBRUSH gBrPaper = nullptr;
WNDPROC gOldHdr = nullptr;

typedef int(WINAPI* SetPreferredAppModeFn)(int);
typedef void(WINAPI* FlushMenuThemesFn)(void);
typedef BOOL(WINAPI* AllowDarkModeForWindowFn)(HWND, BOOL);

FARPROC UxOrd(HMODULE ux, int ord) {
  if (!ux) return nullptr;
  return GetProcAddress(ux, (LPCSTR)(ULONG_PTR)(WORD)ord);
}

void AllowUxDark(HMODULE ux) {
  auto setPref = (SetPreferredAppModeFn)UxOrd(ux, 135);
  if (setPref) setPref(1);
}

void RebuildBrushes() {
  auto put = [](HBRUSH* slot, COLORREF c) {
    if (*slot) DeleteObject(*slot);
    *slot = CreateSolidBrush(c);
  };
  put(&gBrNavy, kNavy);
  put(&gBrMid, kNavyMid);
  put(&gBrDeep, kNavyDeep);
  put(&gBrIn, kInput);
  put(&gBrPeach, kComputed);
  put(&gBrOk, kOk);
  put(&gBrBad, kBad);
  put(&gBrHit, kDropHit);
  put(&gBrSheet, kSheet);
  put(&gBrPaper, kPaper);
}

void ApplyWinDark(HWND h);
void ThemeListView(HWND lv);
void ThemeChildEdits(HWND parent);
void RecreateFonts();
void ApplyMainFonts(HWND h);

void ApplyTheme() {
  if (gOptions.darkMode) {
    kNavy = RGB(16, 44, 50);
    kNavyMid = RGB(30, 90, 100);
    kNavyDeep = RGB(22, 62, 70);
    kOnNavy = RGB(231, 246, 242);
    kSheet = RGB(6, 8, 9);
    kPaper = RGB(16, 23, 26);
    kInk = RGB(231, 246, 242);
    kNavyFg = RGB(110, 231, 212);
    kMuted = RGB(143, 179, 174);
    kGrid = RGB(44, 69, 75);
    kInput = RGB(245, 197, 66);
    kComputed = RGB(26, 39, 43);
    kOk = RGB(13, 63, 53);
    kBad = RGB(77, 31, 40);
    kDropHit = RGB(21, 86, 74);
    kCellInk = RGB(28, 20, 6);
    kOkInk = RGB(110, 240, 200);
    kBadInk = RGB(255, 176, 188);
  } else {
    kNavy = RGB(31, 78, 121);
    kNavyMid = RGB(46, 117, 182);
    kNavyDeep = RGB(22, 58, 95);
    kOnNavy = RGB(247, 251, 255);
    kSheet = RGB(238, 241, 244);
    kPaper = RGB(255, 255, 255);
    kInk = RGB(26, 36, 46);
    kNavyFg = RGB(31, 78, 121);
    kMuted = RGB(92, 107, 122);
    kGrid = RGB(197, 208, 219);
    kInput = RGB(255, 244, 194);
    kComputed = RGB(248, 228, 212);
    kOk = RGB(198, 239, 206);
    kBad = RGB(255, 199, 206);
    kDropHit = RGB(214, 234, 223);
    kCellInk = RGB(26, 36, 46);
    kOkInk = RGB(0, 97, 0);
    kBadInk = RGB(156, 0, 6);
  }
  RebuildBrushes();
  gFontPx = 0;
  RecreateFonts();
  HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
  if (!ux) ux = LoadLibraryW(L"uxtheme.dll");
  if (ux) {
    auto setPref = (SetPreferredAppModeFn)UxOrd(ux, 135);
    auto flush = (FlushMenuThemesFn)UxOrd(ux, 136);
    if (setPref) setPref(gOptions.darkMode ? 2 : 3);
    if (flush) flush();
  }
  auto paint = [](HWND w) {
    if (!w || !IsWindow(w)) return;
    ApplyWinDark(w);
    ThemeChildEdits(w);
    InvalidateRect(w, nullptr, TRUE);
    RedrawWindow(w, nullptr, nullptr,
                 RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME | RDW_UPDATENOW);
  };
  paint(gMain);
  paint(gHist);
  paint(gSlip);
  paint(gOpt);
  paint(gAbout);
  paint(gStats);
  if (gMain) ApplyMainFonts(gMain);
  if (gMain) {
    ThemeListView(GetDlgItem(gMain, IDC_LV_DEP));
    ThemeListView(GetDlgItem(gMain, IDC_LV_EOD));
    ThemeListView(GetDlgItem(gMain, IDC_LV_RST));
  }
  if (gHist) {
    ThemeListView(GetDlgItem(gHist, IDC_H_LIST));
    ThemeListView(GetDlgItem(gHist, IDC_H_DETAIL));
    ThemeListView(GetDlgItem(gHist, IDC_H_DENOM));
  }
}

HBRUSH Brush(COLORREF c) {
  if (!gBrSheet) RebuildBrushes();
  if (c == kNavy) return gBrNavy;
  if (c == kNavyMid) return gBrMid;
  if (c == kNavyDeep) return gBrDeep;
  if (c == kInput) return gBrIn;
  if (c == kComputed) return gBrPeach;
  if (c == kOk) return gBrOk;
  if (c == kBad) return gBrBad;
  if (c == kDropHit) return gBrHit;
  if (c == kSheet) return gBrSheet;
  if (c == kPaper) return gBrPaper;
  return gBrPaper;
}

void Untheme(HWND w) {
  if (w && IsWindow(w)) SetWindowTheme(w, L"", L"");
}

void ApplyWinDark(HWND h) {
  if (!h || !IsWindow(h)) return;
  HMODULE ux = GetModuleHandleW(L"uxtheme.dll");
  if (ux) {
    auto allow = (AllowDarkModeForWindowFn)UxOrd(ux, 133);
    if (allow) allow(h, gOptions.darkMode ? TRUE : FALSE);
  }
  BOOL on = gOptions.darkMode ? TRUE : FALSE;
  DwmSetWindowAttribute(h, 20, &on, sizeof(on));
  DwmSetWindowAttribute(h, 19, &on, sizeof(on));
}

LRESULT CALLBACK ThemeHdrProc(HWND h, UINT msg, WPARAM w, LPARAM l) {
  if (msg == WM_ERASEBKGND) {
    RECT rc;
    GetClientRect(h, &rc);
    FillRect((HDC)w, &rc, Brush(kNavyDeep));
    return 1;
  }
  if (msg == WM_PAINT) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(h, &ps);
    RECT rc;
    GetClientRect(h, &rc);
    FillRect(dc, &rc, Brush(kNavyDeep));
    const int n = Header_GetItemCount(h);
    HFONT old = (HFONT)SelectObject(dc, gFont);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, kOnNavy);
    HPEN pen = CreatePen(PS_SOLID, 1, kGrid);
    HGDIOBJ op = SelectObject(dc, pen);
    for (int i = 0; i < n; ++i) {
      RECT ir{};
      Header_GetItemRect(h, i, &ir);
      wchar_t buf[64]{};
      HDITEMW it{};
      it.mask = HDI_TEXT;
      it.pszText = buf;
      it.cchTextMax = 64;
      Header_GetItem(h, i, &it);
      MoveToEx(dc, ir.right - 1, ir.top + 3, nullptr);
      LineTo(dc, ir.right - 1, ir.bottom - 3);
      InflateRect(&ir, -6, 0);
      DrawTextW(dc, buf, -1, &ir, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    }
    SelectObject(dc, op);
    DeleteObject(pen);
    SelectObject(dc, old);
    EndPaint(h, &ps);
    return 0;
  }
  return CallWindowProcW(gOldHdr, h, msg, w, l);
}

void ThemeListView(HWND lv) {
  if (!lv || !IsWindow(lv)) return;
  Untheme(lv);
  ListView_SetBkColor(lv, kPaper);
  ListView_SetTextBkColor(lv, kPaper);
  ListView_SetTextColor(lv, kInk);
  LONG_PTR ex = GetWindowLongPtrW(lv, GWL_EXSTYLE);
  if (gOptions.darkMode) ex &= ~WS_EX_CLIENTEDGE;
  else ex |= WS_EX_CLIENTEDGE;
  SetWindowLongPtrW(lv, GWL_EXSTYLE, ex);
  ::SetWindowPos(lv, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
  HWND hdr = ListView_GetHeader(lv);
  if (hdr) {
    Untheme(hdr);
    if (!gOldHdr) gOldHdr = (WNDPROC)GetWindowLongPtrW(hdr, GWLP_WNDPROC);
    WNDPROC cur = (WNDPROC)GetWindowLongPtrW(hdr, GWLP_WNDPROC);
    if (cur != ThemeHdrProc)
      SetWindowLongPtrW(hdr, GWLP_WNDPROC, (LONG_PTR)ThemeHdrProc);
    InvalidateRect(hdr, nullptr, TRUE);
  }
  InvalidateRect(lv, nullptr, TRUE);
}

BOOL CALLBACK ThemeEditCb(HWND c, LPARAM) {
  wchar_t cls[32]{};
  GetClassNameW(c, cls, 32);
  if (_wcsicmp(cls, L"Edit") == 0 || _wcsicmp(cls, L"ComboBox") == 0 ||
      _wcsicmp(cls, L"Button") == 0 || _wcsicmp(cls, WC_TABCONTROLW) == 0 ||
      _wcsicmp(cls, WC_LISTVIEWW) == 0) {
    Untheme(c);
  }
  return TRUE;
}

void ThemeChildEdits(HWND parent) {
  if (!parent) return;
  EnumChildWindows(parent, ThemeEditCb, 0);
}

void FillRoundDc(HDC dc, RECT r, COLORREF fill, COLORREF edge) {
  HBRUSH b = CreateSolidBrush(fill);
  HPEN p = CreatePen(PS_SOLID, 1, edge);
  HGDIOBJ ob = SelectObject(dc, b);
  HGDIOBJ op = SelectObject(dc, p);
  RoundRect(dc, r.left, r.top, r.right, r.bottom, 8, 8);
  SelectObject(dc, ob);
  SelectObject(dc, op);
  DeleteObject(b);
  DeleteObject(p);
}

bool IsNavyButton(int id) {
  return id == IDC_CLEAR || id == IDC_UNDO || id == IDC_HISTORY;
}

void DrawThemedItem(const DRAWITEMSTRUCT* di) {
  if (!di) return;
  RECT r = di->rcItem;
  const bool down = (di->itemState & ODS_SELECTED) != 0;
  const bool dis = (di->itemState & ODS_DISABLED) != 0;
  const bool focus = (di->itemState & ODS_FOCUS) != 0;

  if (di->CtlType == ODT_BUTTON) {
    const bool navy = IsNavyButton(di->CtlID);
    COLORREF fill, fg, edge;
    if (navy) {
      fill = down ? kNavy : kNavyDeep;
      fg = dis ? kMuted : kOnNavy;
      edge = kNavyMid;
    } else {
      fill = down ? kComputed : kPaper;
      fg = dis ? kMuted : kInk;
      edge = kGrid;
    }
    FillRoundDc(di->hDC, r, fill, edge);
    wchar_t text[128]{};
    GetWindowTextW(di->hwndItem, text, 128);
    SetBkMode(di->hDC, TRANSPARENT);
    SetTextColor(di->hDC, fg);
    HFONT old = (HFONT)SelectObject(di->hDC, gFont);
    DrawTextW(di->hDC, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(di->hDC, old);
    if (focus && !dis) {
      InflateRect(&r, -3, -3);
      DrawFocusRect(di->hDC, &r);
    }
    return;
  }

  if (di->CtlType == ODT_TAB) {
    const bool sel = (di->itemState & ODS_SELECTED) != 0;
    FillRect(di->hDC, &r, Brush(sel ? kNavyMid : kNavyDeep));
    wchar_t buf[32]{};
    TCITEMW it{};
    it.mask = TCIF_TEXT;
    it.pszText = buf;
    it.cchTextMax = 32;
    TabCtrl_GetItem(di->hwndItem, (int)di->itemID, &it);
    SetBkMode(di->hDC, TRANSPARENT);
    SetTextColor(di->hDC, sel ? kOnNavy : kMuted);
    HFONT old = (HFONT)SelectObject(di->hDC, gFont);
    DrawTextW(di->hDC, buf, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(di->hDC, old);
    return;
  }

  if (di->CtlType == ODT_COMBOBOX) {
    const bool editPart = (di->itemState & ODS_COMBOBOXEDIT) != 0;
    const bool sel = (di->itemState & ODS_SELECTED) != 0;
    COLORREF fill, fg;
    if (editPart) {
      fill = down ? kNavy : kNavyDeep;
      fg = kOnNavy;
    } else {
      fill = sel ? kNavyMid : kPaper;
      fg = sel ? kOnNavy : kInk;
    }
    FillRect(di->hDC, &r, fill == kPaper ? Brush(kPaper) : (fill == kNavyMid ? Brush(kNavyMid) : Brush(kNavyDeep)));
    wchar_t buf[64]{};
    if (di->itemID != (UINT)-1)
      SendMessageW(di->hwndItem, CB_GETLBTEXT, di->itemID, (LPARAM)buf);
    SetBkMode(di->hDC, TRANSPARENT);
    SetTextColor(di->hDC, fg);
    HFONT old = (HFONT)SelectObject(di->hDC, gFont);
    RECT tr = r;
    tr.left += 8;
    DrawTextW(di->hDC, buf, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(di->hDC, old);
  }
}

LRESULT LvCustomDraw(NMLVCUSTOMDRAW* cd) {
  if (cd->nmcd.dwDrawStage == CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;
  if (cd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
    cd->clrText = kInk;
    cd->clrTextBk = kPaper;
    return CDRF_NEWFONT;
  }
  return CDRF_DODEFAULT;
}

std::wstring StatePath() {
  wchar_t buf[MAX_PATH];
  GetModuleFileNameW(nullptr, buf, MAX_PATH);
  std::wstring p(buf);
  const auto slash = p.find_last_of(L"\\/");
  if (slash != std::wstring::npos) p.resize(slash + 1);
  return p + L"RegisterDropCounter.state";
}

std::wstring NamesPath() {
  wchar_t buf[MAX_PATH];
  GetModuleFileNameW(nullptr, buf, MAX_PATH);
  std::wstring p(buf);
  const auto slash = p.find_last_of(L"\\/");
  if (slash != std::wstring::npos) p.resize(slash + 1);
  return p + L"RegisterDropCounter.names";
}

std::wstring SlipPath() {
  wchar_t buf[MAX_PATH];
  GetModuleFileNameW(nullptr, buf, MAX_PATH);
  std::wstring p(buf);
  const auto slash = p.find_last_of(L"\\/");
  if (slash != std::wstring::npos) p.resize(slash + 1);
  return p + L"RegisterDropCounter.slip";
}

std::wstring HistoryPath() {
  wchar_t buf[MAX_PATH];
  GetModuleFileNameW(nullptr, buf, MAX_PATH);
  std::wstring p(buf);
  const auto slash = p.find_last_of(L"\\/");
  if (slash != std::wstring::npos) p.resize(slash + 1);
  return p + L"RegisterDropCounter.history";
}

std::wstring OptionsPath() {
  wchar_t buf[MAX_PATH];
  GetModuleFileNameW(nullptr, buf, MAX_PATH);
  std::wstring p(buf);
  const auto slash = p.find_last_of(L"\\/");
  if (slash != std::wstring::npos) p.resize(slash + 1);
  return p + L"RegisterDropCounter.options";
}

std::wstring WindowPath() {
  wchar_t buf[MAX_PATH];
  GetModuleFileNameW(nullptr, buf, MAX_PATH);
  std::wstring p(buf);
  const auto slash = p.find_last_of(L"\\/");
  if (slash != std::wstring::npos) p.resize(slash + 1);
  return p + L"RegisterDropCounter.window";
}

void ClampToWorkArea(RECT* r) {
  int w = r->right - r->left;
  int h = r->bottom - r->top;
  if (w < 1000) w = 1000;
  if (h < 600) h = 600;
  HMONITOR mon = MonitorFromRect(r, MONITOR_DEFAULTTONEAREST);
  MONITORINFO mi{sizeof(mi)};
  if (!GetMonitorInfoW(mon, &mi)) return;
  const RECT& wa = mi.rcWork;
  const int maxW = wa.right - wa.left;
  const int maxH = wa.bottom - wa.top;
  if (w > maxW) w = maxW;
  if (h > maxH) h = maxH;
  if (r->left < wa.left) r->left = wa.left;
  if (r->top < wa.top) r->top = wa.top;
  if (r->left + w > wa.right) r->left = wa.right - w;
  if (r->top + h > wa.bottom) r->top = wa.bottom - h;
  if (r->left < wa.left) r->left = wa.left;
  if (r->top < wa.top) r->top = wa.top;
  r->right = r->left + w;
  r->bottom = r->top + h;
}

void SaveWindowPlacement(HWND h) {
  if (!h || !IsWindow(h)) return;
  WINDOWPLACEMENT wp{sizeof(wp)};
  if (!GetWindowPlacement(h, &wp)) return;
  std::wofstream out(WindowPath().c_str());
  if (!out) return;
  out << L"RDCW1\n"
      << (int)wp.showCmd << L' ' << wp.rcNormalPosition.left << L' ' << wp.rcNormalPosition.top
      << L' ' << wp.rcNormalPosition.right << L' ' << wp.rcNormalPosition.bottom << L'\n';
}

bool LoadWindowPlacement(WINDOWPLACEMENT* wp) {
  std::wifstream in(WindowPath().c_str());
  if (!in) return false;
  std::wstring mag;
  std::getline(in, mag);
  if (mag != L"RDCW1") return false;
  int show = SW_SHOWNORMAL, l = 0, t = 0, r = 0, b = 0;
  in >> show >> l >> t >> r >> b;
  if (!in) return false;
  if (r - l < 200 || b - t < 160) return false;
  *wp = WINDOWPLACEMENT{};
  wp->length = sizeof(*wp);
  wp->flags = 0;
  wp->showCmd = show;
  if (wp->showCmd == SW_SHOWMINIMIZED || wp->showCmd == SW_MINIMIZE ||
      wp->showCmd == SW_FORCEMINIMIZE)
    wp->showCmd = SW_SHOWNORMAL;
  wp->rcNormalPosition = {l, t, r, b};
  ClampToWorkArea(&wp->rcNormalPosition);
  return true;
}

void PersistHistory() {
  rdc::SaveHistory(HistoryPath(), gHistory);
  if (!gOptions.syncOneDrive) return;
  const std::wstring folder = rdc::ResolvedSyncFolder(gOptions);
  if (folder.empty() || !rdc::EnsureFolder(folder)) return;
  rdc::SaveHistory(rdc::CloudHistoryPath(gOptions), gHistory);
}

void PersistOptions() { rdc::SaveOptions(OptionsPath(), gOptions); }

std::wstring SyncHistoryNow() {
  if (!gOptions.syncOneDrive) return L"OneDrive sync is off.";
  const std::wstring folder = rdc::ResolvedSyncFolder(gOptions);
  if (folder.empty())
    return L"OneDrive folder not found. Choose a folder in Options.";
  if (!rdc::EnsureFolder(folder)) return L"Could not create the OneDrive folder.";
  const std::wstring cloud = rdc::CloudHistoryPath(gOptions);
  std::vector<rdc::HistoryEntry> remote;
  rdc::LoadHistory(cloud, &remote);
  gHistory = rdc::MergeHistories(gHistory, remote);
  rdc::SaveHistory(HistoryPath(), gHistory);
  rdc::SaveHistory(cloud, gHistory);
  wchar_t msg[160];
  swprintf(msg, 160, L"Synced %d snapshot(s) with OneDrive.", (int)gHistory.size());
  return msg;
}

bool LoadHistoryFromCloud() {
  const std::wstring cloud = rdc::CloudHistoryPath(gOptions);
  if (cloud.empty()) return false;
  std::vector<rdc::HistoryEntry> remote;
  if (!rdc::LoadHistory(cloud, &remote)) return false;
  gHistory = rdc::MergeHistories(gHistory, remote);
  PersistHistory();
  return true;
}

bool SaveHistoryToCloud() {
  if (!gOptions.syncOneDrive) return false;
  const std::wstring folder = rdc::ResolvedSyncFolder(gOptions);
  if (folder.empty() || !rdc::EnsureFolder(folder)) return false;
  rdc::SaveHistory(rdc::CloudHistoryPath(gOptions), gHistory);
  return true;
}

void SnapshotBeforeClear(rdc::HistoryKind kind, int registerIndex) {
  if (gSampleScratch) {
    if (kind == rdc::HistAll) gSampleScratch = false;
    return;
  }
  rdc::HistoryEntry e;
  if (!rdc::MakeSnapshot(&e, kind, registerIndex, gBase, gRegs)) return;
  rdc::PrependHistory(&gHistory, e);
  PersistHistory();
}

void EnableUndoBtn(HWND h) {
  HWND b = GetDlgItem(h, IDC_UNDO);
  if (b) EnableWindow(b, gHistory.empty() ? FALSE : TRUE);
}

void PersistSlipFields() {
  std::wofstream out(SlipPath().c_str());
  if (!out) return;
  out << L"RDCS1\n" << gSlipBag << L"\n" << gSlipInitials << L"\n";
}

void LoadSlipFields() {
  gSlipBag.clear();
  gSlipInitials.clear();
  std::wifstream in(SlipPath().c_str());
  if (!in) return;
  std::wstring mag;
  std::getline(in, mag);
  if (mag != L"RDCS1") return;
  std::getline(in, gSlipBag);
  std::getline(in, gSlipInitials);
  while (!gSlipBag.empty() && (gSlipBag.back() == L'\r' || gSlipBag.back() == L'\n'))
    gSlipBag.pop_back();
  while (!gSlipInitials.empty() &&
         (gSlipInitials.back() == L'\r' || gSlipInitials.back() == L'\n'))
    gSlipInitials.pop_back();
}

void DefaultTillNames() {
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    wchar_t buf[8];
    swprintf(buf, 8, L"R%d", i + 1);
    gNames[i] = buf;
  }
}

std::wstring SanitizeTillName(const wchar_t* raw, int index) {
  std::wstring s = raw ? raw : L"";
  while (!s.empty() && (s.front() == L' ' || s.front() == L'\t')) s.erase(s.begin());
  while (!s.empty() && (s.back() == L' ' || s.back() == L'\t' || s.back() == L'\r')) s.pop_back();
  if (s.size() > 20) s.resize(20);
  if (s.empty()) {
    wchar_t buf[8];
    swprintf(buf, 8, L"R%d", index + 1);
    return buf;
  }
  return s;
}

void PersistTillNames() {
  std::wofstream out(NamesPath().c_str());
  if (!out) return;
  out << L"RDCN1\n";
  for (int i = 0; i < rdc::kRegisterCount; ++i) out << gNames[i] << L"\n";
}

void LoadTillNames() {
  DefaultTillNames();
  std::wifstream in(NamesPath().c_str());
  if (!in) return;
  std::wstring mag;
  std::getline(in, mag);
  if (mag != L"RDCN1") return;
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    std::wstring line;
    if (!std::getline(in, line)) break;
    while (!line.empty() && (line.back() == L'\r' || line.back() == L'\n')) line.pop_back();
    gNames[i] = SanitizeTillName(line.c_str(), i);
  }
}

void SaveState() {
  std::wofstream out(StatePath().c_str());
  if (!out) return;
  out << L"RDC1\n" << gBase << L"\n";
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    for (int d = 0; d < rdc::DenomCount; ++d) {
      if (d) out << L',';
      out << gRegs[i].n[d];
    }
    out << L"\n";
  }
  out << L"SAMPLE " << (gSampleScratch ? 1 : 0) << L"\n";
}

bool LoadState() {
  std::wifstream in(StatePath().c_str());
  if (!in) return false;
  std::wstring mag;
  std::getline(in, mag);
  if (mag != L"RDC1") return false;
  in >> gBase;
  in.ignore(64, L'\n');
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    std::wstring line;
    if (!std::getline(in, line)) break;
    std::vector<int> vals;
    size_t start = 0;
    for (;;) {
      const auto comma = line.find(L',', start);
      const auto tok = line.substr(
          start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
      vals.push_back(rdc::ClampCount(_wtoi(tok.c_str())));
      if (comma == std::wstring::npos) break;
      start = comma + 1;
    }
    // RDC1 files written before $2 bills had 14 denoms; Two sits after One (index 9).
    if (vals.size() == 14) vals.insert(vals.begin() + 9, 0);
    for (int d = 0; d < rdc::DenomCount && d < (int)vals.size(); ++d)
      gRegs[i].n[d] = vals[d];
  }
  gSampleScratch = false;
  std::wstring extra;
  if (std::getline(in, extra)) {
    while (!extra.empty() && (extra.back() == L'\r' || extra.back() == L'\n')) extra.pop_back();
    if (extra.rfind(L"SAMPLE", 0) == 0) {
      gSampleScratch = extra.find(L"1") != std::wstring::npos;
    }
  }
  if (gBase != 100 && gBase != 200 && gBase != 300 && gBase != 400 && gBase != 500)
    gBase = 400;
  return true;
}

void SetEditInt(HWND h, int id, int v) {
  wchar_t buf[16];
  if (v == 0)
    buf[0] = 0;
  else
    swprintf(buf, 16, L"%d", v);
  SetDlgItemTextW(h, id, buf);
}

int GetEditInt(HWND h, int id) {
  wchar_t buf[32];
  GetDlgItemTextW(h, id, buf, 32);
  return rdc::ClampCount(_wtoi(buf));
}

void CopyText(const std::wstring& text) {
  if (!OpenClipboard(gMain)) return;
  EmptyClipboard();
  const size_t bytes = (text.size() + 1) * sizeof(wchar_t);
  HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, bytes);
  if (!mem) {
    CloseClipboard();
    return;
  }
  void* p = GlobalLock(mem);
  memcpy(p, text.c_str(), bytes);
  GlobalUnlock(mem);
  SetClipboardData(CF_UNICODETEXT, mem);
  CloseClipboard();
}

void Status(HWND h, const wchar_t* msg) { SetDlgItemTextW(h, IDC_STATUS, msg); }

HWND MakeLv(HWND parent, int id, int x, int y, int w, int h) {
  HWND lv = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
                            x, y, w, h, parent, (HMENU)(INT_PTR)id, gInst, nullptr);
  ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
  SendMessageW(lv, WM_SETFONT, (WPARAM)gMono, TRUE);
  ThemeListView(lv);
  return lv;
}

void InsertCol(HWND lv, int i, const wchar_t* title, int w) {
  LVCOLUMNW c{};
  c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
  c.pszText = const_cast<wchar_t*>(title);
  c.cx = w;
  c.fmt = i == 0 ? LVCFMT_LEFT : LVCFMT_RIGHT;
  ListView_InsertColumn(lv, i, &c);
}

void AddLvRow(HWND lv, int row, const std::vector<std::wstring>& cols) {
  LVITEMW it{};
  it.mask = LVIF_TEXT;
  it.iItem = row;
  it.pszText = const_cast<wchar_t*>(cols[0].c_str());
  ListView_InsertItem(lv, &it);
  for (size_t i = 1; i < cols.size(); ++i) {
    ListView_SetItemText(lv, row, (int)i, const_cast<wchar_t*>(cols[i].c_str()));
  }
}

std::vector<rdc::LogRow> Collect(rdc::LogRow (*fn)(const rdc::RegisterResult&, int)) {
  std::vector<rdc::LogRow> rows;
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    auto r = rdc::ComputeRegister(gRegs[i], gBase);
    rows.push_back(fn(r, gBase));
  }
  return rows;
}

rdc::LogRow DepFn(const rdc::RegisterResult& r, int) { return rdc::DepositRow(r); }
rdc::LogRow EodFn(const rdc::RegisterResult& r, int) { return rdc::EodRow(r); }
rdc::LogRow RstFn(const rdc::RegisterResult& r, int base) { return rdc::ResetRow(r, base); }

void RefreshCashLog(HWND h) {
  auto dep = Collect(DepFn);
  auto eod = Collect(EodFn);
  auto rst = Collect(RstFn);
  HWND lvD = GetDlgItem(h, IDC_LV_DEP);
  HWND lvE = GetDlgItem(h, IDC_LV_EOD);
  HWND lvR = GetDlgItem(h, IDC_LV_RST);
  ListView_DeleteAllItems(lvD);
  ListView_DeleteAllItems(lvE);
  ListView_DeleteAllItems(lvR);
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    wchar_t lab[8];
    swprintf(lab, 8, L"R%d", i + 1);
    auto put = [&](HWND lv, const rdc::LogRow& r) {
      AddLvRow(lv, i,
               {lab, std::to_wstring(r.ones), std::to_wstring(r.twos),
                std::to_wstring(r.fives), std::to_wstring(r.tens),
                std::to_wstring(r.twenties), std::to_wstring(r.fifties),
                std::to_wstring(r.hundreds), rdc::PlainMoney(r.coinCents),
                rdc::Money(r.totalCents)});
    };
    put(lvD, dep[i]);
    put(lvE, eod[i]);
    put(lvR, rst[i]);
  }
}

void RefreshComputed(HWND h) {
  auto r = rdc::ComputeRegister(gRegs[gActive], gBase);
  for (int i = 0; i < rdc::DenomCount; ++i) {
    SetDlgItemTextW(h, IDC_AMT0 + i, rdc::Money(r.counts.n[i] * rdc::kCents[i]).c_str());
    wchar_t db[16], lb[16];
    swprintf(db, 16, L"%d", r.drop.n[i]);
    swprintf(lb, 16, L"%d", r.left.n[i]);
    SetDlgItemTextW(h, IDC_DROP0 + i, db);
    SetDlgItemTextW(h, IDC_LEFT0 + i, lb);
  }
  SetDlgItemTextW(h, IDC_TOTAL_AMT, rdc::Money(r.amountCents).c_str());
  SetDlgItemTextW(h, IDC_TOTAL_DROP, rdc::Money(r.dropCents).c_str());
  SetDlgItemTextW(h, IDC_TOTAL_LEFT, rdc::Money(r.leftCents).c_str());
  wchar_t copyLab[32];
  swprintf(copyLab, 32, L"Copy R%d", gActive + 1);
  SetDlgItemTextW(h, IDC_COPY_R, copyLab);
  RefreshCashLog(h);

  int counted = 0, ok = 0, off = 0;
  for (int i = 0; i < rdc::kRegisterCount; ++i) {
    auto x = rdc::ComputeRegister(gRegs[i], gBase);
    if (!x.hasCount) continue;
    ++counted;
    if (x.balanced)
      ++ok;
    else
      ++off;
  }
  wchar_t st[192];
  swprintf(st, 192,
           L"%d counted   %d on base   %d off base   |   Yellow cells are counts. Left total is green when it equals the register base.",
           counted, ok, off);
  Status(h, st);
  InvalidateRect(GetDlgItem(h, IDC_TOTAL_LEFT), nullptr, TRUE);
  for (int i = 0; i < rdc::DenomCount; ++i)
    InvalidateRect(GetDlgItem(h, IDC_DROP0 + i), nullptr, TRUE);
}

void LoadCountsIntoEdits(HWND h) {
  gRefreshing = true;
  auto r = rdc::ComputeRegister(gRegs[gActive], gBase);
  for (int i = 0; i < rdc::DenomCount; ++i)
    SetEditInt(h, IDC_COUNT0 + i, r.counts.n[i]);
  gRefreshing = false;
  RefreshComputed(h);
}

void ReadCountsFromEdits(HWND h) {
  for (int i = 0; i < rdc::DenomCount; ++i)
    gRegs[gActive].n[i] = GetEditInt(h, IDC_COUNT0 + i);
}

HWND MakeStatic(HWND p, int id, const wchar_t* t, int x, int y, int w, int h, int align = SS_CENTER) {
  HWND s = CreateWindowW(L"STATIC", t, WS_CHILD | WS_VISIBLE | align, x, y, w, h, p,
                         (HMENU)(INT_PTR)id, gInst, nullptr);
  SendMessageW(s, WM_SETFONT, (WPARAM)gFont, TRUE);
  return s;
}

void FocusCount(HWND parent, int denomIndex) {
  HWND e = GetDlgItem(parent, IDC_COUNT0 + denomIndex);
  if (!e) return;
  SetFocus(e);
  SendMessageW(e, EM_SETSEL, 0, -1);
}

void MoveCountFocus(HWND parent, int id, bool back) {
  const int i = id - IDC_COUNT0;
  ReadCountsFromEdits(parent);
  SaveState();
  if (!back) {
    if (i + 1 < rdc::DenomCount) {
      FocusCount(parent, i + 1);
      return;
    }
    FocusCount(parent, 0);
    return;
  }
  if (i > 0) {
    FocusCount(parent, i - 1);
    return;
  }
  FocusCount(parent, rdc::DenomCount - 1);
}

void FocusNextTab(HWND root, bool back) {
  HWND focus = GetFocus();
  HWND next = GetNextDlgTabItem(root, focus, back ? TRUE : FALSE);
  if (!next) return;
  SetFocus(next);
  wchar_t cls[32]{};
  GetClassNameW(next, cls, 32);
  if (_wcsicmp(cls, L"EDIT") == 0) SendMessageW(next, EM_SETSEL, 0, -1);
}

LRESULT CALLBACK EditProc(HWND h, UINT m, WPARAM w, LPARAM l) {
  const bool nav = w == VK_RETURN || w == VK_TAB || w == VK_LEFT || w == VK_RIGHT ||
                   w == VK_UP || w == VK_DOWN;
  if (m == WM_GETDLGCODE && nav) {
    return DLGC_WANTALLKEYS | DLGC_WANTARROWS | DLGC_HASSETSEL;
  }
  if (m == WM_KEYDOWN && nav) {
    const bool back =
        w == VK_LEFT || w == VK_UP ||
        ((w == VK_RETURN || w == VK_TAB) && (GetKeyState(VK_SHIFT) & 0x8000) != 0);
    HWND parent = GetParent(h);
    const int id = GetDlgCtrlID(h);
    if (id >= IDC_COUNT0 && id < IDC_COUNT0 + rdc::DenomCount)
      MoveCountFocus(parent, id, back);
    else
      FocusNextTab(parent, back);
    return 0;
  }
  return CallWindowProcW(gOldEditProc, h, m, w, l);
}

HWND MakeEdit(HWND p, int id, int x, int y, int w, int h) {
  HWND e = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                           WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER | ES_CENTER,
                           x, y, w, h, p, (HMENU)(INT_PTR)id, gInst, nullptr);
  SendMessageW(e, WM_SETFONT, (WPARAM)gMono, TRUE);
  Untheme(e);
  gOldEditProc = (WNDPROC)SetWindowLongPtrW(e, GWLP_WNDPROC, (LONG_PTR)EditProc);
  return e;
}

HWND MakeBtn(HWND p, int id, const wchar_t* t, int x, int y, int w, int h) {
  HWND b = CreateWindowW(L"BUTTON", t, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW, x, y, w,
                         h, p, (HMENU)(INT_PTR)id, gInst, nullptr);
  SendMessageW(b, WM_SETFONT, (WPARAM)gFont, TRUE);
  Untheme(b);
  return b;
}

HWND MakeEditText(HWND p, int id, int x, int y, int w, int h) {
  HWND e = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                           WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOHSCROLL, x, y, w, h,
                           p, (HMENU)(INT_PTR)id, gInst, nullptr);
  SendMessageW(e, WM_SETFONT, (WPARAM)gMono, TRUE);
  Untheme(e);
  return e;
}

void Place(HDWP* dwp, HWND p, int id, int x, int y, int w, int h) {
  HWND c = GetDlgItem(p, id);
  if (!c || !*dwp) return;
  *dwp = DeferWindowPos(*dwp, c, nullptr, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

void SizeLvCols(HWND lv, const int* parts, int n) {
  if (!lv) return;
  RECT r{};
  GetClientRect(lv, &r);
  int w = r.right - GetSystemMetrics(SM_CXVSCROLL);
  if (w < 80) w = r.right;
  int sum = 0;
  for (int i = 0; i < n; ++i) sum += parts[i];
  if (sum <= 0) return;
  int used = 0;
  for (int i = 0; i < n - 1; ++i) {
    int cw = (int)((long long)w * parts[i] / sum);
    if (cw < 24) cw = 24;
    ListView_SetColumnWidth(lv, i, cw);
    used += cw;
  }
  ListView_SetColumnWidth(lv, n - 1, rdc::MaxI(40, w - used));
}

void RecreateFonts() {
  const bool dark = gOptions.darkMode;
  if (gFont && gFontBold && gMono && gFontPx == 15 && gFontDark == dark) return;
  const int q = dark ? ANTIALIASED_QUALITY : CLEARTYPE_QUALITY;
  HFONT ui = CreateFontW(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, q,
                         VARIABLE_PITCH | FF_SWISS, L"Segoe UI");
  HFONT bold = CreateFontW(-16, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, q,
                           VARIABLE_PITCH | FF_SWISS, L"Segoe UI");
  HFONT mono = CreateFontW(-15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, q,
                           FIXED_PITCH | FF_MODERN, L"Consolas");
  if (gFont) DeleteObject(gFont);
  if (gFontBold) DeleteObject(gFontBold);
  if (gMono) DeleteObject(gMono);
  gFont = ui;
  gFontBold = bold;
  gMono = mono;
  gFontPx = 15;
  gFontDark = dark;
}

void ApplyMainFonts(HWND h) {
  auto set = [&](int id, HFONT f) {
    HWND c = GetDlgItem(h, id);
    if (c) SendMessageW(c, WM_SETFONT, (WPARAM)f, TRUE);
  };
  set(IDC_TITLE, gFontBold);
  set(IDC_TODAY, gFontBold);
  set(IDC_LOGTITLE, gFontBold);
  set(IDC_TOTAL_AMT, gFontBold);
  set(IDC_TOTAL_DROP, gFontBold);
  set(IDC_TOTAL_LEFT, gFontBold);
  set(IDC_TOTAL_LBL, gFontBold);
  set(IDC_BASE, gFont);
  set(IDC_BASELBL, gFont);
  set(IDC_CLEAR, gFont);
  set(IDC_UNDO, gFont);
  set(IDC_HISTORY, gFont);
  set(IDC_OPTIONS, gFont);
  set(IDC_ABOUT, gFont);
  set(IDC_TABS, gFont);
  set(IDC_CLEAR_REG, gFont);
  set(IDC_SLIP, gFont);
  set(IDC_TAB_EDIT, gFont);
  set(IDC_COPY_R, gFont);
  set(IDC_COPY_DEP, gFont);
  set(IDC_COPY_EOD, gFont);
  set(IDC_COPY_RST, gFont);
  set(IDC_COPY_TOT, gFont);
  set(IDC_DEP_LBL, gFont);
  set(IDC_EOD_LBL, gFont);
  set(IDC_RST_LBL, gFont);
  set(IDC_STATUS, gFont);
  for (int i = 0; i < 6; ++i) set(IDC_HDR0 + i, gFont);
  for (int i = 0; i < rdc::DenomCount; ++i) {
    set(IDC_BADGE0 + i, gFont);
    set(IDC_COUNT0 + i, gMono);
    set(IDC_DENOM0 + i, gMono);
    set(IDC_AMT0 + i, gMono);
    set(IDC_DROP0 + i, gMono);
    set(IDC_LEFT0 + i, gMono);
  }
  set(IDC_LV_DEP, gMono);
  set(IDC_LV_EOD, gMono);
  set(IDC_LV_RST, gMono);
}

void Relayout(HWND h) {
  RECT rc{};
  GetClientRect(h, &rc);
  const int W = rc.right;
  const int H = rc.bottom;
  if (W < 200 || H < 160) return;

  const int pad = rdc::MaxI(8, W / 140);
  const int btnH = rdc::MaxI(22, rdc::MinI(30, H / 26));
  const bool twoLine = W < 1520;
  const int headerH = twoLine ? (btnH * 2 + 18) : (btnH + 16);
  gHeaderH = headerH;
  const int statusH = rdc::MaxI(20, H / 34);
  RecreateFonts();
  ApplyMainFonts(h);

  int leftW = (W - pad * 3) * 47 / 100;
  if (leftW < 430) leftW = rdc::MinI(430, W * 48 / 100);
  if (leftW > W - 360) leftW = rdc::MaxI(360, W - 360);
  const int rightX = pad + leftW + pad;
  const int rightW = rdc::MaxI(280, W - rightX - pad);

  const int yTab = headerH + 4;
  const int tabH = rdc::MaxI(24, btnH);
  const int yGrid = yTab + tabH + 6;
  const int yStatus = H - statusH - 4;
  const int gridBottom = yStatus - 6;
  const int rows = rdc::DenomCount + 2;
  int rowH = (gridBottom - yGrid) / rows;
  if (rowH < 18) rowH = 18;
  if (rowH > 32) rowH = 32;

  const int colW[6] = {leftW * 8 / 100, leftW * 16 / 100, leftW * 16 / 100,
                       leftW * 22 / 100, leftW * 18 / 100, leftW * 20 / 100};
  int colX[6];
  colX[0] = pad;
  for (int i = 1; i < 6; ++i) colX[i] = colX[i - 1] + colW[i - 1];

  HDWP dwp = BeginDeferWindowPos(140);
  if (!dwp) return;

  Place(&dwp, h, IDC_TITLE, pad, 6, rdc::MinI(260, W / 3), btnH);
  Place(&dwp, h, IDC_TODAY, W - pad - rdc::MinI(280, W / 3), 6, rdc::MinI(280, W / 3), btnH);
  int bx = twoLine ? pad : pad + 268;
  const int by = twoLine ? (btnH + 10) : 6;
  Place(&dwp, h, IDC_BASELBL, bx, by, 96, btnH);
  bx += 100;
  Place(&dwp, h, IDC_BASE, bx, by - 2, 110, 220);
  bx += 118;
  Place(&dwp, h, IDC_CLEAR, bx, by, 90, btnH);
  bx += 96;
  Place(&dwp, h, IDC_UNDO, bx, by, 92, btnH);
  bx += 98;
  Place(&dwp, h, IDC_HISTORY, bx, by, 80, btnH);

  Place(&dwp, h, IDC_TABS, pad, yTab, leftW - 240, tabH);
  Place(&dwp, h, IDC_SLIP, pad + leftW - 236, yTab, 76, tabH);
  Place(&dwp, h, IDC_CLEAR_REG, pad + leftW - 154, yTab, 64, tabH);
  Place(&dwp, h, IDC_COPY_R, pad + leftW - 86, yTab, 86, tabH);

  const int yHdr = yGrid;
  for (int i = 0; i < 6; ++i)
    Place(&dwp, h, IDC_HDR0 + i, colX[i], yHdr, colW[i], rdc::MaxI(16, rowH - 4));

  const int y0 = yHdr + rdc::MaxI(16, rowH - 4) + 2;
  const int rollGap = 3;
  for (int i = 0; i < rdc::DenomCount; ++i) {
    int y = y0 + i * rowH;
    if (i >= rdc::PRoll) y += rollGap;
    if (i > rdc::QRoll) y += rollGap;
    Place(&dwp, h, IDC_BADGE0 + i, colX[0], y, colW[0], rowH - 2);
    Place(&dwp, h, IDC_COUNT0 + i, colX[1], y, colW[1], rowH - 2);
    Place(&dwp, h, IDC_DENOM0 + i, colX[2], y, colW[2], rowH - 2);
    Place(&dwp, h, IDC_AMT0 + i, colX[3], y, colW[3], rowH - 2);
    Place(&dwp, h, IDC_DROP0 + i, colX[4], y, colW[4], rowH - 2);
    Place(&dwp, h, IDC_LEFT0 + i, colX[5], y, colW[5], rowH - 2);
  }
  const int ty = y0 + rdc::DenomCount * rowH + 2 + rollGap * 2;
  const int rollTop = y0 + rdc::PRoll * rowH + rollGap;
  const int rollBot = y0 + (rdc::QRoll + 1) * rowH + rollGap - 2;
  gRollBox = {colX[0] - 2, rollTop - 3, colX[5] + colW[5] + 2, rollBot + 3};
  Place(&dwp, h, IDC_TOTAL_LBL, colX[0], ty, colW[0] + colW[1] + colW[2], rowH);
  Place(&dwp, h, IDC_TOTAL_AMT, colX[3], ty, colW[3], rowH);
  Place(&dwp, h, IDC_TOTAL_DROP, colX[4], ty, colW[4], rowH);
  Place(&dwp, h, IDC_TOTAL_LEFT, colX[5], ty, colW[5], rowH);

  const int cy = yTab;
  const int copyW = rdc::MinI(118, rightW / 5);
  Place(&dwp, h, IDC_LOGTITLE, rightX, cy, rdc::MaxI(140, rightW - copyW * 3 - 16), 24);
  Place(&dwp, h, IDC_COPY_DEP, rightX + rightW - copyW * 3 - 12, cy, copyW, 24);
  Place(&dwp, h, IDC_COPY_EOD, rightX + rightW - copyW * 2 - 6, cy, copyW, 24);
  Place(&dwp, h, IDC_COPY_RST, rightX + rightW - copyW, cy, copyW, 24);

  const int logTop = cy + 30;
  const int logH = rdc::MaxI(80, yStatus - logTop - 8);
  const int block = logH / 3;
  const int capH = 20;
  const int lvH = rdc::MaxI(48, block - capH - 8);

  Place(&dwp, h, IDC_DEP_LBL, rightX, logTop, 160, capH);
  Place(&dwp, h, IDC_COPY_TOT, rightX + 170, logTop, 100, capH + 2);
  Place(&dwp, h, IDC_LV_DEP, rightX, logTop + capH + 2, rightW, lvH);

  int y2 = logTop + block;
  Place(&dwp, h, IDC_EOD_LBL, rightX, y2, rightW, capH);
  Place(&dwp, h, IDC_LV_EOD, rightX, y2 + capH + 2, rightW, lvH);

  int y3 = logTop + block * 2;
  Place(&dwp, h, IDC_RST_LBL, rightX, y3, rightW, capH);
  Place(&dwp, h, IDC_LV_RST, rightX, y3 + capH + 2, rightW, lvH);

  Place(&dwp, h, IDC_STATUS, pad, yStatus, W - pad * 2, statusH);
  EndDeferWindowPos(dwp);

  const int logParts[] = {48, 40, 36, 40, 44, 48, 44, 52, 56, 80};
  SizeLvCols(GetDlgItem(h, IDC_LV_DEP), logParts, 10);
  SizeLvCols(GetDlgItem(h, IDC_LV_EOD), logParts, 10);
  SizeLvCols(GetDlgItem(h, IDC_LV_RST), logParts, 10);
  EnableUndoBtn(h);
  InvalidateRect(h, nullptr, TRUE);
}

#include "AppWindows.inc"
