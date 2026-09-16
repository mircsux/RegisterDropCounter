#include "rdc_store.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void rdc_default_names(char names[RDC_REGISTER_COUNT][24]) {
  if (!names) return;
  for (int i = 0; i < RDC_REGISTER_COUNT; ++i) snprintf(names[i], 24, "R%d", i + 1);
}

void rdc_sheet_init(rdc_sheet *s) {
  if (!s) return;
  memset(s, 0, sizeof(*s));
  s->base = 400;
  rdc_default_names(s->names);
}

void rdc_set_name(rdc_sheet *s, int index, const char *raw) {
  if (!s || index < 0 || index >= RDC_REGISTER_COUNT) return;
  char tmp[24];
  size_t o = 0;
  bool space = false;
  if (!raw) raw = "";
  for (const char *p = raw; *p && o + 1 < sizeof tmp && o < (size_t)RDC_NAME_MAX; ++p) {
    unsigned char c = (unsigned char)*p;
    if (isspace(c)) {
      if (o == 0 || space) continue;
      tmp[o++] = ' ';
      space = true;
    } else {
      tmp[o++] = (char)c;
      space = false;
    }
  }
  while (o > 0 && tmp[o - 1] == ' ') o--;
  tmp[o] = '\0';
  if (!tmp[0])
    snprintf(s->names[index], 24, "R%d", index + 1);
  else
    snprintf(s->names[index], 24, "%s", tmp);
}

bool rdc_clear_register(rdc_sheet *s, int index) {
  if (!s) return false;
  rdc_hist_entry e;
  if (!rdc_make_snapshot(&e, RDC_HIST_REGISTER, index, s->base, s->registers)) return false;
  rdc_history_prepend(&s->history, &e);
  rdc_counts_zero(&s->registers[index]);
  return true;
}

bool rdc_clear_all(rdc_sheet *s) {
  if (!s) return false;
  rdc_hist_entry e;
  if (!rdc_make_snapshot(&e, RDC_HIST_ALL, -1, s->base, s->registers)) return false;
  rdc_history_prepend(&s->history, &e);
  for (int i = 0; i < RDC_REGISTER_COUNT; ++i) rdc_counts_zero(&s->registers[i]);
  return true;
}

bool rdc_undo_clear(rdc_sheet *s) {
  if (!s || s->history.count <= 0) return false;
  rdc_apply_undo(s->registers, &s->base, &s->history.items[0]);
  if (s->history.items[0].kind == RDC_HIST_REGISTER && s->history.items[0].register_index >= 0)
    s->active = s->history.items[0].register_index;
  return true;
}

static int write_text(const char *path, const char *text) {
  if (!path || !text) return -1;
  FILE *f = fopen(path, "wb");
  if (!f) return -1;
  size_t n = strlen(text);
  size_t w = fwrite(text, 1, n, f);
  fclose(f);
  return w == n ? 0 : -1;
}

static char *read_text(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) return RDC_NULL;
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return RDC_NULL;
  }
  long sz = ftell(f);
  if (sz < 0 || sz > 8'000'000) {
    fclose(f);
    return RDC_NULL;
  }
  rewind(f);
  char *buf = malloc((size_t)sz + 1);
  if (!buf) {
    fclose(f);
    return RDC_NULL;
  }
  size_t n = fread(buf, 1, (size_t)sz, f);
  fclose(f);
  buf[n] = '\0';
  return buf;
}

int rdc_save_state_path(const rdc_sheet *s, const char *path) {
  if (!s || !path) return -1;
  char buf[8192];
  int used = snprintf(buf, sizeof buf, "RDC1\n%d\n", s->base);
  if (used < 0) return -1;
  for (int i = 0; i < RDC_REGISTER_COUNT; ++i) {
    for (int d = 0; d < RDC_DENOM_COUNT; ++d) {
      int w = snprintf(buf + used, sizeof buf - (size_t)used, "%s%d", d ? "," : "",
                       s->registers[i].n[d]);
      if (w < 0 || (size_t)w >= sizeof buf - (size_t)used) return -1;
      used += w;
    }
    int w = snprintf(buf + used, sizeof buf - (size_t)used, "\n");
    if (w < 0) return -1;
    used += w;
  }
  return write_text(path, buf);
}

int rdc_load_state_path(rdc_sheet *s, const char *path) {
  char *text = read_text(path);
  if (!text || !s) {
    free(text);
    return -1;
  }
  char *p = text;
  if (strncmp(p, "RDC1", 4) != 0) {
    free(text);
    return -1;
  }
  p = strchr(p, '\n');
  if (!p) {
    free(text);
    return -1;
  }
  s->base = (int)strtol(p + 1, &p, 10);
  if (!rdc_valid_base(s->base)) s->base = 400;
  if (*p == '\r') p++;
  if (*p == '\n') p++;
  for (int i = 0; i < RDC_REGISTER_COUNT && p && *p; ++i) {
    char line[256];
    int li = 0;
    while (*p && *p != '\n' && li + 1 < (int)sizeof line) line[li++] = *p++;
    line[li] = '\0';
    if (*p == '\n') p++;
    int vals[16], nv = 0;
    char *tok = line;
    while (nv < 16 && tok && *tok) {
      char *comma = strchr(tok, ',');
      if (comma) *comma = '\0';
      vals[nv++] = rdc_clamp_count((int)strtol(tok, RDC_NULL, 10));
      tok = comma ? comma + 1 : RDC_NULL;
    }
    if (nv == 14) {
      for (int k = 14; k > 9; --k) vals[k] = vals[k - 1];
      vals[9] = 0;
      nv = 15;
    }
    rdc_counts_zero(&s->registers[i]);
    for (int d = 0; d < RDC_DENOM_COUNT && d < nv; ++d) s->registers[i].n[d] = vals[d];
  }
  free(text);
  return 0;
}

int rdc_save_names_path(const rdc_sheet *s, const char *path) {
  if (!s || !path) return -1;
  char buf[512];
  int used = snprintf(buf, sizeof buf, "RDCN1\n");
  for (int i = 0; i < RDC_REGISTER_COUNT; ++i) {
    int w = snprintf(buf + used, sizeof buf - (size_t)used, "%s\n", s->names[i]);
    if (w < 0) return -1;
    used += w;
  }
  return write_text(path, buf);
}

int rdc_load_names_path(rdc_sheet *s, const char *path) {
  char *text = read_text(path);
  if (!text || !s) {
    free(text);
    return -1;
  }
  if (strncmp(text, "RDCN1", 5) != 0) {
    free(text);
    return -1;
  }
  char *p = strchr(text, '\n');
  if (p) p++;
  for (int i = 0; i < RDC_REGISTER_COUNT && p; ++i) {
    char line[64];
    int li = 0;
    while (*p && *p != '\n' && li + 1 < (int)sizeof line) {
      if (*p != '\r') line[li++] = *p;
      p++;
    }
    line[li] = '\0';
    if (*p == '\n') p++;
    rdc_set_name(s, i, line);
  }
  free(text);
  return 0;
}

int rdc_save_history_path(const rdc_sheet *s, const char *path) {
  if (!s || !path) return -1;
  char *buf = malloc(256 * 1024);
  if (!buf) return -1;
  int n = rdc_history_encode(&s->history, buf, 256 * 1024);
  int rc = n < 0 ? -1 : write_text(path, buf);
  free(buf);
  return rc;
}

int rdc_load_history_path(rdc_sheet *s, const char *path) {
  char *text = read_text(path);
  if (!text || !s) {
    free(text);
    return -1;
  }
  int n = rdc_history_decode(text, &s->history);
  free(text);
  return n < 0 ? -1 : 0;
}

int rdc_save_slip_path(const rdc_sheet *s, const char *path) {
  if (!s || !path) return -1;
  char buf[256];
  snprintf(buf, sizeof buf, "RDCS1\n%s\n%s\n", s->bag, s->initials);
  return write_text(path, buf);
}

int rdc_load_slip_path(rdc_sheet *s, const char *path) {
  char *text = read_text(path);
  if (!text || !s) {
    free(text);
    return -1;
  }
  if (strncmp(text, "RDCS1", 5) != 0) {
    free(text);
    return -1;
  }
  char *p = strchr(text, '\n');
  if (p) p++;
  char *nl = p ? strchr(p, '\n') : RDC_NULL;
  if (p && nl) {
    size_t n = (size_t)(nl - p);
    if (n >= sizeof s->bag) n = sizeof s->bag - 1;
    memcpy(s->bag, p, n);
    s->bag[n] = '\0';
    if (n && s->bag[n - 1] == '\r') s->bag[n - 1] = '\0';
    p = nl + 1;
    nl = strchr(p, '\n');
    n = nl ? (size_t)(nl - p) : strlen(p);
    if (n >= sizeof s->initials) n = sizeof s->initials - 1;
    memcpy(s->initials, p, n);
    s->initials[n] = '\0';
    if (n && s->initials[n - 1] == '\r') s->initials[n - 1] = '\0';
  }
  free(text);
  return 0;
}

int rdc_drop_slip_text(const rdc_sheet *s, int index, char *buf, size_t n) {
  if (!s || !buf || n == 0 || index < 0 || index >= RDC_REGISTER_COUNT) return -1;
  rdc_result r = rdc_compute(&s->registers[index], s->base);
  char date[80], amount[32], drop[32], left[32], base[32];
  rdc_format_sheet_date(time(RDC_NULL), date, sizeof date);
  rdc_money(r.amount_cents, amount, sizeof amount);
  rdc_money(r.drop_cents, drop, sizeof drop);
  rdc_money(r.left_cents, left, sizeof left);
  rdc_money(s->base * 100, base, sizeof base);
  int used = snprintf(buf, n,
                      "REGISTER DROP SLIP\nDate: %s\nRegister: %s\nRegister base: %s\n"
                      "Bag / seal #: %s\nInitials: %s\n\nDenom                  Qty      Amount\n",
                      date, s->names[index], base, s->bag[0] ? s->bag : "________",
                      s->initials[0] ? s->initials : "________");
  if (used < 0 || (size_t)used >= n) return -1;
  for (int d = 0; d < RDC_DENOM_COUNT; ++d) {
    if (r.drop.n[d] <= 0) continue;
    char amt[32];
    rdc_money(r.drop.n[d] * rdc_cents((enum rdc_denom)d), amt, sizeof amt);
    int w = snprintf(buf + used, n - (size_t)used, "%-22s %4d  %10s\n", rdc_slip_name((enum rdc_denom)d),
                     r.drop.n[d], amt);
    if (w < 0 || (size_t)w >= n - (size_t)used) return -1;
    used += w;
  }
  const char *bal = !r.has_count ? "Empty" : r.balanced ? "Yes - left equals base" : "No - off base";
  int w = snprintf(buf + used, n - (size_t)used,
                   "\nDrop total:  %s\nLeft in drawer:  %s\nBalanced:  %s\nDrawer counted:  %s\n",
                   drop, left, bal, amount);
  if (w < 0) return -1;
  return used + w;
}
