#ifndef RDC_HISTORY_H
#define RDC_HISTORY_H

#include "rdc_engine.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { RDC_HISTORY_LIMIT = 200 };

enum rdc_hist_kind { RDC_HIST_REGISTER = 0, RDC_HIST_ALL = 1 };

typedef struct rdc_hist_entry {
  time_t at;
  enum rdc_hist_kind kind;
  int register_index; /* 0-based, or -1 for all */
  int base;
  rdc_counts registers[RDC_REGISTER_COUNT];
} rdc_hist_entry;

typedef struct rdc_history {
  rdc_hist_entry items[RDC_HISTORY_LIMIT];
  int count;
} rdc_history;

bool rdc_make_snapshot(rdc_hist_entry *out, enum rdc_hist_kind kind, int register_index,
                       int base, const rdc_counts regs[RDC_REGISTER_COUNT]);
void rdc_history_prepend(rdc_history *hist, const rdc_hist_entry *e);
void rdc_apply_undo(rdc_counts live[RDC_REGISTER_COUNT], int *base, const rdc_hist_entry *e);
void rdc_date_key(time_t t, char *buf, size_t n);
void rdc_format_when(time_t t, char *buf, size_t n);
void rdc_format_sheet_date(time_t t, char *buf, size_t n);
void rdc_history_label(const rdc_hist_entry *e, const char names[RDC_REGISTER_COUNT][24],
                       char *buf, size_t n);

int rdc_history_encode(const rdc_history *hist, char *buf, size_t n);
int rdc_history_decode(const char *text, rdc_history *hist);

#ifdef __cplusplus
}
#endif

#endif
