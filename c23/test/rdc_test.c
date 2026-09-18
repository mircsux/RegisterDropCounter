#include "rdc_store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail;

static void expect_int(const char *name, int got, int want) {
  if (got == want) return;
  fprintf(stderr, "FAIL %s: got %d want %d\n", name, got, want);
  g_fail = 1;
}

static void expect_str(const char *name, const char *got, const char *want) {
  if (got && want && strcmp(got, want) == 0) return;
  fprintf(stderr, "FAIL %s: got '%s' want '%s'\n", name, got ? got : "(null)", want ? want : "(null)");
  g_fail = 1;
}

int main(void) {
  rdc_counts regs[RDC_REGISTER_COUNT];
  rdc_load_sample(regs);

  rdc_result r1 = rdc_compute(&regs[0], 400);
  expect_int("r1 amount", r1.amount_cents, 380585);
  expect_int("r1 drop", r1.drop_cents, 340585);
  expect_int("r1 left", r1.left_cents, 40000);
  expect_int("r1 balanced", r1.balanced ? 1 : 0, 1);
  expect_int("r1 drop five", r1.drop.n[RDC_FIVE], 1);
  expect_int("r1 drop twenty", r1.drop.n[RDC_TWENTY], 95);
  expect_int("r1 drop fifty", r1.drop.n[RDC_FIFTY], 6);
  expect_int("r1 drop hundred", r1.drop.n[RDC_HUNDRED], 12);
  expect_int("r1 drop quarter", r1.drop.n[RDC_QUARTER], 3);
  expect_int("r1 drop dime", r1.drop.n[RDC_DIME], 1);
  expect_int("r1 left one", r1.left.n[RDC_ONE], 109);
  expect_int("r1 left five", r1.left.n[RDC_FIVE], 4);
  expect_int("r1 left ten", r1.left.n[RDC_TEN], 10);
  expect_int("r1 left twenty", r1.left.n[RDC_TWENTY], 8);

  char money[32];
  rdc_money(r1.amount_cents, money, sizeof money);
  expect_str("r1 money", money, "$3,805.85");

  rdc_log_row dep = rdc_deposit_row(&r1);
  expect_int("dep ones", dep.ones, 0);
  expect_int("dep twos", dep.twos, 0);
  expect_int("dep fives", dep.fives, 1);
  expect_int("dep twenties", dep.twenties, 95);
  expect_int("dep coin", dep.coin_cents, 85);
  expect_int("dep total", dep.total_cents, 340585);

  rdc_log_row eod = rdc_eod_row(&r1);
  expect_int("eod ones", eod.ones, 109);
  expect_int("eod coin", eod.coin_cents, 1185);
  expect_int("eod total", eod.total_cents, 380585);

  rdc_log_row rst = rdc_reset_row(&r1, 400);
  expect_int("rst ones", rst.ones, 109);
  expect_int("rst fives", rst.fives, 4);
  expect_int("rst twenties", rst.twenties, 8);
  expect_int("rst coin", rst.coin_cents, 1100);
  expect_int("rst total", rst.total_cents, 40000);

  rdc_result r2 = rdc_compute(&regs[1], 400);
  expect_int("r2 amount", r2.amount_cents, 273835);
  expect_int("r2 drop", r2.drop_cents, 233835);
  expect_int("r2 left", r2.left_cents, 40000);
  expect_int("r2 drop one", r2.drop.n[RDC_ONE], 3);
  expect_int("r2 drop twenty", r2.drop.n[RDC_TWENTY], 84);
  expect_int("r2 drop hundred", r2.drop.n[RDC_HUNDRED], 5);

  rdc_counts two = {0};
  two.n[RDC_ONE] = 400;
  two.n[RDC_TWO] = 3;
  two.n[RDC_FIVE] = 1;
  rdc_result rtwo = rdc_compute(&two, 400);
  expect_int("$2 bills drop", rtwo.drop.n[RDC_TWO], 3);
  expect_int("$2 left ones", rtwo.left.n[RDC_ONE], 400);

  rdc_sheet s;
  rdc_sheet_init(&s);
  rdc_load_sample(s.registers);
  expect_int("clear r1", rdc_clear_register(&s, 0) ? 1 : 0, 1);
  expect_int("r1 empty after clear", rdc_counts_any(&s.registers[0]) ? 1 : 0, 0);
  expect_int("undo", rdc_undo_clear(&s) ? 1 : 0, 1);
  expect_int("r1 back", s.registers[0].n[RDC_HUNDRED], 12);

  rdc_set_name(&s, 0, "  Drive-thru  ");
  expect_str("name", s.names[0], "Drive-thru");
  rdc_set_name(&s, 1, "");
  expect_str("name fallback", s.names[1], "R2");

  enum rdc_denom d;
  expect_int("parse twenty", rdc_parse_denom("20", &d) && d == RDC_TWENTY ? 1 : 0, 1);
  expect_int("parse two", rdc_parse_denom("$2", &d) && d == RDC_TWO ? 1 : 0, 1);

  char tsv[256];
  rdc_one_row_tsv(&dep, tsv, sizeof tsv);
  expect_int("tsv has tabs", strchr(tsv, '\t') ? 1 : 0, 1);

  rdc_sheet slip_s;
  rdc_sheet_init(&slip_s);
  slip_s.registers[0].n[RDC_TWENTY] = 25;
  snprintf(slip_s.bag, sizeof slip_s.bag, "SEAL-998877");
  snprintf(slip_s.initials, sizeof slip_s.initials, "RRJR");
  rdc_set_name(&slip_s, 0, "Drive-thru lane");
  char slip[4096];
  expect_int("slip ok", rdc_drop_slip_text(&slip_s, 0, slip, sizeof slip) >= 0 ? 1 : 0, 1);
  expect_int("slip title", strstr(slip, "DROP SLIP") ? 1 : 0, 1);
  expect_int("slip till", strstr(slip, "Till: Drive-thru lane") ? 1 : 0, 1);
  expect_int("slip total", strstr(slip, "DROP TOTAL") ? 1 : 0, 1);
  expect_int("slip left", strstr(slip, "LEFT IN DRAWER") ? 1 : 0, 1);
  {
    int maxw = 0;
    for (char *p = slip; *p;) {
      char *nl = strchr(p, '\n');
      int w = nl ? (int)(nl - p) : (int)strlen(p);
      if (w > maxw) maxw = w;
      p = nl ? nl + 1 : p + strlen(p);
    }
    expect_int("slip width <= 42", maxw <= RDC_RECEIPT_COLS ? 1 : 0, 1);
    expect_int("receipt cols", RDC_RECEIPT_COLS, 42);
  }

  if (g_fail) {
    fputs("rdc_test failed\n", stderr);
    return 1;
  }
  puts("rdc_test: all checks passed (workbook sample, $2 bills, undo, names)");
  return 0;
}
