/*
 * Copyright 2021 Rubicon Communications LLC (Netgate)
 */

#ifndef DISPATCH_DISPATCHER_H
#define DISPATCH_DISPATCHER_H

/*
 * prototype for a function to handle the path
 * minimally needs the path it's working on, but probably
 * we want to hand down cached data somehow
 */

typedef int (*handler_function)(char *path, void *args);

/*
 * this structure is used to map a handler to a path
 */
typedef struct {
    char *path;
    handler_function handler;
} dispatcher_definition;

/*
 * the dispatcher_entry_t is the structure created from
 * the registered dispatcher_definitions
 */

struct _dispatcher_entry;
typedef struct _dispatcher_entry dispatcher_entry_t;

struct _dispatcher_entry {
    /*
     * the name of this node, NOT the complete path
     */
    char *node_name;

    /*
     * peer points at peer to the right of this one
     * if NULL then this is the rightmost and last on list
     */
    dispatcher_entry_t *peer;

    /*
     * peer_head points at leftmost peer at this level
     * if NULL, then this is the leftmost and first on the list
     */
    dispatcher_entry_t *peer_head;

    /*
     * points at peer_head of children list
     * if NULL, then no children
     */
    dispatcher_entry_t *children;

    /*
     * pointer to handler function for this node
     */
    handler_function handler;
};

dispatcher_entry_t *register_dispatcher_handler (dispatcher_entry_t **root, dispatcher_definition *x);
handler_function get_handler (dispatcher_entry_t *root, char *path);
int call_handlers(dispatcher_entry_t *root, char *path, void *user_args);

#endif /* DISPATCH_DISPATCHER_H */
