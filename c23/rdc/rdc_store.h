#ifndef RDC_STORE_H
#define RDC_STORE_H

#include "rdc_history.h"

#ifdef __cplusplus
extern "C" {
#endif

enum { RDC_NAME_MAX = 20 };

typedef struct rdc_sheet {
  int base;
  int active;
  rdc_counts registers[RDC_REGISTER_COUNT];
  char names[RDC_REGISTER_COUNT][24];
  rdc_history history;
  char bag[64];
  char initials[32];
} rdc_sheet;

void rdc_sheet_init(rdc_sheet *s);
void rdc_default_names(char names[RDC_REGISTER_COUNT][24]);
void rdc_set_name(rdc_sheet *s, int index, const char *raw);
bool rdc_clear_register(rdc_sheet *s, int index);
bool rdc_clear_all(rdc_sheet *s);
bool rdc_undo_clear(rdc_sheet *s);

int rdc_save_state_path(const rdc_sheet *s, const char *path);
int rdc_load_state_path(rdc_sheet *s, const char *path);
int rdc_save_names_path(const rdc_sheet *s, const char *path);
int rdc_load_names_path(rdc_sheet *s, const char *path);
int rdc_save_history_path(const rdc_sheet *s, const char *path);
int rdc_load_history_path(rdc_sheet *s, const char *path);
int rdc_save_slip_path(const rdc_sheet *s, const char *path);
int rdc_load_slip_path(rdc_sheet *s, const char *path);

int rdc_drop_slip_text(const rdc_sheet *s, int index, char *buf, size_t n);

#ifdef __cplusplus
}
#endif

#endif
