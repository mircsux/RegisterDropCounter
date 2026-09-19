#pragma once
// Drop trends from History — same rules as the web Stats for Nerds sheet.

#include "DropEngine.h"
#include "History.h"

#include <algorithm>
#include <string>
#include <vector>

namespace rdc {

struct DropEvent {
  std::time_t at = 0;
  int registerIndex = 0;
  int dropCents = 0;
  int amountCents = 0;
  Counts drop{};
};

struct ChartItem {
  std::wstring label;
  int cents = 0;
};

struct NerdStats {
  int clearCount = 0;
  int tillCount = 0;
  int dayCount = 0;
  int totalDropCents = 0;
  int avgDropCents = 0;
  int medianDropCents = 0;
  int maxDropCents = 0;
  int weekDropCents = 0;
  int priorWeekDropCents = 0;
  int weekDeltaPct = 0;  // 0 if no prior week
  bool hasWeekDelta = false;
  std::vector<std::wstring> insights;
  std::vector<ChartItem> byDay;
  std::vector<ChartItem> byTill;
  std::vector<ChartItem> byWeekday;
  std::vector<ChartItem> byDenom;
};

inline const wchar_t* DenomChartLabel(int d) {
  switch (d) {
    case Hundred:
      return L"$100";
    case Fifty:
      return L"$50";
    case Twenty:
      return L"$20";
    case Ten:
      return L"$10";
    case Five:
      return L"$5";
    case Two:
      return L"$2";
    case One:
      return L"$1";
    case Quarter:
      return L"25¢";
    case Dime:
      return L"10¢";
    case Nickel:
      return L"5¢";
    case Penny:
      return L"1¢";
    case QRoll:
      return L"Q-roll";
    case DRoll:
      return L"D-roll";
    case NRoll:
      return L"N-roll";
    case PRoll:
      return L"P-roll";
    default:
      return L"?";
  }
}

inline std::wstring TillLabel(const std::wstring names[kRegisterCount], int i) {
  if (i >= 0 && i < kRegisterCount && !names[i].empty()) return names[i];
  wchar_t buf[16];
  swprintf(buf, 16, L"R%d", i + 1);
  return buf;
}

inline std::time_t StartOfLocalDay(std::time_t t) {
  std::tm tm = LocalTm(t);
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  std::time_t out = std::mktime(&tm);
  return out < 0 ? t : out;
}

inline std::wstring DayChartLabel(std::time_t t) {
  const std::tm tm = LocalTm(t);
  wchar_t buf[16];
  wcsftime(buf, 16, L"%b %d", &tm);
  return buf;
}

inline std::vector<DropEvent> DropEventsFromHistory(const std::vector<HistoryEntry>& hist) {
  std::vector<DropEvent> out;
  for (const auto& e : hist) {
    if (e.kind == HistRegister && e.registerIndex >= 0 && e.registerIndex < kRegisterCount) {
      auto r = ComputeRegister(e.registers[e.registerIndex], e.base);
      if (!r.hasCount || r.dropCents <= 0) continue;
      DropEvent ev;
      ev.at = e.at;
      ev.registerIndex = e.registerIndex;
      ev.dropCents = r.dropCents;
      ev.amountCents = r.amountCents;
      ev.drop = r.drop;
      out.push_back(ev);
    } else {
      for (int i = 0; i < kRegisterCount; ++i) {
        auto r = ComputeRegister(e.registers[i], e.base);
        if (!r.hasCount || r.dropCents <= 0) continue;
        DropEvent ev;
        ev.at = e.at;
        ev.registerIndex = i;
        ev.dropCents = r.dropCents;
        ev.amountCents = r.amountCents;
        ev.drop = r.drop;
        out.push_back(ev);
      }
    }
  }
  std::sort(out.begin(), out.end(), [](const DropEvent& a, const DropEvent& b) { return a.at < b.at; });
  return out;
}

inline NerdStats BuildNerdStats(const std::vector<HistoryEntry>& hist,
                                const std::wstring names[kRegisterCount],
                                std::time_t now = std::time(nullptr)) {
  NerdStats s;
  auto events = DropEventsFromHistory(hist);
  s.clearCount = (int)events.size();
  if (events.empty()) return s;

  std::vector<int> drops;
  int tillCents[kRegisterCount]{};
  int weekdayCents[7]{};
  int denomCents[DenomCount]{};
  struct DayAcc {
    std::wstring key;
    std::time_t day = 0;
    int cents = 0;
  };
  std::vector<DayAcc> days;

  auto findDay = [&](const std::wstring& key) -> DayAcc* {
    for (auto& d : days)
      if (d.key == key) return &d;
    return nullptr;
  };

  for (const auto& e : events) {
    s.totalDropCents += e.dropCents;
    drops.push_back(e.dropCents);
    tillCents[e.registerIndex] += e.dropCents;
    weekdayCents[LocalTm(e.at).tm_wday] += e.dropCents;
    for (int d = 0; d < DenomCount; ++d) {
      int c = e.drop.n[d] * kCents[d];
      if (c > 0) denomCents[d] += c;
    }
    const std::wstring key = DateKey(e.at);
    if (auto* day = findDay(key)) {
      day->cents += e.dropCents;
    } else {
      DayAcc acc;
      acc.key = key;
      acc.day = StartOfLocalDay(e.at);
      acc.cents = e.dropCents;
      days.push_back(acc);
    }
  }

  std::sort(drops.begin(), drops.end());
  s.avgDropCents = s.totalDropCents / s.clearCount;
  s.maxDropCents = drops.back();
  if (drops.size() % 2 == 0)
    s.medianDropCents = (drops[drops.size() / 2 - 1] + drops[drops.size() / 2]) / 2;
  else
    s.medianDropCents = drops[drops.size() / 2];

  const std::time_t first = StartOfLocalDay(events.front().at);
  const std::time_t last = StartOfLocalDay(events.back().at);
  std::tm cursor = LocalTm(first);
  cursor.tm_hour = 0;
  cursor.tm_min = 0;
  cursor.tm_sec = 0;
  cursor.tm_isdst = -1;
  for (;;) {
    std::time_t t = std::mktime(&cursor);
    if (t < 0) break;
    if (t > last) break;
    const std::wstring key = DateKey(t);
    ChartItem item;
    item.label = DayChartLabel(t);
    if (auto* day = findDay(key)) item.cents = day->cents;
    s.byDay.push_back(item);
    cursor.tm_mday += 1;
    if ((int)s.byDay.size() > 120) break;
  }
  s.dayCount = (int)days.size();

  for (int i = 0; i < kRegisterCount; ++i) {
    if (tillCents[i] <= 0) continue;
    s.byTill.push_back({TillLabel(names, i), tillCents[i]});
  }
  std::sort(s.byTill.begin(), s.byTill.end(),
            [](const ChartItem& a, const ChartItem& b) { return a.cents > b.cents; });
  s.tillCount = (int)s.byTill.size();

  static const wchar_t* kWd[] = {L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat"};
  for (int i = 0; i < 7; ++i) s.byWeekday.push_back({kWd[i], weekdayCents[i]});

  for (int d = 0; d < DenomCount; ++d) {
    if (denomCents[d] <= 0) continue;
    s.byDenom.push_back({DenomChartLabel(d), denomCents[d]});
  }
  std::sort(s.byDenom.begin(), s.byDenom.end(),
            [](const ChartItem& a, const ChartItem& b) { return a.cents > b.cents; });
  if (s.byDenom.size() > 8) s.byDenom.resize(8);

  const std::time_t today = StartOfLocalDay(now);
  const std::time_t weekStart = today - 6 * 24 * 60 * 60;
  const std::time_t priorStart = today - 13 * 24 * 60 * 60;
  for (const auto& e : events) {
    const std::time_t day = StartOfLocalDay(e.at);
    if (day >= weekStart) s.weekDropCents += e.dropCents;
    else if (day >= priorStart) s.priorWeekDropCents += e.dropCents;
  }
  if (s.priorWeekDropCents > 0) {
    s.hasWeekDelta = true;
    s.weekDeltaPct = (int)(((long long)s.weekDropCents - s.priorWeekDropCents) * 100 /
                           s.priorWeekDropCents);
  }

  if (!s.byTill.empty()) {
    const int pct = s.totalDropCents > 0 ? (int)((long long)s.byTill[0].cents * 100 / s.totalDropCents) : 0;
    wchar_t buf[160];
    swprintf(buf, 160, L"%s is the heavy till — %d%% of the drop.", s.byTill[0].label.c_str(), pct);
    s.insights.push_back(buf);
  }
  int topWd = 0;
  for (int i = 1; i < 7; ++i)
    if (s.byWeekday[i].cents > s.byWeekday[topWd].cents) topWd = i;
  if (s.byWeekday[topWd].cents > 0) {
    static const wchar_t* kFull[] = {L"Sunday", L"Monday", L"Tuesday", L"Wednesday",
                                     L"Thursday", L"Friday", L"Saturday"};
    wchar_t buf[160];
    swprintf(buf, 160, L"%s is the big day for cash pulled.", kFull[topWd]);
    s.insights.push_back(buf);
  }
  if (s.hasWeekDelta) {
    wchar_t buf[200];
    if (s.weekDeltaPct > 4)
      swprintf(buf, 200, L"This week is up %d%% vs the prior week.", s.weekDeltaPct);
    else if (s.weekDeltaPct < -4)
      swprintf(buf, 200, L"This week is down %d%% vs the prior week.", -s.weekDeltaPct);
    else
      swprintf(buf, 200, L"This week is about even with the prior week.");
    s.insights.push_back(buf);
  }
  int tw = 0, hu = 0;
  for (const auto& d : s.byDenom) {
    if (d.label == L"$20") tw = d.cents;
    if (d.label == L"$100") hu = d.cents;
  }
  if (tw >= hu && tw > 0) {
    const int pct = s.totalDropCents > 0 ? (int)((long long)tw * 100 / s.totalDropCents) : 0;
    wchar_t buf[180];
    swprintf(buf, 180, L"$20s are %d%% of what you pull.", pct);
    s.insights.push_back(buf);
  } else if (hu > 0) {
    const int pct = s.totalDropCents > 0 ? (int)((long long)hu * 100 / s.totalDropCents) : 0;
    wchar_t buf[180];
    swprintf(buf, 180, L"$100s are %d%% of the drop.", pct);
    s.insights.push_back(buf);
  }
  if (s.insights.size() > 4) s.insights.resize(4);
  return s;
}

}  // namespace rdc
