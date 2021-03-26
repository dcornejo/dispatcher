#include <stdio.h>
#include "dispatcher.h"

/*
 * ===== DEBUGGING CODE =====
 */

int handler_default(char *path, handler_args_t *args) { printf ("%s: '%s'\n", __func__, path); return 0; }
int handler_a(char *path, handler_args_t *args) { printf ("%s: path is '%s'\n", __func__, path); return 0; }
int handler_aa (char *path, handler_args_t *args) { printf ("%s: path is '%s'\n", __func__, path); return 0; }
int handler_b (char *path, handler_args_t *args) { printf ("%s: '%s'\n", __func__, path); return 0; }
int handler_ab (char *path, handler_args_t *args) { printf ("%s: '%s'\n", __func__, path); return 0; }
int handler_aba (char *path, handler_args_t *args) { printf ("%s: '%s'\n", __func__, path); return 0; }
int handler_binterface (char *path, handler_args_t *args) { printf ("%s: '%s'\n", __func__, path); return 0; }
int handler_binterface_key (char *path, handler_args_t *args) { printf ("%s: '%s'\n", __func__, path); return 0; }


dispatcher_definition test_table[] = {
        { "/a", handler_a },
        { "/a/aa", handler_aa },
        { "/a/dead", NULL },
        { "/a/ab", handler_ab },
        { "/a/ab/aba", handler_aba },
        { "/b", handler_b },
        { "/b/interface", handler_binterface },
        { "/b/interface=", handler_binterface_key },
        { "/", handler_default },
        { NULL }
};

void tester(void)
{
    dispatcher_definition *x = test_table;
    dispatcher_entry_t *htable = NULL;
    handler_function x2;

    while (x->path != NULL) {
        register_dispatcher_handler(&htable, x);
        x++;
    }

    char *xpath = "/";
    call_handlers(htable, xpath);
    printf("====\n");

    xpath = "/a";
    call_handlers(htable, xpath);
    printf("====\n");

    xpath = "/a/dead";
    call_handlers(htable, xpath);
    printf("====\n");

    xpath = "/a/aa";
    call_handlers(htable, xpath);
    printf("====\n");

    xpath = "/a/ab";
    call_handlers(htable, xpath);
    printf("====\n");

    xpath = "/a/ab/aba";
    call_handlers(htable, xpath);
    printf("====\n");


    xpath = "/a/aa/xxx";
    call_handlers(htable, xpath);
    printf("====\n");


    xpath = "/b";
    call_handlers(htable, xpath);
    printf("====\n");

    xpath = "/b/interface";
    call_handlers(htable, xpath);
    printf("====\n");


    xpath = "/b/interface=eth0";
    call_handlers(htable, xpath);
    printf("====\n");


    printf("\ndone\n");
}

int main( )
{
    tester();

    printf("---\n");
    return 0;
}
