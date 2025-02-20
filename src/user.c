#include "user.h"
#include "util.h"
#include "item.h"
#include "log.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void layout(struct Monitor *monitor, const union Arg *arg) {
    if ((monitor->layout == 0 && !arg->ui) || (monitor->layout == layoutcount-1 && arg->ui))
        return;

    if (arg->ui)
        monitor->layout++;
    else
        monitor->layout--;

    zdwl_ipc_output_v2_set_layout(monitor->dwl_output, monitor->layout);
}

void spawn(struct Monitor *monitor, const union Arg *arg) {
  if (fork() != 0)
    return;

  char* const* argv = arg->v;
  setsid();
  execvp(argv[0], argv);
  fprintf(stderr, "dwl-bar: execvp %s", argv[0]);
  perror(" failed\n");
  exit(1);
}

void systray(struct Monitor *monitor, const union Arg *arg) {
    const struct ItemClick *click = arg->v;
    const struct Item *item = click->item;
    const char *method = button_to_method(click->button);

    if (STRING_EQUAL(method, "nop"))
        return;
    if (STRINGN_EQUAL(method, "Scroll", strlen("Scroll"))) {
        char direction = method[strlen("Scroll")];
        char *orientation = (direction == 'U' || direction == 'D') ? "vertical" : "horizontal";
        int sign = (direction == 'U' || direction == 'L') ? -1 : 1;

        sd_bus_call_method_async(item->tray->bus, NULL, item->service, item->path,
                item->interface, "Scroll", NULL, NULL, "is", sign, orientation);
    } else {
        sd_bus_call_method_async(item->tray->bus, NULL, item->service, item->path,
                item->interface, method, NULL, NULL, "ii", click->x, click->y);
    }
}

void tag(struct Monitor *monitor, const union Arg *arg) {
    zdwl_ipc_output_v2_set_client_tags(monitor->dwl_output, 0, 1<<arg->ui);
}

void toggle_view(struct Monitor *monitor, const union Arg *arg) {
    zdwl_ipc_output_v2_set_tags(monitor->dwl_output, monitor->tags ^ (1<<arg->ui), 0);
}

void view(struct Monitor *monitor, const union Arg *arg) {
    zdwl_ipc_output_v2_set_tags(monitor->dwl_output, 1<<arg->ui, 1);
}
