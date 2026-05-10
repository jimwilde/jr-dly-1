#ifndef MENU_H
#define MENU_H

#include <stdatomic.h>
#include <stdbool.h>

void print_menu(void);
void on_channels_changed(void *context);
void run_menu(void *linkHandle, _Atomic(bool) *shutdown);

#endif /* MENU_H */
