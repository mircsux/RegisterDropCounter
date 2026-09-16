#include "rdc_engine.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int k_cents[RDC_DENOM_COUNT] = {
    1, 5, 10, 25, 50, 200, 500, 1'000, 100, 200, 500, 1'000, 2'000, 5'000, 10'000};

static const char *k_labels[RDC_DENOM_COUNT] = {
    "0.01", "0.05", "0.10", "0.25", "0.50", "2.00", "5.00", "10.00",
    "1",    "2",    "5",    "10",   "20",   "50",   "100"};

static const char *k_roll[RDC_DENOM_COUNT] = {
    "", "", "", "", "P", "N", "D", "Q", "", "", "", "", "", "", ""};

static const char *k_slip[RDC_DENOM_COUNT] = {
    "Pennies",       "Nickels",       "Dimes",         "Quarters",
    "P roll (0.50)", "N roll (2.00)", "D roll (5.00)", "Q roll (10.00)",
    "$1",            "$2",            "$5",            "$10",
    "$20",           "$50",           "$100"};

static const char *k_keys[RDC_DENOM_COUNT] = {
    "penny", "nickel", "dime", "quarter", "proll", "nroll", "droll", "qroll",
    "one",   "two",    "five", "ten",     "twenty", "fifty", "hundred"};

static const int k_drop_order[RDC_DENOM_COUNT] = {
    RDC_HUNDRED, RDC_FIFTY, RDC_TWENTY, RDC_TEN,    RDC_FIVE,  RDC_TWO,   RDC_ONE,
    RDC_QUARTER, RDC_DIME,  RDC_NICKEL, RDC_QROLL,  RDC_DROLL, RDC_NROLL, RDC_PROLL,
    RDC_PENNY};

int rdc_cents(enum rdc_denom d) {
  if ((int)d < 0 || d >= RDC_DENOM_COUNT) return 0;
  return k_cents[d];
}

const char *rdc_label(enum rdc_denom d) {
  if ((int)d < 0 || d >= RDC_DENOM_COUNT) return "";
  return k_labels[d];
}

const char *rdc_roll_letter(enum rdc_denom d) {
  if ((int)d < 0 || d >= RDC_DENOM_COUNT) return "";
  return k_roll[d];
}

const char *rdc_slip_name(enum rdc_denom d) {
  if ((int)d < 0 || d >= RDC_DENOM_COUNT) return "";
  return k_slip[d];
}

const char *rdc_key_name(enum rdc_denom d) {
  if ((int)d < 0 || d >= RDC_DENOM_COUNT) return "";
  return k_keys[d];
}

int rdc_clamp_count(int v) {
  if (v < 0) return 0;
  if (v > RDC_COUNT_MAX) return RDC_COUNT_MAX;
  return v;
}

static int icmp(const char *a, const char *b) {
  while (*a && *b) {
    unsigned char ca = (unsigned char)tolower((unsigned char)*a++);
    unsigned char cb = (unsigned char)tolower((unsigned char)*b++);
    if (ca != cb) return (int)ca - (int)cb;
  }
  return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int rdc_parse_denom(const char *s, enum rdc_denom *out) {
  if (!s || !out) return 0;
  while (*s == '$' || *s == ' ') s++;
  for (int i = 0; i < RDC_DENOM_COUNT; ++i) {
    if (icmp(s, k_keys[i]) == 0 || icmp(s, k_labels[i]) == 0) {
      *out = (enum rdc_denom)i;
      return 1;
    }
  }
  if (icmp(s, "p") == 0 || icmp(s, "p-roll") == 0) {
    *out = RDC_PROLL;
    return 1;
  }
  if (icmp(s, "n") == 0 || icmp(s, "n-roll") == 0) {
    *out = RDC_NROLL;
    return 1;
  }
  if (icmp(s, "d") == 0 || icmp(s, "d-roll") == 0) {
    *out = RDC_DROLL;
    return 1;
  }
  if (icmp(s, "q") == 0 || icmp(s, "q-roll") == 0) {
    *out = RDC_QROLL;
    return 1;
  }
  if (icmp(s, "pennies") == 0) {
    *out = RDC_PENNY;
    return 1;
  }
  if (icmp(s, "nickels") == 0) {
    *out = RDC_NICKEL;
    return 1;
  }
  if (icmp(s, "dimes") == 0) {
    *out = RDC_DIME;
    return 1;
  }
  if (icmp(s, "quarters") == 0) {
    *out = RDC_QUARTER;
    return 1;
  }
  return 0;
}

void rdc_counts_zero(rdc_counts *c) {
  if (!c) return;
  memset(c, 0, sizeof(*c));
}

bool rdc_counts_any(const rdc_counts *c) {
  if (!c) return false;
  for (int i = 0; i < RDC_DENOM_COUNT; ++i)
    if (c->n[i] > 0) return true;
  return false;
}

bool rdc_sheet_any(const rdc_counts regs[RDC_REGISTER_COUNT]) {
  if (!regs) return false;
  for (int i = 0; i < RDC_REGISTER_COUNT; ++i)
    if (rdc_counts_any(&regs[i])) return true;
  return false;
}

rdc_result rdc_compute(const rdc_counts *in, int base_dollars) {
  rdc_result r;
  memset(&r, 0, sizeof(r));
  if (in) {
    for (int i = 0; i < RDC_DENOM_COUNT; ++i) r.counts.n[i] = rdc_clamp_count(in->n[i]);
  }
  if (!rdc_valid_base(base_dollars)) base_dollars = 400;
  const int base_cents = base_dollars * 100;
  for (int i = 0; i < RDC_DENOM_COUNT; ++i) {
    r.amount_cents += r.counts.n[i] * k_cents[i];
    if (r.counts.n[i] > 0) r.has_count = true;
  }
  int remaining = r.amount_cents - base_cents;
  if (remaining < 0) remaining = 0;
  for (int o = 0; o < RDC_DENOM_COUNT; ++o) {
    const int key = k_drop_order[o];
    const int cents = k_cents[key];
    const int available = r.counts.n[key];
    int need = remaining / cents;
    int d = available < need ? available : need;
    if (d < 0) d = 0;
    r.drop.n[key] = d;
    r.left.n[key] = available - d;
    remaining -= d * cents;
  }
  for (int i = 0; i < RDC_DENOM_COUNT; ++i) {
    r.drop_cents += r.drop.n[i] * k_cents[i];
    r.left_cents += r.left.n[i] * k_cents[i];
  }
  r.balanced = (r.left_cents == base_cents);
  return r;
}

int rdc_loose_coin_cents(const rdc_counts *c) {
  if (!c) return 0;
  return c->n[RDC_PENNY] * k_cents[RDC_PENNY] + c->n[RDC_NICKEL] * k_cents[RDC_NICKEL] +
         c->n[RDC_DIME] * k_cents[RDC_DIME] + c->n[RDC_QUARTER] * k_cents[RDC_QUARTER];
}

int rdc_coin_and_roll_cents(const rdc_counts *c) {
  if (!c) return 0;
  return rdc_loose_coin_cents(c) + c->n[RDC_PROLL] * k_cents[RDC_PROLL] +
         c->n[RDC_NROLL] * k_cents[RDC_NROLL] + c->n[RDC_DROLL] * k_cents[RDC_DROLL] +
         c->n[RDC_QROLL] * k_cents[RDC_QROLL];
}

int rdc_bill_cents(const rdc_counts *c) {
  if (!c) return 0;
  return c->n[RDC_ONE] * 100 + c->n[RDC_TWO] * 200 + c->n[RDC_FIVE] * 500 +
         c->n[RDC_TEN] * 1'000 + c->n[RDC_TWENTY] * 2'000 + c->n[RDC_FIFTY] * 5'000 +
         c->n[RDC_HUNDRED] * 10'000;
}

static void write_grouped(char *dst, size_t n, unsigned v) {
  char tmp[32];
  int len = snprintf(tmp, sizeof tmp, "%u", v);
  if (len < 0) {
    dst[0] = '\0';
    return;
  }
  int groups = (len - 1) / 3;
  int out_len = len + groups;
  if ((size_t)out_len + 1 > n) {
    dst[0] = '\0';
    return;
  }
  int di = 0;
  int lead = len % 3;
  if (lead == 0) lead = 3;
  for (int i = 0; i < len; ++i) {
    if (i == lead && i < len) {
      dst[di++] = ',';
      lead += 3;
    }
    dst[di++] = tmp[i];
  }
  dst[di] = '\0';
}

void rdc_money(int cents, char *buf, size_t buf_n) {
  if (!buf || buf_n == 0) return;
  const int neg = cents < 0;
  unsigned a = (unsigned)(neg ? -cents : cents);
  char num[32];
  write_grouped(num, sizeof num, a / 100);
  snprintf(buf, buf_n, "%s$%s.%02u", neg ? "-" : "", num, a % 100);
}

void rdc_plain_money(int cents, char *buf, size_t buf_n) {
  if (!buf || buf_n == 0) return;
  const int neg = cents < 0;
  unsigned a = (unsigned)(neg ? -cents : cents);
  snprintf(buf, buf_n, "%s%u.%02u", neg ? "-" : "", a / 100, a % 100);
}

rdc_log_row rdc_deposit_row(const rdc_result *r) {
  rdc_log_row o;
  memset(&o, 0, sizeof(o));
  if (!r) return o;
  o.ones = r->drop.n[RDC_ONE];
  o.twos = r->drop.n[RDC_TWO];
  o.fives = r->drop.n[RDC_FIVE];
  o.tens = r->drop.n[RDC_TEN];
  o.twenties = r->drop.n[RDC_TWENTY];
  o.fifties = r->drop.n[RDC_FIFTY];
  o.hundreds = r->drop.n[RDC_HUNDRED];
  o.coin_cents = rdc_loose_coin_cents(&r->drop);
  o.total_cents = rdc_bill_cents(&r->drop) + o.coin_cents;
  return o;
}

rdc_log_row rdc_eod_row(const rdc_result *r) {
  rdc_log_row o;
  memset(&o, 0, sizeof(o));
  if (!r) return o;
  o.ones = r->counts.n[RDC_ONE];
  o.twos = r->counts.n[RDC_TWO];
  o.fives = r->counts.n[RDC_FIVE];
  o.tens = r->counts.n[RDC_TEN];
  o.twenties = r->counts.n[RDC_TWENTY];
  o.fifties = r->counts.n[RDC_FIFTY];
  o.hundreds = r->counts.n[RDC_HUNDRED];
  o.coin_cents = rdc_coin_and_roll_cents(&r->counts);
  o.total_cents = rdc_bill_cents(&r->counts) + o.coin_cents;
  return o;
}

rdc_log_row rdc_reset_row(const rdc_result *r, int base_dollars) {
  rdc_log_row o;
  memset(&o, 0, sizeof(o));
  if (!r) return o;
  o.ones = r->left.n[RDC_ONE];
  o.twos = r->left.n[RDC_TWO];
  o.fives = r->left.n[RDC_FIVE];
  o.tens = r->left.n[RDC_TEN];
  o.twenties = r->left.n[RDC_TWENTY];
  o.fifties = r->left.n[RDC_FIFTY];
  o.hundreds = r->left.n[RDC_HUNDRED];
  o.coin_cents = r->has_count ? base_dollars * 100 - rdc_bill_cents(&r->left) : 0;
  o.total_cents = r->left_cents;
  return o;
}

rdc_log_row rdc_sum_rows(const rdc_log_row *rows, int n) {
  rdc_log_row t;
  memset(&t, 0, sizeof(t));
  if (!rows || n <= 0) return t;
  for (int i = 0; i < n; ++i) {
    t.ones += rows[i].ones;
    t.twos += rows[i].twos;
    t.fives += rows[i].fives;
    t.tens += rows[i].tens;
    t.twenties += rows[i].twenties;
    t.fifties += rows[i].fifties;
    t.hundreds += rows[i].hundreds;
    t.coin_cents += rows[i].coin_cents;
    t.total_cents += rows[i].total_cents;
  }
  return t;
}

void rdc_one_row_tsv(const rdc_log_row *r, char *buf, size_t buf_n) {
  if (!buf || buf_n == 0) return;
  if (!r) {
    buf[0] = '\0';
    return;
  }
  char coin[32];
  rdc_plain_money(r->coin_cents, coin, sizeof coin);
  snprintf(buf, buf_n, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s", r->ones, r->twos, r->fives, r->tens,
           r->twenties, r->fifties, r->hundreds, coin);
}

void rdc_area_row_tsv(const char *label, const rdc_log_row *r, char *buf, size_t buf_n) {
  if (!buf || buf_n == 0) return;
  char row[128];
  char tot[32];
  if (!r) {
    buf[0] = '\0';
    return;
  }
  rdc_plain_money(r->total_cents, tot, sizeof tot);
  rdc_one_row_tsv(r, row, sizeof row);
  snprintf(buf, buf_n, "%s\t%s\t%s", label ? label : "", row, tot);
}

void rdc_area_tsv(const rdc_log_row *rows, int n, char *buf, size_t buf_n) {
  if (!buf || buf_n == 0) return;
  buf[0] = '\0';
  if (!rows || n <= 0) return;
  size_t used = 0;
  for (int i = 0; i < n; ++i) {
    char lab[16];
    char line[256];
    snprintf(lab, sizeof lab, "R%d", i + 1);
    rdc_area_row_tsv(lab, &rows[i], line, sizeof line);
    int w = snprintf(buf + used, buf_n - used, "%s%s", used ? "\r\n" : "", line);
    if (w < 0 || (size_t)w >= buf_n - used) return;
    used += (size_t)w;
  }
  {
    rdc_log_row sum = rdc_sum_rows(rows, n);
    char line[256];
    rdc_area_row_tsv("Totals", &sum, line, sizeof line);
    snprintf(buf + used, buf_n - used, "\r\n%s", line);
  }
}

int rdc_valid_base(int dollars) {
  return dollars == 100 || dollars == 200 || dollars == 300 || dollars == 400 || dollars == 500;
}

void rdc_load_sample(rdc_counts regs[RDC_REGISTER_COUNT]) {
  if (!regs) return;
  for (int i = 0; i < RDC_REGISTER_COUNT; ++i) rdc_counts_zero(&regs[i]);
  regs[0].n[RDC_NICKEL] = 6;
  regs[0].n[RDC_DIME] = 48;
  regs[0].n[RDC_QUARTER] = 27;
  regs[0].n[RDC_ONE] = 109;
  regs[0].n[RDC_FIVE] = 5;
  regs[0].n[RDC_TEN] = 10;
  regs[0].n[RDC_TWENTY] = 103;
  regs[0].n[RDC_FIFTY] = 6;
  regs[0].n[RDC_HUNDRED] = 12;

  regs[1].n[RDC_NICKEL] = 51;
  regs[1].n[RDC_DIME] = 18;
  regs[1].n[RDC_QUARTER] = 16;
  regs[1].n[RDC_ONE] = 100;
  regs[1].n[RDC_FIVE] = 2;
  regs[1].n[RDC_TEN] = 1;
  regs[1].n[RDC_TWENTY] = 98;
  regs[1].n[RDC_FIFTY] = 3;
  regs[1].n[RDC_HUNDRED] = 5;

  regs[2].n[RDC_PENNY] = 5;
  regs[2].n[RDC_NICKEL] = 63;
  regs[2].n[RDC_DIME] = 40;
  regs[2].n[RDC_ONE] = 89;
  regs[2].n[RDC_FIVE] = 19;
  regs[2].n[RDC_TEN] = 25;
  regs[2].n[RDC_TWENTY] = 154;
  regs[2].n[RDC_FIFTY] = 2;
  regs[2].n[RDC_HUNDRED] = 13;
}
