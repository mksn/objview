#ifndef TERMINAL_H
#define TERMINAL_H
#include "objview.h"

void terminal_puts(const char *s);
void terminal_printf(const char *fmt, ...);
void terminal_display(void);
void terminal_init(void);
int terminal_keyboard(const char key);
int terminal_special(const char special);
void terminal_open(void);
void terminal_close(void);

#endif
