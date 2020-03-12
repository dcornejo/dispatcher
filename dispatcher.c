/*
 * we start with a series of dispatcher_definitions, which are a
 * path and handler.
 *
 * we break the path up into elements and build a tree out of them
 * example:
 *
 * we start with two paths /a/b/c and /a/d with handler_c() and
 * handler_d() as their handlers respectively.
 *
 * this produces a tree like this:
 *
 * [/] root_handler()
 *     [a] NULL
 *         [b] NULL
 *             [c] handler_c()
 *         [d] handler_d()
 *
 * NULL means that there is no handler defined - if the terminal
 * element of the path has a NULL handler then you look for the
 * closest ancestor that does.
 *
 * for example, if I lookup /a/b I get back a pointer to root_handler()
 * if i lookup /a/d, I get handler_d().
 *
 * if a element has a key (/a/b=c) then the list element is
 * marked with an = sign and without the key
 * so /a/b=c creates multiple entries:
 *
 *  [/]
 *      [a]
 *          [b=]
 *          [b]
 *
 * NOTE 1: there is not a mechanism to free the created structures since
 * it is intended that this tree is created only at startup. if use case
 * changes, this function is trivial.
 *
 * NOTE 2: there is no attempt to optimize list searching here, sorry. I
 * do not think that the known use cases will get big enough to make the
 * tree get too large, i do not recommend that you encode every possible
 * path, just top level key handlers.
 *
 * there are 2 functions to the API:
 * register_dispatcher_handler(): build the dispatcher table
 * get_handler(): query the dispatcher table
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#include "dispatcher.h"

/* ===== utility routines ==== */

#define PATH_CHUNKS 32

/**
 * spilt a path into elements
 *
 * given an api-path, break it up into chunks separated by '/'
 * characters. it is expected that api-paths are URI encoded, so no need
 * to deal with unescaped special characters like ', ", and /
 *
 * @param path [input] path string
 * @param plist [output] pointer to split path array
 * @param plist_len [output] pointer to storage space for path array length
 */

static void split_path(char *path, char ***plist, size_t *plist_len)
{
    size_t allocated = PATH_CHUNKS;

    /* don't modify the original copy */
    char *work = strdup(path);

    char **list = malloc(allocated * sizeof(char *));
    memset(list, 0, allocated * sizeof(char *));

    size_t len = 0;

    char *ptr = work;
    if (*ptr == '/') {
        char *new_element = strdup("/");
        list[len++] = new_element;
        ptr++;
    }

    ptr = strtok(ptr, "/");

    while (ptr != NULL) {
        if (len > allocated) {
            /* we've run out of space, allocate a bigger list */
            allocated += PATH_CHUNKS;
            list = realloc(list, allocated * sizeof(char *));
        }

        char *new_element = strdup(ptr);
        list[len++] = new_element;

        ptr = strtok(NULL, "/");
    }

    *plist = list;
    *plist_len = len;

    free(work);
}

/**
 * free a split path structure
 *
 * @param list [input] pointer to split path array
 * @param len [input] length of split path array
 */

static void split_path_free(char **list, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++) {
        free(list[i]);
    }
    free(list);
}

/**
 * print out a list of elements from a split path
 *
 * DEBUG ROUTINE
 *
 * @param list [input] pointer to split path array
 * @param len [input] length of split path array
 */

static void print_split(char **list, size_t len)
{
    printf("%lu elements: ", len);
    for (size_t i = 0; i < len; i++) {
        printf("['%s' @ %p]", list[i], (void *) list[i]);
    }
    printf("\n");
}

/**
 * print out a peer list from the tree
 *
 * this routine will print out the entire list in order, the item
 * originally passed in is indicated with an @ character
 *
 * DEBUG ROUTINE
 *
 * @param ptr [input] pointer to an element of q peer list
 */

static void print_list(dispatcher_entry *ptr)
{
    printf("list ");

    if (ptr == NULL) {
        printf("[null]\n");
        return;
    }

    dispatcher_entry *x = ptr->peer_head;

    while (x != NULL) {
        if (ptr == x) {
            printf("@");
        }
        printf("[%s]", x->node_name);
        x = x->peer;
    }
    printf("\n");
}

/**
 * find a peer of this node by name
 * search through the list pointed at by peer
 *
 * @param node [input] pointer to a node in the peer list
 * @param node_name [input] name of node we're looking for
 * @return pointer to found node or NULL
 */

static dispatcher_entry *find_peer(dispatcher_entry *node, char *node_name)
{
    if ((node == NULL) || (node_name == NULL)) {
        /*  protect against idiot users */
        return NULL;
    }

    dispatcher_entry *i = node->peer_head;

    while (i != NULL) {
        if (strcmp(node_name, i->node_name) == 0) {
            break;
        }
        i = i->peer;
    }

    return i;
}

/**
 * add a node as the last node in peer list
 *
 * @param node [input] pointer to an element of the peer list
 * @param name [input] name of new node
 * @return pointer to added/existing node
 */

static dispatcher_entry *add_peer_node(dispatcher_entry *node, char *name)
{
    dispatcher_entry *new_node = malloc(sizeof(dispatcher_entry));
    memset(new_node, 0, sizeof(dispatcher_entry));

    if (node == NULL) {
        /* this is a new node */

        new_node->node_name = strdup(name);
        new_node->peer = NULL;
        new_node->children = NULL;
        new_node->peer_head = new_node;

        return new_node;
    } else {
        /* possibly adding to the list */

        /* search for existing, or get tail end of list */
        dispatcher_entry *eptr = node->peer_head;
        while (eptr->peer != NULL) {
            if (strcmp(eptr->node_name, name) == 0) {
                return eptr;
            }
            eptr = eptr->peer;
        }

        // if eptr->node_name == name, we done
        if (strcmp(eptr->node_name, name) == 0) {
            return eptr;
        }

        new_node->node_name = strdup(name);
        new_node->peer = NULL;
        new_node->children = NULL;
        new_node->peer_head = node->peer_head;

        eptr->peer = new_node;

        return new_node;
    }
}

/**
 * add a node as a child of this node
 *
 * this is different from add_peer_node() in that it returns a
 * pointer to the head_peer of the children list where the node was
 * added.
 *
 * @param node [input] pointer to parent node of children list
 * @param name [input] name of child node
 * @return pointer to head of children list
 */

static dispatcher_entry *add_child_node(dispatcher_entry *node, char *name)
{
    dispatcher_entry *child_ptr = add_peer_node(node->children, name);
    node->children = child_ptr->peer_head;

    return child_ptr;
}

/*
 * ===== PUBLIC API FUNCTIONS =====
 */

/**
 * register a dispatcher handler
 *
 * called from initialization code to build a dispatcher tree
 *
 * @param root pointer to pointer to dispatch tree
 * @param x handler to registration data
 * @return
 */

dispatcher_entry *register_dispatcher_handler(dispatcher_entry **root, dispatcher_definition *x)
{
    char **split_path_list = NULL;
    size_t split_path_len = 0;

    if (*x->path != '/') {
        printf("%s: part '%s' must start at root\n", __func__, x->path);
        return NULL;
    }

    /*
     * get the path from the dispatcher_definition, break it
     * up to create the elements of the dispatcher table
     */
    split_path(x->path, &split_path_list, &split_path_len);

    /*
     * the first element is always a peer to the top level
     */
    dispatcher_entry *ptr = *root;

    ptr = add_peer_node(ptr, split_path_list[0]);
    if (*root == NULL) {
        *root = ptr;
    }

    for (size_t i = 1; i < split_path_len; i++) {
        ptr = add_child_node(ptr, split_path_list[i]);
    }

    /* when we get here, ptr points at last entry added */
    if (x->handler != NULL) {
        /*
         * we're adding/changing a handler
         * you could make this an error optionally
         */
        if (ptr->handler != NULL) {
            printf("%s: warning: replacing existing handler: (%s) %p -> %p\n", __func__,
                   ptr->node_name, ptr->handler, x->handler);
        }
        ptr->handler = x->handler;
    }

    /* clean up */
    split_path_free(split_path_list, split_path_len);

    return NULL;
}

/**
 * given a path, return a handler for it
 *
 * @param root [input] pointer to root of dispatcher tree
 * @param path [input] path to get handler for
 * @return pointer to handler_function for path
 */

handler_function get_handler(dispatcher_entry *root, char *path)
{
    handler_function handler = NULL;
    char **split_path_list = NULL;
    size_t split_path_len = 0;

    /* cut the path up into individual elements */
    split_path(path, &split_path_list, &split_path_len);

    /* some elements may have keys defined, strip them off */
    for (int i = 0; i < split_path_len; i++) {
        char *kptr = strchr(split_path_list[i], '=');

        if ((kptr != NULL) && (*kptr == '=')) {
            *(kptr + 1) = 0;
        }
    }

    dispatcher_entry *ptr = root;

    /* search down the tree */
    for (int i = 0; i < split_path_len; i++) {

        char *query = split_path_list[i];
        ptr = find_peer(ptr, query);

        if (ptr == NULL) {
            /* we ran out of matches, use last found handler */
            goto done;
        }
        if (ptr->handler != NULL) {
            /* if handler is defined, save it */
            handler = ptr->handler;
        }

        /* skip to next element */
        ptr = ptr->children;
    }

    done:
    return handler;
}

