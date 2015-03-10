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
#define VERSION "0.1"

/* Namespace for the extension. */

#define NS "::" PACKAGE

/* Limit on the maximum number of Augeas objects. */

#define MAX_COUNT 16

/* Command names. */

#define INIT "::init"
#define CLOSE "::close"
#define SAVE "::save"
#define LOAD "::load"
#define GET "::get"
#define SET "::set"
#define SETM "::setm"
#define INSERT "::insert"
#define MV "::mv"
#define RM "::rm"
#define MATCH "::match"

/* Error messages. */

#define ERROR_TOKEN "can't parse token"
#define ERROR_INTEGER "integer expected"
#define ERROR_ARGS "wrong # args: should be "
#define ERROR_PATH "invalid path"
#define ERROR_NO_NODES "no nodes match path"
#define ERROR_UNKNOWN "unknown error"
#define ERROR_MULTIPLE "multiple nodes match path"

/* Usage. */

#define USAGE_INIT "\"init root ?loadpath? ?flags?\""
#define USAGE_CLOSE "\"close token\""
#define USAGE_SAVE "\"save token\""
#define USAGE_LOAD "\"load token\""
#define USAGE_GET "\"get token path\""
#define USAGE_SET "\"set token path value\""
#define USAGE_SETM "\"setm token base sub value\""
#define USAGE_INSERT "\"insert token path label ?before?\""
#define USAGE_MV "\"mv token src dst\""
#define USAGE_RM "\"rm token path\""
#define USAGE_MATCH "\"match token path\""

#define AUG_CDATA ((struct AugeasData *) cdata)

/* Data types. */

struct AugeasData
{
    augeas *object[MAX_COUNT];
    int active[MAX_COUNT];
};

/* Functions */

/* Set id to the integer value of the Augeas interpreter token. */
static int
parse_id(ClientData cdata, Tcl_Interp *interp, Tcl_Obj *const idobj, int *id)
{
    Tcl_Obj* cmd[2];
    Tcl_Obj* result_obj = NULL;
    int success;
    int conv_result;

    cmd[0] = Tcl_NewStringObj(NS "::parseToken", -1);
    cmd[1] = idobj;
    Tcl_IncrRefCount(cmd[0]);
    Tcl_IncrRefCount(cmd[1]);
    success = Tcl_EvalObjv(interp, 2, cmd, 0);
    Tcl_DecrRefCount(cmd[0]);
    Tcl_DecrRefCount(cmd[1]);

    if (success == TCL_OK) {
        result_obj = Tcl_GetObjResult(interp);

        Tcl_IncrRefCount(result_obj);
        conv_result = Tcl_GetIntFromObj(interp, result_obj, id);
        Tcl_DecrRefCount(result_obj);
        Tcl_FreeResult(interp);

        if (conv_result != TCL_OK) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
            return TCL_ERROR;
        }

        (*id)--; /* Tokens start from one while actual ids start from zero. */

        if (AUG_CDATA->active[*id] != 1) {
            Tcl_SetObjResult(interp, Tcl_NewStringObj("object not found", -1));
            return TCL_ERROR;
        }

        return TCL_OK;
    } else {
        return success;
    }
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
    char* root = NULL;
    char* loadpath = NULL;
    int flags = 0;
    augeas *aug;
    int id = -1;
    int i;
    Tcl_Obj *token[1];
    Tcl_Obj *result;

    if ((objc < 2) || (objc > 4)) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_INIT, -1));
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

    /* Find an unused object slot. */
    for (i = 0; i < MAX_COUNT; i++) {
        if (AUG_CDATA->active[i] == 0) {
            id = i;
            break;
        }
    }
    if (id == -1) {
        return TCL_ERROR;
    }

    AUG_CDATA->object[id] = aug;
    AUG_CDATA->active[id] = 1;

    token[0] = Tcl_NewIntObj(id + 1);
    result = Tcl_Format(interp, NS "::%d", 1, token);

    Tcl_SetObjResult(interp, result);

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
    int id;
    int success;
    int aug_result;

    if (objc != 2) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_LOAD, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }

    aug_result = aug_load(AUG_CDATA->object[id]);

    if (aug_result == 0) {
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp,
                    Tcl_NewStringObj("can't load", -1));
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
    int id;
    int success;

    if (objc != 2) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_CLOSE, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }

    aug_close(AUG_CDATA->object[id]);
    AUG_CDATA->active[id] = 0;

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
    int id;
    int success;
    int aug_result;

    if (objc != 2) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_SAVE, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }

    aug_result = aug_save(AUG_CDATA->object[id]);

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
    int id;
    int success;
    const char* path;
    const char* value;

    if (objc != 3) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_GET, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }

    path = Tcl_GetString(objv[2]);

    int aug_result = aug_get(AUG_CDATA->object[id], path, &value);

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
    int id;
    int success;
    const char* path;
    const char* value;
    int aug_result;

    if (objc != 4) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_SET, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }

    path = Tcl_GetString(objv[2]);
    value = Tcl_GetString(objv[3]);

    aug_result = aug_set(AUG_CDATA->object[id], path, value);

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
    int id;
    int success;
    const char* base;
    const char* sub;
    const char* value;
    int aug_result;

    if (objc != 5) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_SETM, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }


    base = Tcl_GetString(objv[2]);
    sub = Tcl_GetString(objv[3]);
    value = Tcl_GetString(objv[4]);

    aug_result = aug_setm(AUG_CDATA->object[id], base, sub, value);

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
 * Insert a sibling node of path with label label before or after path.
 * Usage: insert token path label ?before?
 * Return value: nothing.
 * Side effects: changes an Augeas object.
 */
static int
Insert_Cmd(ClientData cdata, Tcl_Interp *interp,
        int objc, Tcl_Obj *const objv[])
{
    int id;
    int success;
    const char* base;
    const char* label;
    int conv_result;
    int before;
    int aug_result;

    if ((objc < 4) || (objc > 5)) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_INSERT, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
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

    aug_result = aug_insert(AUG_CDATA->object[id], base, label, before);

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
    int id;
    int success;
    const char* src;
    const char* dst;
    int aug_result;

    if (objc != 4) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_MV, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }

    src = Tcl_GetString(objv[2]);
    dst = Tcl_GetString(objv[3]);

    aug_result = aug_mv(AUG_CDATA->object[id], src, dst);

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
    int id;
    int success;
    const char* path;
    int aug_result;

    if (objc != 3) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_RM, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }


    path = Tcl_GetString(objv[2]);

    aug_result = aug_rm(AUG_CDATA->object[id], path);

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
    int id;
    int success;
    const char *path;
    char **matches = NULL;
    int i;

    if (objc != 3) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_ARGS USAGE_MATCH, -1));
        return TCL_ERROR;
    }

    success = parse_id(cdata, interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }

    path = Tcl_GetString(objv[2]);

    aug_result = aug_match(AUG_CDATA->object[id], path, &matches);

    list = Tcl_NewListObj(0, NULL);

    if (aug_result >= 0) {
        /* Return the matched paths. */

        /* This is different from Setm_Cmd and Rm_Cmd in we expect the user
         * to consider the case when the result is empty as part of normal
         * operation. Hence, no error is generated. */

        if (aug_result > 0) {
            for (i = 0; i < aug_result; i++)
            {
                Tcl_ListObjAppendElement(interp, list,
                        Tcl_NewStringObj(matches[i], -1));
                free(matches[i]);
            }
        }

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
Tclaugeas_Init(Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr;
    int i;
    struct AugeasData *augeas_data;
    augeas_data = (struct AugeasData *) ckalloc(sizeof(struct AugeasData));

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

    for (i = 0; i < MAX_COUNT; i++) {
        augeas_data->active[i] = 0;
    }

    Tcl_CreateObjCommand(interp, NS INIT, Init_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS CLOSE, Close_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS SAVE, Save_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS LOAD, Load_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS GET, Get_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS SET, Set_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS SETM, Setm_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS INSERT, Insert_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS MV, Mv_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS RM, Rm_Cmd, augeas_data, NULL);
    Tcl_CreateObjCommand(interp, NS MATCH, Match_Cmd, augeas_data, NULL);
    Tcl_PkgProvide(interp, PACKAGE, VERSION);

    Tcl_Eval(interp, "proc " NS "::parseToken token { \
        if {![regexp {^(?:" NS "::)?([1-9]+[0-9]*)$} $token _ id]} { \
            error {" ERROR_TOKEN "}\n\
        } \n\
        return $id \n\
    }");

    return TCL_OK;
}
