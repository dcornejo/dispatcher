#include <stdio.h>
#include "dispatcher.h"

/*
 * ===== DEBUGGING CODE =====
 */

int handler_default (char *path) { printf ("%s: '%s'\n", __func__, path); return 0; }
int handler_a (char *path) { printf ("%s: '%s'\n", __func__, path); return 0; }
int handler_b (char *path) { printf ("%s: '%s'\n", __func__, path); return 0; }
int handler_ab (char *path) { printf ("%s: '%s'\n", __func__, path); return 0; }
int handler_aba (char *path) { printf ("%s: '%s'\n", __func__, path); return 0; }

dispatcher_definition test_table[] = {
        { "/node_a", handler_a },
        { "/node_a/node_aa", NULL },
        { "/node_a/node_ab", handler_ab },
        { "/node_a/node_ab/node_aba", handler_aba },
        { "/node_b", handler_b },
        { "/node_b/interface", handler_b },
        { "/node_b/interface=", handler_b },
        { "/", handler_default },
        { NULL }
};

void tester(void)
{
    dispatcher_definition *x = test_table;
    dispatcher_entry *htable = NULL;

    while (x->path != NULL) {
        register_dispatcher_handler(&htable, x);
        x++;
    }

    char *xpath = "/";
    handler_function x2 = get_handler(htable, xpath);
    (*x2)(xpath);

    xpath = "/node_a";
    x2 = get_handler(htable, xpath);
    (*x2)(xpath);

    xpath = "/node_a/node_aa";
    x2 = get_handler(htable, xpath);
    (*x2)(xpath);

    xpath = "/node_a/node_aa/node_aaa";
    x2 = get_handler(htable, xpath);
    (*x2)(xpath);

    xpath = "/node_b";
    x2 = get_handler(htable, xpath);
    (*x2)(xpath);

    printf("\ndone\n");
}

int main( )
{
    tester();

    printf("---\n");
    return 0;
}
