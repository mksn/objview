#ifndef terminal_h
#include "objview.h"

void terminal_puts(const char *s);
void terminal_printf(const char *fmt, ...);
void terminal_display(void);
void terminal_init(void);
void terminal_keyboard(void *keyEvent);

#endif
