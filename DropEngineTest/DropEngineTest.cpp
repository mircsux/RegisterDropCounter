// Console tests for DropEngine.h — same sample data as RegisterDropCounter-08102026.xlsx.
#include "../RegisterDropCounter/DropEngine.h"
#include "../RegisterDropCounter/History.h"
#include <iostream>

static int gFails = 0;

static void Expect(long long got, long long want, const char* name) {
  if (got != want) {
    std::cerr << "FAIL " << name << " got " << got << " expected " << want << "\n";
    ++gFails;
  } else {
    std::cout << "ok   " << name << "\n";
  }
}

int main() {
  rdc::Counts r1{};
  r1.n[rdc::Nickel] = 6;
  r1.n[rdc::Dime] = 48;
  r1.n[rdc::Quarter] = 27;
  r1.n[rdc::One] = 109;
  r1.n[rdc::Five] = 5;
  r1.n[rdc::Ten] = 10;
  r1.n[rdc::Twenty] = 103;
  r1.n[rdc::Fifty] = 6;
  r1.n[rdc::Hundred] = 12;
  auto a = rdc::ComputeRegister(r1, 400);
  Expect(a.amountCents, 380585, "R1 amount");
  Expect(a.dropCents, 340585, "R1 drop");
  Expect(a.leftCents, 40000, "R1 left");
  Expect(a.drop.n[rdc::Five], 1, "R1 drop $5");
  Expect(a.drop.n[rdc::Twenty], 95, "R1 drop $20");
  Expect(a.drop.n[rdc::Hundred], 12, "R1 drop $100");
  Expect(a.left.n[rdc::One], 109, "R1 left $1");
  auto d1 = rdc::DepositRow(a);
  Expect(d1.ones, 0, "R1 deposit ones");
  Expect(d1.coinCents, 85, "R1 deposit coin");

  rdc::Counts r2{};
  r2.n[rdc::Nickel] = 51;
  r2.n[rdc::Dime] = 18;
  r2.n[rdc::Quarter] = 16;
  r2.n[rdc::One] = 100;
  r2.n[rdc::Five] = 2;
  r2.n[rdc::Ten] = 1;
  r2.n[rdc::Twenty] = 98;
  r2.n[rdc::Fifty] = 3;
  r2.n[rdc::Hundred] = 5;
  auto b = rdc::ComputeRegister(r2, 400);
  Expect(b.amountCents, 273835, "R2 amount");
  Expect(b.dropCents, 233835, "R2 drop");
  Expect(b.leftCents, 40000, "R2 left");

  rdc::Counts r3{};
  r3.n[rdc::Penny] = 5;
  r3.n[rdc::Nickel] = 63;
  r3.n[rdc::Dime] = 40;
  r3.n[rdc::One] = 89;
  r3.n[rdc::Five] = 19;
  r3.n[rdc::Ten] = 25;
  r3.n[rdc::Twenty] = 154;
  r3.n[rdc::Fifty] = 2;
  r3.n[rdc::Hundred] = 13;
  auto c = rdc::ComputeRegister(r3, 400);
  Expect(c.amountCents, 492120, "R3 amount");
  Expect(c.dropCents, 452120, "R3 drop");
  Expect(c.leftCents, 40000, "R3 left");

  rdc::Counts pennyRoll{};
  pennyRoll.n[rdc::Twenty] = 20;
  pennyRoll.n[rdc::PRoll] = 1;
  auto p = rdc::ComputeRegister(pennyRoll, 400);
  Expect(p.drop.n[rdc::PRoll], 1, "penny roll drops before 50 pennies");
  Expect(p.drop.n[rdc::Penny], 0, "no loose pennies dropped when a roll covers it");
  Expect(p.leftCents, 40000, "penny-roll case left");

  rdc::Counts loose{};
  loose.n[rdc::Twenty] = 20;
  loose.n[rdc::Penny] = 50;
  auto q = rdc::ComputeRegister(loose, 400);
  Expect(q.drop.n[rdc::Penny], 50, "50 pennies drop when no roll");
  Expect(q.drop.n[rdc::PRoll], 0, "no penny roll to drop");

  rdc::Counts qpath{};
  qpath.n[rdc::Twenty] = 20;
  qpath.n[rdc::Quarter] = 4;
  qpath.n[rdc::QRoll] = 1;
  auto qp = rdc::ComputeRegister(qpath, 400);
  Expect(qp.drop.n[rdc::Quarter], 4, "loose quarters before Q roll");
  Expect(qp.drop.n[rdc::QRoll], 1, "Q roll after loose quarters");

  rdc::Counts twos{};
  twos.n[rdc::One] = 400;
  twos.n[rdc::Two] = 3;
  twos.n[rdc::Five] = 1;
  auto t2 = rdc::ComputeRegister(twos, 400);
  Expect(t2.amountCents, 41100, "$2 amount");
  Expect(t2.drop.n[rdc::Five], 1, "$5 drops before $2");
  Expect(t2.drop.n[rdc::Two], 3, "$2 bills drop");
  Expect(t2.drop.n[rdc::One], 0, "$1 stay when $2 covers the rest");
  Expect(t2.leftCents, 40000, "$2 case left");
  auto d2 = rdc::DepositRow(t2);
  Expect(d2.twos, 3, "deposit $2 column");
  auto d2e = rdc::EodRow(t2);
  Expect(d2e.twos, 3, "eod $2 column");

  auto area = rdc::AreaTsv({d1});
  Expect(area.rfind(L"R1\t", 0) == 0 ? 1 : 0, 1, "area tsv is deposit table only");
  Expect(area.find(L"Totals\t") != std::wstring::npos ? 1 : 0, 1, "area tsv includes totals row");

  rdc::Counts sheet[rdc::kRegisterCount]{};
  rdc::HistoryEntry snap;
  Expect(rdc::MakeSnapshot(&snap, rdc::HistAll, -1, 400, sheet) ? 1 : 0, 0, "history skips empty sheet");
  sheet[0].n[rdc::One] = 5;
  Expect(rdc::MakeSnapshot(&snap, rdc::HistAll, -1, 400, sheet) ? 1 : 0, 1, "history saves clear-all");
  Expect(rdc::MakeSnapshot(&snap, rdc::HistRegister, 4, 400, sheet) ? 1 : 0, 0,
         "history skips empty register");
  Expect(rdc::MakeSnapshot(&snap, rdc::HistRegister, 0, 400, sheet) ? 1 : 0, 1,
         "history saves cleared R1");
  Expect(snap.registerIndex, 0, "history stores register index");
  Expect(snap.kind, rdc::HistRegister, "history kind is register");

  rdc::HistoryEntry ha = snap;
  rdc::HistoryEntry hb = snap;
  hb.at = snap.at + 10;
  auto merged = rdc::MergeHistories({ha}, {ha, hb});
  Expect((int)merged.size(), 2, "history merge keeps unique snapshots");
  Expect(merged[0].at > snap.at ? 1 : 0, 1, "history merge newest first");

  if (gFails) {
    std::cerr << gFails << " test(s) failed\n";
    return 1;
  }
  std::cout << "All DropEngine tests passed.\n";
  return 0;
}
