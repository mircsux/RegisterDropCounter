#pragma once
// Integer-cent drop engine matching RegisterDropCounter-08102026.xlsx, plus $2 bills.
// Drop order: $100 → $50 → $20 → $10 → $5 → $2 → $1 → Q → D → N → Q-roll → D-roll → N-roll → P-roll → pennies.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cwchar>
#include <sstream>
#include <string>
#include <vector>

namespace rdc {

enum Denom : int {
  Penny = 0,
  Nickel,
  Dime,
  Quarter,
  PRoll,
  NRoll,
  DRoll,
  QRoll,
  One,
  Two,
  Five,
  Ten,
  Twenty,
  Fifty,
  Hundred,
  DenomCount
};

inline constexpr int kCents[DenomCount] = {
    1, 5, 10, 25, 50, 200, 500, 1000, 100, 200, 500, 1000, 2000, 5000, 10000};

inline constexpr const wchar_t* kLabels[DenomCount] = {
    L"0.01", L"0.05", L"0.10", L"0.25", L"0.50", L"2.00", L"5.00", L"10.00",
    L"1",    L"2",    L"5",    L"10",   L"20",   L"50",   L"100"};

inline constexpr const wchar_t* kRollLetter[DenomCount] = {
    L"", L"", L"", L"", L"P", L"N", L"D", L"Q", L"", L"", L"", L"", L"", L"", L""};

inline constexpr int kDropOrder[DenomCount] = {
    Hundred, Fifty, Twenty, Ten, Five, Two, One, Quarter, Dime,
    Nickel,  QRoll, DRoll,  NRoll, PRoll, Penny};

inline constexpr int kRegisterCount = 10;
inline constexpr int kBaseOptions[] = {100, 200, 300, 400, 500};

struct Counts {
  int n[DenomCount]{};
};

struct RegisterResult {
  Counts counts{};
  Counts drop{};
  Counts left{};
  int amountCents = 0;
  int dropCents = 0;
  int leftCents = 0;
  bool balanced = false;
  bool hasCount = false;
};

inline int ClampCount(int v) {
  if (v < 0) return 0;
  if (v > 99999) return 99999;
  return v;
}

inline RegisterResult ComputeRegister(const Counts& in, int baseDollars) {
  RegisterResult r;
  r.counts = in;
  for (int i = 0; i < DenomCount; ++i) r.counts.n[i] = ClampCount(r.counts.n[i]);

  const int baseCents = baseDollars * 100;
  for (int i = 0; i < DenomCount; ++i) {
    r.amountCents += r.counts.n[i] * kCents[i];
    if (r.counts.n[i] > 0) r.hasCount = true;
  }

  int remaining = (std::max)(0, r.amountCents - baseCents);
  for (int o = 0; o < DenomCount; ++o) {
    const int key = kDropOrder[o];
    const int cents = kCents[key];
    const int available = r.counts.n[key];
    const int need = remaining / cents;
    const int d = (std::max)(0, (std::min)(available, need));
    r.drop.n[key] = d;
    r.left.n[key] = available - d;
    remaining -= d * cents;
  }
  for (int i = 0; i < DenomCount; ++i) {
    r.dropCents += r.drop.n[i] * kCents[i];
    r.leftCents += r.left.n[i] * kCents[i];
  }
  r.balanced = (r.leftCents == baseCents);
  return r;
}

inline int LooseCoinCents(const Counts& c) {
  return c.n[Penny] * kCents[Penny] + c.n[Nickel] * kCents[Nickel] +
         c.n[Dime] * kCents[Dime] + c.n[Quarter] * kCents[Quarter];
}

inline int CoinAndRollCents(const Counts& c) {
  int s = LooseCoinCents(c);
  s += c.n[PRoll] * kCents[PRoll] + c.n[NRoll] * kCents[NRoll] +
       c.n[DRoll] * kCents[DRoll] + c.n[QRoll] * kCents[QRoll];
  return s;
}

inline int BillCents(const Counts& c) {
  return c.n[One] * 100 + c.n[Two] * 200 + c.n[Five] * 500 + c.n[Ten] * 1000 +
         c.n[Twenty] * 2000 + c.n[Fifty] * 5000 + c.n[Hundred] * 10000;
}

inline std::wstring Money(int cents) {
  const int a = std::abs(cents);
  wchar_t buf[64];
  if (cents < 0)
    swprintf(buf, 64, L"-$%d.%02d", a / 100, a % 100);
  else
    swprintf(buf, 64, L"$%d.%02d", a / 100, a % 100);
  return buf;
}

inline std::wstring PlainMoney(int cents) {
  const int a = std::abs(cents);
  wchar_t buf[32];
  if (cents < 0)
    swprintf(buf, 32, L"-%d.%02d", a / 100, a % 100);
  else
    swprintf(buf, 32, L"%d.%02d", a / 100, a % 100);
  return buf;
}

struct LogRow {
  int ones = 0, twos = 0, fives = 0, tens = 0, twenties = 0, fifties = 0, hundreds = 0;
  int coinCents = 0;
  int totalCents = 0;
};

inline LogRow DepositRow(const RegisterResult& r) {
  LogRow o;
  o.ones = r.drop.n[One];
  o.twos = r.drop.n[Two];
  o.fives = r.drop.n[Five];
  o.tens = r.drop.n[Ten];
  o.twenties = r.drop.n[Twenty];
  o.fifties = r.drop.n[Fifty];
  o.hundreds = r.drop.n[Hundred];
  o.coinCents = LooseCoinCents(r.drop);
  o.totalCents = BillCents(r.drop) + o.coinCents;
  return o;
}

inline LogRow EodRow(const RegisterResult& r) {
  LogRow o;
  o.ones = r.counts.n[One];
  o.twos = r.counts.n[Two];
  o.fives = r.counts.n[Five];
  o.tens = r.counts.n[Ten];
  o.twenties = r.counts.n[Twenty];
  o.fifties = r.counts.n[Fifty];
  o.hundreds = r.counts.n[Hundred];
  o.coinCents = CoinAndRollCents(r.counts);
  o.totalCents = BillCents(r.counts) + o.coinCents;
  return o;
}

inline LogRow ResetRow(const RegisterResult& r, int baseDollars) {
  LogRow o;
  o.ones = r.left.n[One];
  o.twos = r.left.n[Two];
  o.fives = r.left.n[Five];
  o.tens = r.left.n[Ten];
  o.twenties = r.left.n[Twenty];
  o.fifties = r.left.n[Fifty];
  o.hundreds = r.left.n[Hundred];
  o.coinCents = r.hasCount ? baseDollars * 100 - BillCents(r.left) : 0;
  o.totalCents = r.leftCents;
  return o;
}

inline std::wstring OneRowTsv(const LogRow& r) {
  std::wstringstream ss;
  ss << r.ones << L'\t' << r.twos << L'\t' << r.fives << L'\t' << r.tens << L'\t'
     << r.twenties << L'\t' << r.fifties << L'\t' << r.hundreds << L'\t'
     << PlainMoney(r.coinCents);
  return ss.str();
}

inline std::wstring AreaRowTsv(const wchar_t* label, const LogRow& r) {
  std::wstringstream ss;
  ss << label << L'\t' << r.ones << L'\t' << r.twos << L'\t' << r.fives << L'\t' << r.tens
     << L'\t' << r.twenties << L'\t' << r.fifties << L'\t' << r.hundreds << L'\t'
     << PlainMoney(r.coinCents) << L'\t' << PlainMoney(r.totalCents);
  return ss.str();
}

inline std::wstring OneDepositTsv(const LogRow& r) { return OneRowTsv(r); }

inline std::wstring DepositTsv(const std::vector<LogRow>& rows) {
  std::wstringstream ss;
  for (size_t i = 0; i < rows.size(); ++i) {
    ss << OneRowTsv(rows[i]);
    if (i + 1 < rows.size()) ss << L"\r\n";
  }
  return ss.str();
}

inline LogRow SumRows(const std::vector<LogRow>& rows) {
  LogRow t{};
  for (const auto& r : rows) {
    t.ones += r.ones;
    t.twos += r.twos;
    t.fives += r.fives;
    t.tens += r.tens;
    t.twenties += r.twenties;
    t.fifties += r.fifties;
    t.hundreds += r.hundreds;
    t.coinCents += r.coinCents;
    t.totalCents += r.totalCents;
  }
  return t;
}

/** All cells in one cash-log table (10 registers + totals, every column). */
inline std::wstring AreaTsv(const std::vector<LogRow>& rows) {
  std::wstringstream ss;
  for (size_t i = 0; i < rows.size(); ++i) {
    wchar_t lab[8];
    swprintf(lab, 8, L"R%d", (int)i + 1);
    ss << AreaRowTsv(lab, rows[i]) << L"\r\n";
  }
  ss << AreaRowTsv(L"Totals", SumRows(rows));
  return ss.str();
}

inline std::wstring TotalsTsv(const std::vector<LogRow>& rows) {
  return OneRowTsv(SumRows(rows));
}

inline std::wstring EodTsv(const std::vector<LogRow>& rows) { return DepositTsv(rows); }

inline std::wstring ResetTsv(const std::vector<LogRow>& rows) { return DepositTsv(rows); }

inline void LoadSample(Counts regs[kRegisterCount]) {
  for (int i = 0; i < kRegisterCount; ++i) regs[i] = Counts{};
  regs[0].n[Nickel] = 6;
  regs[0].n[Dime] = 48;
  regs[0].n[Quarter] = 27;
  regs[0].n[One] = 109;
  regs[0].n[Five] = 5;
  regs[0].n[Ten] = 10;
  regs[0].n[Twenty] = 103;
  regs[0].n[Fifty] = 6;
  regs[0].n[Hundred] = 12;

  regs[1].n[Nickel] = 51;
  regs[1].n[Dime] = 18;
  regs[1].n[Quarter] = 16;
  regs[1].n[One] = 100;
  regs[1].n[Five] = 2;
  regs[1].n[Ten] = 1;
  regs[1].n[Twenty] = 98;
  regs[1].n[Fifty] = 3;
  regs[1].n[Hundred] = 5;

  regs[2].n[Penny] = 5;
  regs[2].n[Nickel] = 63;
  regs[2].n[Dime] = 40;
  regs[2].n[One] = 89;
  regs[2].n[Five] = 19;
  regs[2].n[Ten] = 25;
  regs[2].n[Twenty] = 154;
  regs[2].n[Fifty] = 2;
  regs[2].n[Hundred] = 13;
}

}  // namespace rdc
