
#ifndef DISPATCH_DISPATCHER_H
#define DISPATCH_DISPATCHER_H

/*
 * cache_data is a structure that can be used to pass stored
 * data around
 */

typedef struct {
    char *cached_path;
} cached_data;

/*
 * prototype for a function to handle the path
 * minimally needs the path it's working on, but probably
 * we want to hand down cached data somehow
 */

typedef int (*handler_function)(char *path);

/*
 * this structure is used to map a handler to a path
 */
typedef struct {
    char *path;
    handler_function handler;
} dispatcher_definition;

/*
 * the dispatcher_entry is the structure created from
 * the registered dispatcher_definitions
 */

struct _dispatcher_entry;
typedef struct _dispatcher_entry dispatcher_entry;

struct _dispatcher_entry {
    /*
     * the name of this node, NOT the complete path
     */
    char *node_name;

    /*
     * peer points at peer to right of this one
     * if NULL then this is the rightmost and last on list
     */
    dispatcher_entry *peer;

    /*
     * peer_head points at leftmost peer at this level
     * if NULL, then this is the leftmost and first on the list
     */
    dispatcher_entry *peer_head;

    /*
     * point at parent node
     * if NULL then we are at the root node
     */
    dispatcher_entry *parent;

    /*
     * points at peer_head of children list
     * if NULL, then no children
     */
    dispatcher_entry *children;

    /*
     * pointer to handler function for this node
     */
    handler_function handler;
};

dispatcher_entry *register_dispatcher_handler (dispatcher_entry **root, dispatcher_definition *x);
handler_function get_handler (dispatcher_entry *root, char *path);

#endif /* DISPATCH_DISPATCHER_H */
