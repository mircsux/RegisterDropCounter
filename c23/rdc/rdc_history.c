#include "rdc_history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool rdc_make_snapshot(rdc_hist_entry *out, enum rdc_hist_kind kind, int register_index,
                       int base, const rdc_counts regs[RDC_REGISTER_COUNT]) {
  if (!out || !regs) return false;
  if (kind == RDC_HIST_REGISTER) {
    if (register_index < 0 || register_index >= RDC_REGISTER_COUNT) return false;
    if (!rdc_counts_any(&regs[register_index])) return false;
  } else if (!rdc_sheet_any(regs)) {
    return false;
  }
  memset(out, 0, sizeof(*out));
  out->at = time(RDC_NULL);
  out->kind = kind;
  out->register_index = (kind == RDC_HIST_REGISTER) ? register_index : -1;
  out->base = rdc_valid_base(base) ? base : 400;
  memcpy(out->registers, regs, sizeof(out->registers));
  return true;
}

void rdc_history_prepend(rdc_history *hist, const rdc_hist_entry *e) {
  if (!hist || !e) return;
  if (hist->count >= RDC_HISTORY_LIMIT) hist->count = RDC_HISTORY_LIMIT - 1;
  if (hist->count > 0)
    memmove(&hist->items[1], &hist->items[0], (size_t)hist->count * sizeof(hist->items[0]));
  hist->items[0] = *e;
  hist->count += 1;
}

void rdc_apply_undo(rdc_counts live[RDC_REGISTER_COUNT], int *base, const rdc_hist_entry *e) {
  if (!live || !e) return;
  if (e->kind == RDC_HIST_REGISTER && e->register_index >= 0 &&
      e->register_index < RDC_REGISTER_COUNT) {
    live[e->register_index] = e->registers[e->register_index];
    return;
  }
  memcpy(live, e->registers, sizeof(e->registers));
  if (base) *base = e->base;
}

static void local_tm(time_t t, struct tm *out) {
  memset(out, 0, sizeof(*out));
#if defined(_WIN32)
  localtime_s(out, &t);
#else
  {
    struct tm *p = localtime(&t);
    if (p) *out = *p;
  }
#endif
}

void rdc_date_key(time_t t, char *buf, size_t n) {
  if (!buf || n == 0) return;
  struct tm tm;
  local_tm(t, &tm);
  strftime(buf, n, "%Y-%m-%d", &tm);
}

void rdc_format_when(time_t t, char *buf, size_t n) {
  if (!buf || n == 0) return;
  struct tm tm;
  local_tm(t, &tm);
  strftime(buf, n, "%a %b %d, %Y  %I:%M %p", &tm);
}

void rdc_format_sheet_date(time_t t, char *buf, size_t n) {
  if (!buf || n == 0) return;
  struct tm tm;
  local_tm(t, &tm);
  strftime(buf, n, "%A, %B %d, %Y", &tm);
}

void rdc_history_label(const rdc_hist_entry *e, const char names[RDC_REGISTER_COUNT][24],
                       char *buf, size_t n) {
  if (!buf || n == 0) return;
  if (!e) {
    buf[0] = '\0';
    return;
  }
  if (e->kind == RDC_HIST_ALL) {
    snprintf(buf, n, "Cleared all registers");
    return;
  }
  int i = e->register_index;
  if (names && i >= 0 && i < RDC_REGISTER_COUNT && names[i][0])
    snprintf(buf, n, "Cleared %s", names[i]);
  else
    snprintf(buf, n, "Cleared R%d", i + 1);
}

int rdc_history_encode(const rdc_history *hist, char *buf, size_t n) {
  if (!buf || n == 0) return -1;
  int used = snprintf(buf, n, "RDCH1\n%d\n", hist ? hist->count : 0);
  if (used < 0 || (size_t)used >= n) return -1;
  if (!hist) return used;
  for (int e = 0; e < hist->count; ++e) {
    const rdc_hist_entry *it = &hist->items[e];
    int w = snprintf(buf + used, n - (size_t)used, "%ld %d %d %d\n", (long)it->at,
                     (int)it->kind, it->register_index, it->base);
    if (w < 0 || (size_t)w >= n - (size_t)used) return -1;
    used += w;
    for (int r = 0; r < RDC_REGISTER_COUNT; ++r) {
      char line[128];
      int p = 0;
      line[0] = '\0';
      for (int d = 0; d < RDC_DENOM_COUNT; ++d) {
        int k = snprintf(line + p, sizeof line - (size_t)p, "%s%d", d ? "," : "",
                         it->registers[r].n[d]);
        if (k < 0) return -1;
        p += k;
      }
      w = snprintf(buf + used, n - (size_t)used, "%s\n", line);
      if (w < 0 || (size_t)w >= n - (size_t)used) return -1;
      used += w;
    }
  }
  return used;
}

static int parse_int(const char *s) { return (int)strtol(s, RDC_NULL, 10); }

int rdc_history_decode(const char *text, rdc_history *hist) {
  if (!hist) return -1;
  memset(hist, 0, sizeof(*hist));
  if (!text) return -1;
  char *copy = malloc(strlen(text) + 1);
  if (!copy) return -1;
  memcpy(copy, text, strlen(text) + 1);
  char *save = RDC_NULL;
  char *line = strtok(copy, "\n");
  if (!line || strcmp(line, "RDCH1") != 0) {
    free(copy);
    return -1;
  }
  line = strtok(RDC_NULL, "\n");
  int n = line ? parse_int(line) : 0;
  if (n < 0) n = 0;
  if (n > RDC_HISTORY_LIMIT) n = RDC_HISTORY_LIMIT;
  for (int e = 0; e < n; ++e) {
    line = strtok(RDC_NULL, "\n");
    if (!line) break;
    rdc_hist_entry it;
    memset(&it, 0, sizeof(it));
    long at = 0;
    int kind = 1, reg = -1, base = 400;
    sscanf(line, "%ld %d %d %d", &at, &kind, &reg, &base);
    it.at = (time_t)at;
    it.kind = kind == 0 ? RDC_HIST_REGISTER : RDC_HIST_ALL;
    it.register_index = it.kind == RDC_HIST_REGISTER ? reg : -1;
    it.base = rdc_valid_base(base) ? base : 400;
    for (int r = 0; r < RDC_REGISTER_COUNT; ++r) {
      line = strtok(RDC_NULL, "\n");
      if (!line) break;
      rdc_counts_zero(&it.registers[r]);
      char tmp[256];
      snprintf(tmp, sizeof tmp, "%s", line);
      char *p = tmp;
      int vals[16];
      int nv = 0;
      while (nv < 16 && p && *p) {
        char *comma = strchr(p, ',');
        if (comma) *comma = '\0';
        vals[nv++] = rdc_clamp_count(parse_int(p));
        p = comma ? comma + 1 : RDC_NULL;
      }
      if (nv == 14) {
        for (int i = 14; i > 9; --i) vals[i] = vals[i - 1];
        vals[9] = 0;
        nv = 15;
      }
      for (int d = 0; d < RDC_DENOM_COUNT && d < nv; ++d) it.registers[r].n[d] = vals[d];
    }
    if (hist->count < RDC_HISTORY_LIMIT) hist->items[hist->count++] = it;
    (void)save;
  }
  free(copy);
  return hist->count;
}
