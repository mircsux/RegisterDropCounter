#ifndef RDC_ENGINE_H
#define RDC_ENGINE_H

#include "rdc_c23.h"

#ifdef __cplusplus
extern "C" {
#endif

enum rdc_denom {
  RDC_PENNY = 0,
  RDC_NICKEL,
  RDC_DIME,
  RDC_QUARTER,
  RDC_PROLL,
  RDC_NROLL,
  RDC_DROLL,
  RDC_QROLL,
  RDC_ONE,
  RDC_TWO,
  RDC_FIVE,
  RDC_TEN,
  RDC_TWENTY,
  RDC_FIFTY,
  RDC_HUNDRED,
  RDC_DENOM_COUNT
};

enum { RDC_REGISTER_COUNT = 10 };
enum { RDC_COUNT_MAX = 99999 };
enum { RDC_RECEIPT_COLS = 42 };
enum { RDC_RECEIPT_PAPER_MM = 80 };
enum { RDC_RECEIPT_PRINT_MM = 72 };

typedef struct rdc_counts {
  int n[RDC_DENOM_COUNT];
} rdc_counts;

typedef struct rdc_result {
  rdc_counts counts;
  rdc_counts drop;
  rdc_counts left;
  int amount_cents;
  int drop_cents;
  int left_cents;
  bool balanced;
  bool has_count;
} rdc_result;

typedef struct rdc_log_row {
  int ones, twos, fives, tens, twenties, fifties, hundreds;
  int coin_cents;
  int total_cents;
} rdc_log_row;

int rdc_cents(enum rdc_denom d);
const char *rdc_label(enum rdc_denom d);
const char *rdc_roll_letter(enum rdc_denom d);
const char *rdc_slip_name(enum rdc_denom d);
const char *rdc_key_name(enum rdc_denom d);
int rdc_clamp_count(int v);
int rdc_parse_denom(const char *s, enum rdc_denom *out);

void rdc_counts_zero(rdc_counts *c);
bool rdc_counts_any(const rdc_counts *c);
bool rdc_sheet_any(const rdc_counts regs[RDC_REGISTER_COUNT]);

RDC_NODISCARD rdc_result rdc_compute(const rdc_counts *in, int base_dollars);
int rdc_loose_coin_cents(const rdc_counts *c);
int rdc_coin_and_roll_cents(const rdc_counts *c);
int rdc_bill_cents(const rdc_counts *c);

void rdc_money(int cents, char *buf, size_t buf_n);
void rdc_plain_money(int cents, char *buf, size_t buf_n);

rdc_log_row rdc_deposit_row(const rdc_result *r);
rdc_log_row rdc_eod_row(const rdc_result *r);
rdc_log_row rdc_reset_row(const rdc_result *r, int base_dollars);
rdc_log_row rdc_sum_rows(const rdc_log_row *rows, int n);

void rdc_one_row_tsv(const rdc_log_row *r, char *buf, size_t buf_n);
void rdc_area_row_tsv(const char *label, const rdc_log_row *r, char *buf, size_t buf_n);
void rdc_area_tsv(const rdc_log_row *rows, int n, char *buf, size_t buf_n);

int rdc_valid_base(int dollars);
void rdc_load_sample(rdc_counts regs[RDC_REGISTER_COUNT]);

#ifdef __cplusplus
}
#endif

#endif
