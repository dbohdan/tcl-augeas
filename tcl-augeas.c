/*
 * tcl-augeas, Tcl bindings for Augeas.
 * Copyright (C) 2015 Danyil Bohdan.
 * This code is released under the terms of the MIT license. See the file
 * LICENSE for details.
 */
#include <augeas.h>
#include <tcl.h>

/* Namespace for the extension. */

#define PACKAGE "augeas"
#define NS "::" PACKAGE

/* Extension version. */

#define VERSION "0.1"

/* Limit the maximum number of Augeas objects. */

#define MAX_COUNT 16

/* Command names */

#define INIT "::init"
#define CLOSE "::close"
#define SAVE "::save"

#define GET "::get"
#define SET "::set"
#define SETM "::setm"
#define INSERT "::insert"
#define MV "::mv"
#define RM "::rm"
#define MATCH "::match"

/* Error messages */

#define ERROR_TOKEN "cannot parse token"

/* Globals */

augeas * augeas_objects[MAX_COUNT];
int augeas_object_active[MAX_COUNT];

/* Functions */

/* Set id to the integer value of the Augeas interpreter token. */
static int
parse_id(Tcl_Interp *interp, Tcl_Obj *const idobj, int * id)
{
    Tcl_Obj* cmd[2];
    Tcl_Obj* result_obj = NULL;
    int success;

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
        Tcl_GetIntFromObj(interp, result_obj, id); /* TODO */
        (*id)--; /* Tokens start from one while real ids start from zero. */
        Tcl_DecrRefCount(result_obj);

        if (augeas_object_active[*id] != 1) {
            return TCL_ERROR;
        }

        return TCL_OK;
    } else {
        return success;
    }
}

/*
 *
 * Usage: init root ?loadpath? ?flags?
 * Return value: string token of the form "::augeas::(integer)".
 * Side effects: creates an Augeas object and marks its slot as in use.
 */
static int
Init_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if ((objc < 2) || (objc > 4)) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj(
                "wrong # args: should be \"init root ?loadpath? ?flags?\"",
                -1
            )
        );
        return TCL_ERROR;
    }

    char* root = Tcl_GetString(objv[1]);

    char* loadpath = NULL;
    if (objc >= 3) {
        loadpath = Tcl_GetString(objv[2]);
    }

    int flags = 0;
    if (objc == 4) {
        Tcl_GetIntFromObj(interp, objv[3], &flags); /* TODO */
    }

    augeas *aug;
    aug = aug_init(root, loadpath, flags);

    int id = -1;
    int i;
    /* Find unused object slot. */
    for (i = 0; i < MAX_COUNT; i++) {
        if (augeas_object_active[id] == 0) {
            id = i;
            break;
        }
    }

    if (id == -1) {
        return TCL_ERROR;
    }

    augeas_objects[id] = malloc(sizeof(augeas *));
    augeas_objects[id] = aug;
    augeas_object_active[id] = 1;

    Tcl_Obj * token[1];
    token[0] = Tcl_NewIntObj(id + 1);
    Tcl_Obj *const result = Tcl_Format(interp, NS "::%d", 1, token);

    Tcl_SetObjResult(interp, result);

    return TCL_OK;
}

/*
 * Return value: nothing.
 * Side effects: closes an Augeas object and marks its slot as available.
 */
static int
Close_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if (objc != 2) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj("wrong # args: should be \"close token\"", -1)
        );
        return TCL_ERROR;
    }

    int id;
    int success = parse_id(interp, objv[1], &id);
    if (success != TCL_OK) {
        return success;
    }

    if (augeas_object_active[id] == 1) {
        aug_close(augeas_objects[id]);
        augeas_object_active[id] = 0;

        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("cannot close", -1));
        return TCL_ERROR;
    }
}

/*
 *
 * Return value: nothing.
 * Usage: save token
 * Side effects: causes Augeas to write data to disk.
 */
static int
Save_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    if (objc != 2) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj("wrong # args: should be \"save token\"", -1)
        );
        return TCL_ERROR;
    }

    int id;
    int success = parse_id(interp, objv[1], &id);
    if (success != TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
        return success;
    }

    aug_save(augeas_objects[id]);

    return TCL_OK;
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
    if (objc != 3) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj("wrong # args: should be \"get token path\"", -1)
        );
        return TCL_ERROR;
    }

    int id;
    int success = parse_id(interp, objv[1], &id);
    if (success != TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
        return success;
    }


    const char* path = Tcl_GetString(objv[2]);
    const char* value[255]; /* TODO: BUFFER OVERFLOW? */

    int aug_result = aug_get(augeas_objects[id], path, value);

    if (aug_result == 1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(*value, -1));

        return TCL_OK;
    } else if (aug_result < 0) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj("multiple nodes match path", -1)
        );

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
    if (objc != 4) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj(
                "wrong # args: should be \"set token path value\"",
                -1
            )
        );
        return TCL_ERROR;
    }

    int id;
    int success = parse_id(interp, objv[1], &id);
    if (success != TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
        return success;
    }


    const char* path = Tcl_GetString(objv[2]);
    const char* value = Tcl_GetString(objv[3]);

    int aug_result = aug_set(augeas_objects[id], path, value);

    if (aug_result == 0) {
        return TCL_OK;
    } else if (aug_result == -1) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj("multiple nodes match path", -1)
        );

        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("unknown error", -1));

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
Setm_Cmd(ClientData cdata, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
    if (objc != 5) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj(
                "wrong # args: should be \"setm token base sub value\"",
                -1
            )
        );
        return TCL_ERROR;
    }

    int id;
    int success = parse_id(interp, objv[1], &id);
    if (success != TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
        return success;
    }


    const char* base = Tcl_GetString(objv[2]);
    const char* sub = Tcl_GetString(objv[3]);
    const char* value = Tcl_GetString(objv[4]);

    int aug_result = aug_setm(augeas_objects[id], base, sub, value);

    if (aug_result > 0) {
        /* Return the number of nodes changed. */

        Tcl_SetObjResult(interp, Tcl_NewIntObj(aug_result));
        return TCL_OK;
    } else if (aug_result == 0) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("no nodes matched path", -1));

        return TCL_ERROR;
    } else if (aug_result == -1) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj("could not set value", -1)
        );

        return TCL_ERROR;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("unknown error", -1));

        return TCL_ERROR;
    }
}

/*
 * Insert a sibling node of path with label label before or after path.
 * Usage: insert token path label before
 * Return value: nothing.
 * Side effects: changes an Augeas object.
 */
static int
Insert_Cmd(ClientData cdata, Tcl_Interp *interp,
        int objc, Tcl_Obj *const objv[])
{
    if (objc != 5) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj(
                "wrong # args: should be \"insert token path label before\"",
                -1
            )
        );
        return TCL_ERROR;
    }

    int id;
    int success = parse_id(interp, objv[1], &id);
    if (success != TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
        return success;
    }


    const char* base = Tcl_GetString(objv[2]);
    const char* label = Tcl_GetString(objv[3]);
    int before;
    Tcl_GetIntFromObj(interp, objv[4], &before); /* TODO */

    int aug_result = aug_insert(augeas_objects[id], base, label, before);

    if (aug_result == 0) {
        return TCL_OK;
    } else {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("insert failed", -1));

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
    if (objc != 4) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj(
                "wrong # args: should be \"mv token src dst\"",
                -1
            )
        );
        return TCL_ERROR;
    }

    int id;
    int success = parse_id(interp, objv[1], &id);
    if (success != TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
        return success;
    }

    const char* src = Tcl_GetString(objv[2]);
    const char* dst = Tcl_GetString(objv[3]);

    int aug_result = aug_mv(augeas_objects[id], src, dst);

    if (aug_result == 0) {
        return TCL_OK;
    } else if (aug_result == 1) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("move failed", -1));

        return TCL_ERROR;
    } else { /* aug_result < 0 */
        Tcl_SetObjResult(interp, Tcl_NewStringObj("unknown error", -1));

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
    if (objc != 3) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj(
                "wrong # args: should be \"rm token path\"",
                -1
            )
        );
        return TCL_ERROR;
    }

    int id;
    int success = parse_id(interp, objv[1], &id);
    if (success != TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
        return success;
    }


    const char* path = Tcl_GetString(objv[2]);

    int aug_result = aug_rm(augeas_objects[id], path);

    if (aug_result > 0) {
        /* Return the number of nodes removed. */
        Tcl_SetObjResult(interp, Tcl_NewIntObj(aug_result));

        return TCL_OK;
    } else if (aug_result == 0) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj("no nodes matched path", -1));

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
    if (objc != 3) {
        Tcl_SetObjResult(
            interp,
            Tcl_NewStringObj(
                "wrong # args: should be \"match token path\"",
                -1
            )
        );
        return TCL_ERROR;
    }

    int id;
    int success = parse_id(interp, objv[1], &id);
    if (success != TCL_OK) {
        Tcl_SetObjResult(interp, Tcl_NewStringObj(ERROR_TOKEN, -1));
        return success;
    }

    const char* path = Tcl_GetString(objv[2]);
    char** matches = NULL;

    int aug_result = aug_match(augeas_objects[id], path, &matches);

    Tcl_Obj * list = Tcl_NewListObj(0, NULL);


    if (aug_result >= 0) {
        /* Return the matched paths. */

        /* Different from Mv_Cmd and Rm_Cmd since we expect the user to
         * consider the case when the result is empty. */

        if (aug_result > 0) {
            int i;
            for (i = 0; i < aug_result; i++)
            {
                Tcl_ListObjAppendElement(interp, list, Tcl_NewStringObj(matches[i], -1));
            }
        }

        Tcl_SetObjResult(interp, list);

        return TCL_OK;
    } else { /* aug_result < 0 */
        Tcl_SetObjResult(interp, Tcl_NewStringObj("invalid path", -1));

        return TCL_ERROR;
    }
}


/*
 * Augeas_Init -- Called when Tcl loads the extension.
 */
int DLLEXPORT
Tclaugeas_Init(Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr; /* pointer to hold our own new namespace */

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

    int i;
    for (i = 0; i < MAX_COUNT; i++) {
        augeas_object_active[i] = 0;
    }

    Tcl_CreateObjCommand(interp, NS INIT, Init_Cmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS CLOSE, Close_Cmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS SAVE, Save_Cmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS GET, Get_Cmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS SET, Set_Cmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS SETM, Setm_Cmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS INSERT, Insert_Cmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS MV, Mv_Cmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS RM, Rm_Cmd, NULL, NULL);
    Tcl_CreateObjCommand(interp, NS MATCH, Match_Cmd, NULL, NULL);
    Tcl_PkgProvide(interp, PACKAGE, VERSION);

    Tcl_Eval(interp, "proc " NS "::parseToken token { \
        if {![regexp {^(?:" NS "::)?([1-9]+[0-9]*)$} $token _ id]} { \
            error {cannot parse token}\n\
        } \n\
        return $id \n\
    }");

    return TCL_OK;
}
