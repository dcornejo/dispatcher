#include <stdio.h>
#include "clixon_dispatcher.h"

/*
 * ===== DEBUGGING CODE =====
 */

int handler_default(char *path, void *args) { printf ("%s(\"%s\"))\n", __func__, path); return 0; }
int handler_a(char *path, void *args) { printf ("%s(\"%s\")\n", __func__, path); return 0; }
int handler_aa (char *path, void *args) { printf ("%s(\"%s\")\n", __func__, path); return 0; }
int handler_b (char *path, void *args) { printf ("%s(\"%s\")\n", __func__, path); return 0; }
int handler_ab (char *path, void *args) { printf ("%s(\"%s\")\n", __func__, path); return 0; }
int handler_aba (char *path, void *args) { printf ("%s(\"%s\")\n", __func__, path); return 0; }
int handler_binterface (char *path, void *args) { printf ("%s(\"%s\")\n", __func__, path); return 0; }
int handler_binterface_key (char *path, void *args) { printf ("%s(\"%s\")\n", __func__, path); return 0; }


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

    /* register handlers */
    while (x->path != NULL) {
        register_dispatcher_handler(&htable, x);
        x++;
    }

    char *xpath = "/";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    xpath = "/a";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    xpath = "/a/dead";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    xpath = "/a/aa";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    xpath = "/a/ab";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    xpath = "/a/ab/aba";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    xpath = "/a/aa/xxx";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    xpath = "/b";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    xpath = "/b/interface";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    xpath = "/b/interface=eth0";
    printf("trying '%s'\n", xpath);
    call_handlers(htable, xpath, NULL);
    printf("====\n");

    printf("\ndone\n");
}

int main( )
{
    tester();

    printf("---\n");
    return 0;
}
