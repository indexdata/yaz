/*
 * $Id: zoom-opt.c,v 1.1 2001-10-23 21:00:20 adam Exp $
 *
 * ZOOM layer for C, options handling
 */
#include <assert.h>
#include <yaz/xmalloc.h>
#include <yaz/log.h>

#include "zoom-p.h"

Z3950_options Z3950_options_create (void)
{
    return Z3950_options_create_with_parent (0);
}

Z3950_options Z3950_options_create_with_parent (Z3950_options parent)
{
    Z3950_options opt = xmalloc (sizeof(*opt));

    opt->refcount = 1;
    opt->callback_func = 0;
    opt->callback_handle = 0;
    opt->entries = 0;
    opt->parent= parent;
    if (parent)
	(parent->refcount)++;
    return opt;
}

void Z3950_options_addref (Z3950_options opt)
{
    (opt->refcount)++;
}

Z3950_options_callback Z3950_options_set_callback (
    Z3950_options opt,
    Z3950_options_callback callback_func,
    void *callback_handle)
{
    Z3950_options_callback callback_old;

    assert (opt);
    callback_old = opt->callback_func;
    opt->callback_func = callback_func;
    opt->callback_handle = callback_handle;
    return callback_old;
}

void Z3950_options_destroy (Z3950_options opt)
{
    if (!opt)
	return;
    (opt->refcount)--;
    if (opt->refcount == 0)
    {
	struct Z3950_options_entry *e;
	
	Z3950_options_destroy (opt->parent);
	e = opt->entries;
	while (e)
	{
	    struct Z3950_options_entry *e0 = e;
	    xfree (e->name);
	    xfree (e->value);
	    e = e->next;
	    xfree (e0);
	}
	xfree (opt);
    }
}

void Z3950_options_set (Z3950_options opt, const char *name, const char *value)
{
    struct Z3950_options_entry **e;

    e = &opt->entries;
    while (*e)
    {
	if (!strcmp((*e)->name, name))
	{
	    xfree ((*e)->value);
	    (*e)->value = xstrdup(value);
	    return;
	}
	e = &(*e)->next;
    }
    *e = xmalloc (sizeof(**e));
    (*e)->name = xstrdup (name);
    (*e)->value = xstrdup (value);
    (*e)->next = 0;
}

const char *Z3950_options_get (Z3950_options opt, const char *name)
{
    const char *v = 0;
    if (!opt)
	return 0;
    if (opt->callback_func)
	v = (*opt->callback_func)(opt->callback_handle, name);
    if (!v)
    {
	struct Z3950_options_entry *e;
	for (e = opt->entries; e; e = e->next)
	    if (!strcmp(e->name, name))
	    {
		v = e->value;
		break;
	    }
    }
    if (!v)
	return Z3950_options_get(opt->parent, name);
    return v;
}

int Z3950_options_get_bool (Z3950_options opt, const char *name, int defa)
{
    const char *v = Z3950_options_get (opt, name);

    if (!v)
	return defa;
    if (!strcmp (v, "1") || !strcmp(v, "T"))
	return 1;
    return 0;
}

int Z3950_options_get_int (Z3950_options opt, const char *name, int defa)
{
    const char *v = Z3950_options_get (opt, name);

    if (!v || !*v)
	return defa;
    return atoi(v);
}
