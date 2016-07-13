/*
 * tcl-augeas, Tcl bindings for Augeas.
 * Copyright (C) 2015 Danyil Bohdan.
 * This code is released under the terms of the MIT license. See the file
 * LICENSE for details.
 */
#include <augeas.h>
#include <tcl.h>

/* Package information. */

#define PACKAGE "augeas"
#define VERSION "0.2.0"

/* Namespace for the extension. */

#define NS "::" PACKAGE

/* Command names. */

#define INIT "::init"
#define CLOSE "::close"
#define SAVE "::save"
#define LOAD "::load"
#define GET "::get"
#define SET "::set"
#define SETM "::setm"
#define SPAN "::span"
#define INSERT "::insert"
#define MV "::mv"
#define RM "::rm"
#define RENAME "::rename"
#define MATCH "::match"

/* Error messages. */

#define ERROR_TOKEN "can't parse token"
#define ERROR_INTEGER "integer expected"
#define ERROR_PATH "invalid path"
#define ERROR_NO_NODES "no nodes match path"
#define ERROR_UNKNOWN "unknown error"
#define ERROR_MULTIPLE "multiple nodes match path"
#define ERROR_INIT "can't initialize augeas object"

/* Usage. */

#define USAGE_INIT "root ?loadpath? ?flags?"
#define USAGE_CLOSE "token"
#define USAGE_SAVE "token"
#define USAGE_LOAD "token"
#define USAGE_GET "token path"
#define USAGE_SET "token path value"
#define USAGE_SETM "token base sub value"
#define USAGE_SPAN "token path"
#define USAGE_INSERT "token path label ?before?"
#define USAGE_MV "token src dst"
#define USAGE_RM "token path"
#define USAGE_RENAME "token src lbl"
#define USAGE_MATCH "token path"

/* Data types. */

struct AugeasData
{
    int counter;
    Tcl_HashTable table;
};

#define AUG_CDATA ((struct AugeasData *) cdata)

/* Functions */

static augeas *
parse_id(ClientData cdata, Tcl_Interp *interp, Tcl_Obj *const idobj, int del)
{
    struct AugeasData* aug_data = (struct AugeasData *) cdata;
    Tcl_HashEntry* hPtr;
    augeas* aug;

    hPtr = Tcl_FindHashEntry(&aug_data->table, Tcl_GetString(idobj));
    if (hPtr == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
        return NULL;
    }
    aug = (augeas*) Tcl_GetHashValue(hPtr);
    if (del) {
        Tcl_DeleteHashEntry(hPtr);
    }
    return aug;
}


static void
cleanup_interp(ClientData cdata, Tcl_Interp *interp)
{
    struct AugeasData* aug_data = (struct AugeasData *) cdata;
    Tcl_HashEntry* hPtr;
    Tcl_HashSearch search;
    augeas* aug;

    hPtr = Tcl_FirstHashEntry(&aug_data->table, &search);
    while (hPtr != NULL) {
        aug = (augeas*) Tcl_GetHashValue(hPtr);
        Tcl_SetHashValue(hPtr, (ClientData) NULL);
        aug_close(aug);
        hPtr = Tcl_NextHashEntry(&search);
    }
    Tcl_DeleteHashTable(&aug_data->table);
    ckfree((char *) aug_data);
}


/*
 * Initialize an Augeas object.
 * Usage: init root ?loadpath? ?flags?
 * Return value: string token of the form "::augeas::(integer)".
 * Side effects: creates an Augeas object and marks its slot as in use.
 */
static int
Init_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    const char* root = NULL;
    const char* loadpath = NULL;
    int flags = 0;
    struct AugeasData* aug_data = (struct AugeasData*) cdata;
    augeas* aug;
    Tcl_HashEntry* hPtr;
    int isNew;
    char token[64];

    if ((objc < 2) || (objc > 4)) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_INIT);
        return TCL_ERROR;
    }

    root = Tcl_GetString(objv[1]);

    if (objc >= 3) {
        loadpath = Tcl_GetString(objv[2]);
    }

    if (objc == 4) {
        int conv_result = Tcl_GetIntFromObj(interp, objv[3], &flags);
        if (conv_result != TCL_OK) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_INTEGER, -1));
            return TCL_ERROR;
        }
    }

    aug = aug_init(root, loadpath, flags);
    if (aug == NULL) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_INIT, -1));
        return TCL_ERROR;
    }

    sprintf(token, NS "::%d", aug_data->counter++);
    hPtr = Tcl_CreateHashEntry(&aug_data->table, token, &isNew);
    Tcl_SetHashValue(hPtr, (ClientData) aug);

    Tcl_SetResult(interp, token, TCL_VOLATILE);

    return TCL_OK;
}


/*
 * Reloads an Augeas object.
 * Return value: nothing.
 * Side effects: modifies an Augeas object.
 */
static int
Load_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    augeas *aug;
    int aug_result;

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_LOAD);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    aug_result = aug_load(aug);

    if (aug_result == 0) {
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("can't load", -1));
        return TCL_ERROR;
    }
}


/*
 * Close an Augeas object.
 * Return value: nothing.
 * Side effects: closes an Augeas object and marks its slot as available.
 */
static int
Close_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    augeas *aug;

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_CLOSE);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 1);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    aug_close(aug);

    return TCL_OK;
}


/*
 * Save the changes made to Augeas nodes to disk.
 * Return value: nothing.
 * Usage: save token
 * Side effects: causes Augeas to write data to disk.
 */
static int
Save_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    augeas* aug;
    int aug_result;

    if (objc != 2) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_SAVE);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    aug_result = aug_save(aug);

    if (aug_result == 0) {
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("can't save", -1));

        return TCL_ERROR;
    }
}


/*
 * Get the value of one node.
 * Usage: get token path
 * Return value: string (node value).
 * Side effects: none.
 */
static int
Get_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    augeas* aug;
    const char* path;
    const char* value;

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_GET);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    path = Tcl_GetString(objv[2]);

    int aug_result = aug_get(aug, path, &value);

    if (aug_result == 1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(value, -1));

        return TCL_OK;
    } else if (aug_result < 0) {
        Tcl_SetObjResult(interp,
                Tcl_NewStringObj(ERROR_MULTIPLE, -1));

        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("node not found", -1));

        return TCL_ERROR;
    }
}


/*
 * Set the value of one node.
 * Usage: set token path value
 * Return value: nothing.
 * Side effects: changes an Augeas object.
 */
static int
Set_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    augeas* aug;
    const char* path;
    const char* value;
    int aug_result;

    if (objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_SET);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    path = Tcl_GetString(objv[2]);
    value = Tcl_GetString(objv[3]);

    aug_result = aug_set(aug, path, value);

    if (aug_result == 0) {
        return TCL_OK;
    } else if (aug_result == -1) {
        Tcl_SetObjResult(interp,
                Tcl_NewStringObj(ERROR_MULTIPLE, -1));

        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_UNKNOWN, -1));

        return TCL_ERROR;
    }
}


/*
 * Set the values of multiple node that match the expression sub relative to
 * path base to one value.
 * Return value: the number of nodes changed.
 * Usage: setm token base sub value
 * Side effects: changes an Augeas object.
 */
static int
Setm_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    augeas* aug;
    const char* base;
    const char* sub;
    const char* value;
    int aug_result;

    if (objc != 5) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_SETM);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    base = Tcl_GetString(objv[2]);
    sub = Tcl_GetString(objv[3]);
    value = Tcl_GetString(objv[4]);

    aug_result = aug_setm(aug, base, sub, value);

    if (aug_result > 0) {
        /* Return the number of nodes changed. */

        Tcl_SetObjResult(interp, Tcl_NewIntObj(aug_result));
        return TCL_OK;
    } else if (aug_result == 0) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_NO_NODES, -1));

        return TCL_ERROR;
    } else if (aug_result == -1) {
        Tcl_SetObjResult(interp,
                Tcl_NewStringObj("can't set value", -1));

        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_UNKNOWN, -1));

        return TCL_ERROR;
    }
}


/*
 * Find the span of associated with path in its file.
 * Usage: span token path
 * Return value: [list filename [list label_start label_end]
 *                              [list value_start value_end]
 *                              [list span_start span_end]]
 * Side effects: none.
 */
static int
Span_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *list = NULL;
    Tcl_Obj *sublist = NULL;
    augeas* aug;
    const char* path;
    char* filename;
    unsigned int label_start;
    unsigned int label_end;
    unsigned int value_start;
    unsigned int value_end;
    unsigned int span_start;
    unsigned int span_end;

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_SPAN);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    path = Tcl_GetString(objv[2]);

    int aug_result = aug_span(aug, path, &filename,
            &label_start, &label_end,
            &value_start, &value_end,
            &span_start, &span_end);

    if (aug_result == 0) {
        list = Tcl_NewListObj(0, NULL);

        Tcl_ListObjAppendElement(interp, list, Tcl_NewStringObj(filename, -1));
        free(filename);

        /* Use nested lists for convenience of use with [string range]. */
        sublist = Tcl_NewListObj(0, NULL);
        Tcl_ListObjAppendElement(interp, sublist, Tcl_NewIntObj(label_start));
        Tcl_ListObjAppendElement(interp, sublist, Tcl_NewIntObj(label_end));
        Tcl_ListObjAppendElement(interp, list, sublist);

        sublist = Tcl_NewListObj(0, NULL);
        Tcl_ListObjAppendElement(interp, sublist, Tcl_NewIntObj(value_start));
        Tcl_ListObjAppendElement(interp, sublist, Tcl_NewIntObj(value_end));
        Tcl_ListObjAppendElement(interp, list, sublist);

        sublist = Tcl_NewListObj(0, NULL);
        Tcl_ListObjAppendElement(interp, sublist, Tcl_NewIntObj(span_start));
        Tcl_ListObjAppendElement(interp, sublist, Tcl_NewIntObj(span_end));
        Tcl_ListObjAppendElement(interp, list, sublist);

        Tcl_SetObjResult(interp, list);

        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp,
                Tcl_NewStringObj("path not in a file or doesn't exist", -1));

        return TCL_ERROR;
    }
}


/*
 * Insert a sibling node of path with label label before or after path.
 * Usage: insert token path label ?before?
 * Return value: nothing.
 * Side effects: changes an Augeas object.
 */
static int
Insert_Cmd(ClientData cdata, Tcl_Interp *interp,
        int objc, Tcl_Obj *const objv[])
{
    augeas* aug;
    const char* base;
    const char* label;
    int conv_result;
    int before;
    int aug_result;

    if ((objc < 4) || (objc > 5)) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_INSERT);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    base = Tcl_GetString(objv[2]);
    label = Tcl_GetString(objv[3]);

    if (objc == 4) {
        before = 0;
    } else {
        conv_result = Tcl_GetIntFromObj(interp, objv[4], &before);
        if (conv_result != TCL_OK) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
            return TCL_ERROR;
        }
    }

    aug_result = aug_insert(aug, base, label, before);

    if (aug_result == 0) {
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("can't insert node", -1));

        return TCL_ERROR;
    }
}


/*
 * Move subtree src to dst.
 * Usage: mv token src dst
 * Return value: nothing.
 * Side effects: changes an Augeas object.
 */
static int
Mv_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    augeas* aug;
    const char* src;
    const char* dst;
    int aug_result;

    if (objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_MV);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    src = Tcl_GetString(objv[2]);
    dst = Tcl_GetString(objv[3]);

    aug_result = aug_mv(aug, src, dst);

    if (aug_result == 0) {
        return TCL_OK;
    } else if (aug_result == 1) {
        Tcl_SetObjResult(interp,
                Tcl_NewStringObj("can't move subtree", -1));

        return TCL_ERROR;
    } else { /* aug_result < 0 */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_UNKNOWN, -1));

        return TCL_ERROR;
    }
}


/*
 * Remove all nodes that match path.
 * Usage: rm token path
 * Return value: the number of nodes removed.
 * Side effects: changes an Augeas object.
 */
static int
Rm_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    augeas* aug;
    const char* path;
    int aug_result;

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_RM);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    path = Tcl_GetString(objv[2]);

    aug_result = aug_rm(aug, path);

    if (aug_result > 0) {
        /* Return the number of nodes removed. */
        Tcl_SetObjResult(interp, Tcl_NewIntObj(aug_result));

        return TCL_OK;
    } else if (aug_result == 0) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_NO_NODES, -1));

        return TCL_ERROR;
    } else { /* aug_result < 0 */
        Tcl_SetObjResult(interp, Tcl_NewStringObj("invalid path", -1));

        return TCL_ERROR;
    }
}


#ifndef NO_AUG_RENAME
/*
 * Change the label of all nodes that match src to lbl.
 * Usage: rename token src lbl
 * Return value: nothing.
 * Side effects: changes an Augeas object.
 */
static int
Rename_Cmd(ClientData cdata, Tcl_Interp *interp,
        int objc, Tcl_Obj *const objv[])
{
    augeas* aug;
    const char* src;
    const char* lbl;
    int aug_result;

    if (objc != 4) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_RENAME);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    src = Tcl_GetString(objv[2]);
    lbl = Tcl_GetString(objv[3]);

    aug_result = aug_rename(aug, src, lbl);

    if (aug_result > 0) {
        /* Return the number of nodes renamed. */
        Tcl_SetObjResult(interp, Tcl_NewIntObj(aug_result));

        return TCL_OK;
    } else if (aug_result == 0) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_NO_NODES, -1));

        return TCL_ERROR;
    } else { /* aug_result < 0 */
        Tcl_SetObjResult(interp, Tcl_NewStringObj("invalid path", -1));

        return TCL_ERROR;
    }
}
#endif


/*
 * Find all nodes that match path.
 * Usage: match token path
 * Return value: a list of strings (paths).
 * Side effects: none.
 */
static int
Match_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    Tcl_Obj *list = NULL;
    int aug_result;
    augeas* aug;
    const char *path;
    char **matches = NULL;
    int i;

    if (objc != 3) {
        Tcl_WrongNumArgs(interp, 1, objv, USAGE_MATCH);
        return TCL_ERROR;
    }

    aug = parse_id(cdata, interp, objv[1], 0);
    if (aug == NULL) {
        return TCL_ERROR;
    }

    path = Tcl_GetString(objv[2]);

    aug_result = aug_match(aug, path, &matches);

    if (aug_result >= 0) {
        /* Return the matched paths. */

        /* This is different from Setm_Cmd and Rm_Cmd in we expect the user
         * to consider the case when the result is empty as part of normal
         * operation. Hence, no error is generated. */

        list = Tcl_NewListObj(0, NULL);
        if (aug_result > 0) {
            for (i = 0; i < aug_result; i++)
            {
                Tcl_ListObjAppendElement(interp, list,
                        Tcl_NewStringObj(matches[i], -1));
                free(matches[i]);
            }
        }
        free(matches);

        Tcl_SetObjResult(interp, list);

        return TCL_OK;
    } else { /* aug_result < 0 */
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_PATH, -1));

        return TCL_ERROR;
    }
}


/*
 * Augeas_Init -- Called when Tcl loads the extension.
 */
int DLLEXPORT
Augeas_Init(Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr;
    struct AugeasData *augeas_data;

    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        return TCL_ERROR;
    }

    /* Create the namespace. */
    if (Tcl_FindNamespace(interp, NS, NULL, 0) == NULL) {
        nsPtr = Tcl_CreateNamespace(interp, NS, NULL, NULL);
        if (nsPtr == NULL) {
            return TCL_ERROR;
        }
    }

    augeas_data = (struct AugeasData *) ckalloc(sizeof(struct AugeasData));
    augeas_data->counter = 1;
    Tcl_InitHashTable(&augeas_data->table, TCL_STRING_KEYS);

    Tcl_CreateObjCommand(interp, NS INIT, Init_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS CLOSE, Close_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS SAVE, Save_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS LOAD, Load_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS GET, Get_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS SET, Set_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS SETM, Setm_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS SPAN, Span_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS INSERT, Insert_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS MV, Mv_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS RM, Rm_Cmd, augeas_data, NULL);
    #ifndef NO_AUG_RENAME
    Tcl_CreateObjCommand(interp, NS RENAME, Rename_Cmd, augeas_data, NULL);
    #endif
    Tcl_CreateObjCommand(interp, NS MATCH, Match_Cmd, augeas_data, NULL);
    Tcl_CallWhenDeleted(interp, cleanup_interp, augeas_data);
    Tcl_PkgProvide(interp, PACKAGE, VERSION);

    return TCL_OK;
}

int DLLEXPORT
Tclaugeas_Init(Tcl_Interp *interp)
{
    return Augeas_Init(interp);
}
