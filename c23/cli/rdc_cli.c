#include "rdc_store.h"
#include "rdc_version.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *k_state = "RegisterDropCounter.state";
static const char *k_names = "RegisterDropCounter.names";
static const char *k_hist = "RegisterDropCounter.history";
static const char *k_slip = "RegisterDropCounter.slip";

static void persist(const rdc_sheet *s) {
  rdc_save_state_path(s, k_state);
  rdc_save_names_path(s, k_names);
  rdc_save_history_path(s, k_hist);
  rdc_save_slip_path(s, k_slip);
}

static void restore(rdc_sheet *s) {
  rdc_load_state_path(s, k_state);
  rdc_load_names_path(s, k_names);
  rdc_load_history_path(s, k_hist);
  rdc_load_slip_path(s, k_slip);
}

static void print_sheet(const rdc_sheet *s) {
  char date[80], base[32];
  rdc_format_sheet_date(time(RDC_NULL), date, sizeof date);
  rdc_money(s->base * 100, base, sizeof base);
  printf("Register Drop Counter  %s\n%s    Register base %s\n\n", RDC_VERSION_STAMP, date, base);
  for (int i = 0; i < RDC_REGISTER_COUNT; ++i) {
    rdc_result r = rdc_compute(&s->registers[i], s->base);
    char amount[32], drop[32], left[32];
    rdc_money(r.amount_cents, amount, sizeof amount);
    rdc_money(r.drop_cents, drop, sizeof drop);
    rdc_money(r.left_cents, left, sizeof left);
    const char *st = !r.has_count ? "Empty" : r.balanced ? "Balanced" : "Off base";
    printf("  %-12s  %10s  drop %10s  left %10s  %s%s\n", s->names[i], amount, drop, left, st,
           i == s->active ? "  *" : "");
  }
}

static void help(void) {
  puts("Register Drop Counter — ISO C23\n");
  puts("  show [R]          print one till (default: current)");
  puts("  sheet             ten-till summary");
  puts("  r N               switch till (1-10 or a name)");
  puts("  N COUNT           set denom on the current till  (20 103)");
  puts("  set R DENOM N     set a count");
  puts("  name [R] TEXT     rename a till (defaults R1-R10)");
  puts("  base DOLLARS      100, 200, 300, 400, or 500");
  puts("  clear [R|all]");
  puts("  undo");
  puts("  slip [R]          printable drop slip");
  puts("  bag TEXT / initials TEXT");
  puts("  tsv deposit|eod|reset");
  puts("  history [YYYY-MM-DD]");
  puts("  sample            load the 2026-08-10 workbook drawers");
  puts("  help / quit");
}

static int parse_reg(const rdc_sheet *s, const char *tok, int *out) {
  if (!tok || !out) return 0;
  if ((tok[0] == 'r' || tok[0] == 'R') && isdigit((unsigned char)tok[1])) {
    int n = (int)strtol(tok + 1, RDC_NULL, 10);
    if (n >= 1 && n <= RDC_REGISTER_COUNT) {
      *out = n - 1;
      return 1;
    }
  }
  if (isdigit((unsigned char)tok[0])) {
    int n = (int)strtol(tok, RDC_NULL, 10);
    if (n >= 1 && n <= RDC_REGISTER_COUNT) {
      *out = n - 1;
      return 1;
    }
  }
  for (int i = 0; i < RDC_REGISTER_COUNT; ++i) {
    if (strcmp(s->names[i], tok) == 0) {
      *out = i;
      return 1;
    }
  }
  return 0;
}

static void print_tsv_area(const rdc_sheet *s, const char *kind) {
  rdc_log_row rows[RDC_REGISTER_COUNT];
  for (int i = 0; i < RDC_REGISTER_COUNT; ++i) {
    rdc_result r = rdc_compute(&s->registers[i], s->base);
    if (strcmp(kind, "eod") == 0)
      rows[i] = rdc_eod_row(&r);
    else if (strcmp(kind, "reset") == 0)
      rows[i] = rdc_reset_row(&r, s->base);
    else
      rows[i] = rdc_deposit_row(&r);
  }
  char buf[4096];
  rdc_area_tsv(rows, RDC_REGISTER_COUNT, buf, sizeof buf);
  puts(buf);
}

static void print_history(const rdc_sheet *s, const char *date) {
  if (s->history.count == 0) {
    puts("No snapshots. Clear a register and it will show up here.");
    return;
  }
  for (int i = 0; i < s->history.count; ++i) {
    char when[80], key[16], lab[48];
    rdc_format_when(s->history.items[i].at, when, sizeof when);
    rdc_date_key(s->history.items[i].at, key, sizeof key);
    if (date && date[0] && strcmp(key, date) != 0) continue;
    rdc_history_label(&s->history.items[i], s->names, lab, sizeof lab);
    printf("  %s  %s\n", when, lab);
  }
}

static void print_one_register(const rdc_sheet *s, int index) {
  rdc_result r = rdc_compute(&s->registers[index], s->base);
  char amount[32], drop[32], left[32];
  rdc_money(r.amount_cents, amount, sizeof amount);
  rdc_money(r.drop_cents, drop, sizeof drop);
  rdc_money(r.left_cents, left, sizeof left);
  const char *st = !r.has_count ? "Empty" : r.balanced ? "Balanced" : "Off base";
  printf("\n  %s   Amount %s   Drop %s   Left %s   %s\n", s->names[index], amount, drop, left, st);
  printf("  %-4s %-8s %-8s %-10s %-8s %-10s\n", "", "Count", "Denom", "Amount", "Drop", "Left");
  for (int d = 0; d < RDC_DENOM_COUNT; ++d) {
    char qbuf[16] = "", dbuf[16] = "", ac[32] = "", lc[32] = "";
    if (r.counts.n[d]) {
      snprintf(qbuf, sizeof qbuf, "%d", r.counts.n[d]);
      rdc_money(r.counts.n[d] * rdc_cents((enum rdc_denom)d), ac, sizeof ac);
    }
    if (r.drop.n[d]) snprintf(dbuf, sizeof dbuf, "%d", r.drop.n[d]);
    if (r.left.n[d]) rdc_money(r.left.n[d] * rdc_cents((enum rdc_denom)d), lc, sizeof lc);
    printf("  %-4s %-8s %-8s %-10s %-8s %-10s\n", rdc_roll_letter((enum rdc_denom)d), qbuf,
           rdc_label((enum rdc_denom)d), ac, dbuf, lc);
  }
}

static int run_line(rdc_sheet *s, char *line) {
  while (*line && isspace((unsigned char)*line)) line++;
  if (!*line) return 0;
  if (line[0] == '#') return 0;
  char *argv[16];
  int argc = 0;
  for (char *p = line; *p && argc < 16;) {
    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) break;
    argv[argc++] = p;
    while (*p && !isspace((unsigned char)*p)) p++;
    if (*p) *p++ = '\0';
  }
  if (argc == 0) return 0;
  const char *cmd = argv[0];
  if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 || strcmp(cmd, "q") == 0) return 1;
  if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) {
    help();
    return 0;
  }
  if (strcmp(cmd, "sheet") == 0 || strcmp(cmd, "ls") == 0) {
    print_sheet(s);
    return 0;
  }
  if (strcmp(cmd, "show") == 0) {
    int i = s->active;
    if (argc > 1 && !parse_reg(s, argv[1], &i)) {
      puts("Unknown register.");
      return 0;
    }
    print_one_register(s, i);
    return 0;
  }
  if (strcmp(cmd, "r") == 0 || strcmp(cmd, "reg") == 0) {
    int i;
    if (argc < 2 || !parse_reg(s, argv[1], &i)) {
      puts("Usage: r 3");
      return 0;
    }
    s->active = i;
    printf("Till %s\n", s->names[i]);
    return 0;
  }
  if (strcmp(cmd, "sample") == 0) {
    rdc_load_sample(s->registers);
    persist(s);
    print_sheet(s);
    return 0;
  }
  if (strcmp(cmd, "base") == 0) {
    if (argc < 2) {
      printf("Base %d\n", s->base);
      return 0;
    }
    int b = (int)strtol(argv[1], RDC_NULL, 10);
    if (!rdc_valid_base(b)) {
      puts("Base must be 100, 200, 300, 400, or 500.");
      return 0;
    }
    s->base = b;
    persist(s);
    return 0;
  }
  if (strcmp(cmd, "name") == 0) {
    int i = s->active;
    int text_i = 1;
    if (argc >= 3 && parse_reg(s, argv[1], &i)) text_i = 2;
    if (argc <= text_i) {
      puts("Usage: name Drive-thru");
      return 0;
    }
    char joined[64] = "";
    for (int a = text_i; a < argc; ++a) {
      if (joined[0]) strcat(joined, " ");
      strcat(joined, argv[a]);
    }
    rdc_set_name(s, i, joined);
    persist(s);
    printf("%s\n", s->names[i]);
    return 0;
  }
  if (strcmp(cmd, "set") == 0) {
    if (argc < 4) {
      puts("Usage: set R1 twenty 103");
      return 0;
    }
    int i;
    enum rdc_denom d;
    if (!parse_reg(s, argv[1], &i) || !rdc_parse_denom(argv[2], &d)) {
      puts("Need a till and a denom.");
      return 0;
    }
    s->registers[i].n[d] = rdc_clamp_count((int)strtol(argv[3], RDC_NULL, 10));
    s->active = i;
    persist(s);
    print_one_register(s, i);
    return 0;
  }
  if (strcmp(cmd, "clear") == 0) {
    if (argc > 1 && strcmp(argv[1], "all") == 0) {
      if (!rdc_clear_all(s)) puts("Nothing to clear.");
    } else {
      int i = s->active;
      if (argc > 1 && !parse_reg(s, argv[1], &i)) {
        puts("Unknown register.");
        return 0;
      }
      if (!rdc_clear_register(s, i)) puts("That till is already empty.");
    }
    persist(s);
    return 0;
  }
  if (strcmp(cmd, "undo") == 0) {
    if (!rdc_undo_clear(s))
      puts("Nothing to undo.");
    else
      printf("Brought back %s.\n", s->names[s->active]);
    persist(s);
    return 0;
  }
  if (strcmp(cmd, "bag") == 0) {
    snprintf(s->bag, sizeof s->bag, "%s", argc > 1 ? argv[1] : "");
    persist(s);
    return 0;
  }
  if (strcmp(cmd, "initials") == 0) {
    snprintf(s->initials, sizeof s->initials, "%s", argc > 1 ? argv[1] : "");
    persist(s);
    return 0;
  }
  if (strcmp(cmd, "slip") == 0) {
    int i = s->active;
    if (argc > 1 && !parse_reg(s, argv[1], &i)) {
      puts("Unknown register.");
      return 0;
    }
    char slip[4096];
    if (rdc_drop_slip_text(s, i, slip, sizeof slip) >= 0) fputs(slip, stdout);
    return 0;
  }
  if (strcmp(cmd, "tsv") == 0) {
    print_tsv_area(s, argc > 1 ? argv[1] : "deposit");
    return 0;
  }
  if (strcmp(cmd, "history") == 0) {
    print_history(s, argc > 1 ? argv[1] : "");
    return 0;
  }
  /* Bare "20 103" sets current till. */
  {
    enum rdc_denom d;
    if (rdc_parse_denom(cmd, &d) && argc >= 2) {
      s->registers[s->active].n[d] = rdc_clamp_count((int)strtol(argv[1], RDC_NULL, 10));
      persist(s);
      print_one_register(s, s->active);
      return 0;
    }
  }
  printf("Unknown command '%s'. Type help.\n", cmd);
  return 0;
}

int main(int argc, char **argv) {
  rdc_sheet sheet;
  rdc_sheet_init(&sheet);
  restore(&sheet);

  if (argc <= 1) {
    char date[80];
    rdc_format_sheet_date(time(RDC_NULL), date, sizeof date);
    printf("Register Drop Counter  %s\nISO C23    %s\nType help.  Enter a denom and a count, like:  20 103\n\n",
           RDC_VERSION_STAMP, date);
    print_sheet(&sheet);
    char line[512];
    for (;;) {
      printf("%s> ", sheet.names[sheet.active]);
      if (!fgets(line, sizeof line, stdin)) break;
      size_t n = strlen(line);
      while (n && (line[n - 1] == '\n' || line[n - 1] == '\r')) line[--n] = '\0';
      if (run_line(&sheet, line)) break;
    }
    persist(&sheet);
    return 0;
  }

  char joined[1024] = "";
  size_t used = 0;
  for (int i = 1; i < argc; ++i) {
    int w = snprintf(joined + used, sizeof joined - used, "%s%s", used ? " " : "", argv[i]);
    if (w < 0) break;
    used += (size_t)w;
  }
  run_line(&sheet, joined);
  persist(&sheet);
  return 0;
}
