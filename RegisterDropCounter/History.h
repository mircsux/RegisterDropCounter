#pragma once
// Clear-history snapshots for Register Drop Counter (same behavior as the web History tab).

#include "DropEngine.h"

#include <ctime>
#include <fstream>
#include <string>
#include <vector>

namespace rdc {

inline constexpr int kHistoryLimit = 200;

enum HistoryKind : int { HistRegister = 0, HistAll = 1 };

struct HistoryEntry {
  std::time_t at = 0;
  HistoryKind kind = HistAll;
  int registerIndex = -1;  // 0-based when kind is HistRegister
  int base = 400;
  Counts registers[kRegisterCount]{};
};

inline bool RegisterHasCount(const Counts& c) {
  for (int i = 0; i < DenomCount; ++i)
    if (c.n[i] > 0) return true;
  return false;
}

inline bool SheetHasCount(const Counts regs[kRegisterCount]) {
  for (int i = 0; i < kRegisterCount; ++i)
    if (RegisterHasCount(regs[i])) return true;
  return false;
}

inline bool MakeSnapshot(HistoryEntry* out, HistoryKind kind, int registerIndex, int base,
                         const Counts regs[kRegisterCount]) {
  if (kind == HistRegister) {
    if (registerIndex < 0 || registerIndex >= kRegisterCount) return false;
    if (!RegisterHasCount(regs[registerIndex])) return false;
  } else if (!SheetHasCount(regs)) {
    return false;
  }
  out->at = std::time(nullptr);
  out->kind = kind;
  out->registerIndex = (kind == HistRegister) ? registerIndex : -1;
  out->base = base;
  for (int i = 0; i < kRegisterCount; ++i) out->registers[i] = regs[i];
  return true;
}

inline void PrependHistory(std::vector<HistoryEntry>* hist, const HistoryEntry& e) {
  hist->insert(hist->begin(), e);
  if ((int)hist->size() > kHistoryLimit) hist->resize(kHistoryLimit);
}

inline std::tm LocalTm(std::time_t t) {
  std::tm tm{};
#if defined(_WIN32)
  localtime_s(&tm, &t);
#else
  if (const std::tm* p = std::localtime(&t)) tm = *p;
#endif
  return tm;
}

inline std::wstring DateKey(std::time_t t) {
  const std::tm tm = LocalTm(t);
  wchar_t buf[16];
  wcsftime(buf, 16, L"%Y-%m-%d", &tm);
  return buf;
}

inline std::wstring TodayDateKey() { return DateKey(std::time(nullptr)); }

inline std::wstring FormatSheetDate(std::time_t t = std::time(nullptr)) {
  const std::tm tm = LocalTm(t);
  wchar_t buf[80];
  wcsftime(buf, 80, L"%A, %B %d, %Y", &tm);
  return buf;
}

inline std::wstring FormatWhen(std::time_t t) {
  const std::tm tm = LocalTm(t);
  wchar_t buf[80];
  wcsftime(buf, 80, L"%a %b %d, %Y  %I:%M %p", &tm);
  return buf;
}

inline std::wstring HistoryLabel(const HistoryEntry& e) {
  if (e.kind == HistAll) return L"Cleared all registers";
  wchar_t buf[48];
  swprintf(buf, 48, L"Cleared Register %d", e.registerIndex + 1);
  return buf;
}

struct HistorySummary {
  int counted = 0;
  int amountCents = 0;
  int dropCents = 0;
};

inline HistorySummary Summarize(const HistoryEntry& e) {
  HistorySummary s;
  for (int i = 0; i < kRegisterCount; ++i) {
    auto r = ComputeRegister(e.registers[i], e.base);
    if (r.hasCount) s.counted += 1;
    s.amountCents += r.amountCents;
    s.dropCents += r.dropCents;
  }
  return s;
}

inline std::vector<int> FilterByDate(const std::vector<HistoryEntry>& hist,
                                     const std::wstring& dateKey) {
  std::vector<int> idx;
  for (int i = 0; i < (int)hist.size(); ++i) {
    if (dateKey.empty() || DateKey(hist[i].at).rfind(dateKey, 0) == 0) idx.push_back(i);
  }
  return idx;
}

inline void SaveHistory(const std::wstring& path, const std::vector<HistoryEntry>& hist) {
#if defined(_WIN32)
  std::wofstream out(path.c_str());
  if (!out) return;
  out << L"RDCH1\n" << hist.size() << L"\n";
  for (const auto& e : hist) {
    out << (long long)e.at << L' ' << (int)e.kind << L' ' << e.registerIndex << L' ' << e.base
        << L'\n';
    for (int r = 0; r < kRegisterCount; ++r) {
      for (int d = 0; d < DenomCount; ++d) {
        if (d) out << L',';
        out << e.registers[r].n[d];
      }
      out << L'\n';
    }
  }
#else
  (void)path;
  (void)hist;
#endif
}

inline std::vector<int> ParseCountLine(const std::wstring& line) {
  std::vector<int> vals;
  size_t start = 0;
  for (;;) {
    const auto comma = line.find(L',', start);
    const auto tok =
        line.substr(start, comma == std::wstring::npos ? std::wstring::npos : comma - start);
    vals.push_back(ClampCount((int)std::wcstol(tok.c_str(), nullptr, 10)));
    if (comma == std::wstring::npos) break;
    start = comma + 1;
  }
  if (vals.size() == 14) vals.insert(vals.begin() + 9, 0);
  return vals;
}

inline bool LoadHistory(const std::wstring& path, std::vector<HistoryEntry>* hist) {
  hist->clear();
#if !defined(_WIN32)
  (void)path;
  return false;
#else
  std::wifstream in(path.c_str());
  if (!in) return false;
  std::wstring mag;
  std::getline(in, mag);
  if (mag != L"RDCH1") return false;
  int n = 0;
  in >> n;
  in.ignore(64, L'\n');
  if (n < 0) n = 0;
  if (n > kHistoryLimit) n = kHistoryLimit;
  for (int i = 0; i < n; ++i) {
    HistoryEntry e;
    long long at = 0;
    int kind = 1, reg = -1, base = 400;
    in >> at >> kind >> reg >> base;
    in.ignore(64, L'\n');
    e.at = (std::time_t)at;
    e.kind = (kind == 0) ? HistRegister : HistAll;
    e.registerIndex = (e.kind == HistRegister) ? reg : -1;
    e.base = base;
    if (e.base != 100 && e.base != 200 && e.base != 300 && e.base != 400 && e.base != 500)
      e.base = 400;
    for (int r = 0; r < kRegisterCount; ++r) {
      std::wstring line;
      if (!std::getline(in, line)) break;
      auto vals = ParseCountLine(line);
      for (int d = 0; d < DenomCount && d < (int)vals.size(); ++d) e.registers[r].n[d] = vals[d];
    }
    hist->push_back(e);
  }
  return true;
#endif
}

}  // namespace rdc
