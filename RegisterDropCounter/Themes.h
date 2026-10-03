#pragma once
// Named color themes. Count cells stay amber. Drop slips stay black on white.

#include <string>

namespace rdc {

struct ThemePalette {
  const wchar_t* id;
  const wchar_t* name;
  bool dark;
  COLORREF navy, navyMid, navyDeep, onNavy, navyFg;
  COLORREF sheet, paper, ink, muted, grid;
  COLORREF input, computed, ok, bad, dropHit, cellInk, okInk, badInk;
};

inline const ThemePalette kThemes[] = {
    {L"classic", L"Classic Navy", false, RGB(31, 78, 121), RGB(46, 117, 182), RGB(22, 58, 95),
     RGB(247, 251, 255), RGB(31, 78, 121), RGB(238, 241, 244), RGB(255, 255, 255), RGB(26, 36, 46),
     RGB(92, 107, 122), RGB(197, 208, 219), RGB(255, 244, 194), RGB(248, 228, 212), RGB(198, 239, 206),
     RGB(255, 199, 206), RGB(214, 234, 223), RGB(26, 36, 46), RGB(0, 97, 0), RGB(156, 0, 6)},
    {L"cobalt", L"Cobalt Night", true, RGB(26, 51, 82), RGB(46, 85, 128), RGB(21, 40, 68),
     RGB(231, 246, 242), RGB(110, 231, 212), RGB(27, 42, 61), RGB(36, 54, 76), RGB(231, 246, 242),
     RGB(143, 179, 174), RGB(60, 85, 112), RGB(245, 197, 66), RGB(30, 48, 68), RGB(13, 63, 53),
     RGB(77, 31, 40), RGB(21, 86, 74), RGB(28, 20, 6), RGB(110, 240, 200), RGB(255, 176, 188)},
    {L"forest", L"Evergreen", false, RGB(27, 77, 62), RGB(47, 125, 98), RGB(20, 56, 46),
     RGB(244, 251, 247), RGB(27, 77, 62), RGB(238, 246, 241), RGB(255, 255, 255), RGB(26, 42, 36),
     RGB(92, 114, 104), RGB(195, 217, 204), RGB(255, 244, 194), RGB(243, 230, 212), RGB(198, 239, 206),
     RGB(255, 199, 206), RGB(214, 234, 223), RGB(26, 36, 46), RGB(0, 97, 0), RGB(156, 0, 6)},
    {L"pine", L"Pine Night", true, RGB(26, 61, 50), RGB(47, 107, 86), RGB(19, 46, 38),
     RGB(231, 246, 238), RGB(158, 230, 200), RGB(20, 36, 30), RGB(29, 51, 42), RGB(231, 246, 238),
     RGB(156, 184, 170), RGB(61, 92, 78), RGB(245, 197, 66), RGB(36, 56, 47), RGB(13, 63, 53),
     RGB(77, 31, 40), RGB(26, 77, 60), RGB(28, 20, 6), RGB(110, 240, 200), RGB(255, 176, 188)},
    {L"burgundy", L"Burgundy", false, RGB(122, 36, 56), RGB(163, 61, 86), RGB(92, 26, 42),
     RGB(255, 247, 248), RGB(122, 36, 56), RGB(247, 240, 242), RGB(255, 255, 255), RGB(42, 28, 34),
     RGB(122, 101, 112), RGB(228, 207, 214), RGB(255, 244, 194), RGB(248, 228, 212), RGB(198, 239, 206),
     RGB(255, 199, 206), RGB(214, 234, 223), RGB(26, 36, 46), RGB(0, 97, 0), RGB(156, 0, 6)},
    {L"wine", L"Wine Night", true, RGB(74, 32, 48), RGB(122, 58, 80), RGB(52, 22, 34),
     RGB(251, 239, 242), RGB(240, 180, 196), RGB(36, 22, 28), RGB(50, 32, 40), RGB(251, 239, 242),
     RGB(196, 168, 176), RGB(92, 58, 72), RGB(245, 197, 66), RGB(58, 40, 48), RGB(13, 63, 53),
     RGB(77, 31, 40), RGB(21, 86, 74), RGB(28, 20, 6), RGB(110, 240, 200), RGB(255, 176, 188)},
    {L"ocean", L"Lagoon", false, RGB(14, 107, 122), RGB(26, 154, 171), RGB(10, 78, 89),
     RGB(243, 251, 252), RGB(14, 107, 122), RGB(238, 247, 248), RGB(255, 255, 255), RGB(22, 48, 52),
     RGB(90, 114, 120), RGB(197, 221, 226), RGB(255, 244, 194), RGB(246, 230, 214), RGB(198, 239, 206),
     RGB(255, 199, 206), RGB(214, 234, 223), RGB(26, 36, 46), RGB(0, 97, 0), RGB(156, 0, 6)},
    {L"harbor", L"Harbor Night", true, RGB(14, 61, 72), RGB(26, 106, 120), RGB(10, 44, 52),
     RGB(231, 247, 248), RGB(126, 224, 232), RGB(16, 40, 46), RGB(24, 56, 64), RGB(231, 247, 248),
     RGB(158, 196, 200), RGB(47, 92, 102), RGB(245, 197, 66), RGB(28, 64, 72), RGB(13, 63, 53),
     RGB(77, 31, 40), RGB(21, 86, 74), RGB(28, 20, 6), RGB(110, 240, 200), RGB(255, 176, 188)},
    {L"plum", L"Plum", false, RGB(92, 61, 122), RGB(125, 90, 163), RGB(67, 44, 92),
     RGB(250, 247, 252), RGB(92, 61, 122), RGB(244, 240, 248), RGB(255, 255, 255), RGB(38, 28, 48),
     RGB(110, 101, 120), RGB(216, 207, 230), RGB(255, 244, 194), RGB(246, 228, 216), RGB(198, 239, 206),
     RGB(255, 199, 206), RGB(214, 234, 223), RGB(26, 36, 46), RGB(0, 97, 0), RGB(156, 0, 6)},
    {L"ink", L"Violet Ink", true, RGB(58, 42, 88), RGB(92, 69, 136), RGB(40, 28, 64),
     RGB(244, 239, 252), RGB(212, 196, 245), RGB(28, 22, 40), RGB(40, 32, 54), RGB(244, 239, 252),
     RGB(184, 168, 204), RGB(74, 60, 100), RGB(245, 197, 66), RGB(50, 40, 72), RGB(13, 63, 53),
     RGB(77, 31, 40), RGB(21, 86, 74), RGB(28, 20, 6), RGB(110, 240, 200), RGB(255, 176, 188)},
    {L"sunrise", L"Sunrise", false, RGB(154, 74, 28), RGB(196, 106, 50), RGB(110, 52, 20),
     RGB(255, 248, 243), RGB(154, 74, 28), RGB(251, 244, 236), RGB(255, 253, 248), RGB(44, 34, 24),
     RGB(122, 104, 92), RGB(230, 212, 196), RGB(255, 244, 194), RGB(248, 224, 200), RGB(198, 239, 206),
     RGB(255, 199, 206), RGB(214, 234, 223), RGB(26, 36, 46), RGB(0, 97, 0), RGB(156, 0, 6)},
    {L"slate", L"Slate", false, RGB(61, 76, 92), RGB(90, 112, 132), RGB(44, 56, 68),
     RGB(247, 249, 251), RGB(61, 76, 92), RGB(232, 236, 239), RGB(255, 255, 255), RGB(30, 38, 46),
     RGB(102, 112, 122), RGB(197, 206, 214), RGB(255, 244, 194), RGB(243, 228, 214), RGB(198, 239, 206),
     RGB(255, 199, 206), RGB(214, 234, 223), RGB(26, 36, 46), RGB(0, 97, 0), RGB(156, 0, 6)},
};

inline constexpr int kThemeCount = (int)(sizeof(kThemes) / sizeof(kThemes[0]));

inline const ThemePalette& PaletteFor(const std::wstring& id) {
  for (int i = 0; i < kThemeCount; ++i) {
    if (id == kThemes[i].id) return kThemes[i];
  }
  return kThemes[0];
}

inline std::wstring SanitizeTheme(const std::wstring& raw, bool dark) {
  std::wstring s;
  for (wchar_t c : raw) {
    if ((c >= L'a' && c <= L'z') || (c >= L'0' && c <= L'9')) s.push_back(c);
    if (s.size() >= 16) break;
  }
  for (int i = 0; i < kThemeCount; ++i) {
    if (s == kThemes[i].id) return s;
  }
  return dark ? L"cobalt" : L"classic";
}

}  // namespace rdc
