/*
 * $Id: zoom-opt.c,v 1.2 2001-11-18 21:14:23 adam Exp $
 *
 * ZOOM layer for C, options handling
 */
#include <assert.h>
#include <yaz/xmalloc.h>
#include <yaz/log.h>

#include "zoom-p.h"

ZOOM_options ZOOM_options_create (void)
{
    return ZOOM_options_create_with_parent (0);
}

ZOOM_options ZOOM_options_create_with_parent (ZOOM_options parent)
{
    ZOOM_options opt = xmalloc (sizeof(*opt));

    opt->refcount = 1;
    opt->callback_func = 0;
    opt->callback_handle = 0;
    opt->entries = 0;
    opt->parent= parent;
    if (parent)
	(parent->refcount)++;
    return opt;
}

void ZOOM_options_addref (ZOOM_options opt)
{
    (opt->refcount)++;
}

ZOOM_options_callback ZOOM_options_set_callback (
    ZOOM_options opt,
    ZOOM_options_callback callback_func,
    void *callback_handle)
{
    ZOOM_options_callback callback_old;

    assert (opt);
    callback_old = opt->callback_func;
    opt->callback_func = callback_func;
    opt->callback_handle = callback_handle;
    return callback_old;
}

void ZOOM_options_destroy (ZOOM_options opt)
{
    if (!opt)
	return;
    (opt->refcount)--;
    if (opt->refcount == 0)
    {
	struct ZOOM_options_entry *e;
	
	ZOOM_options_destroy (opt->parent);
	e = opt->entries;
	while (e)
	{
	    struct ZOOM_options_entry *e0 = e;
	    xfree (e->name);
	    xfree (e->value);
	    e = e->next;
	    xfree (e0);
	}
	xfree (opt);
    }
}

void ZOOM_options_set (ZOOM_options opt, const char *name, const char *value)
{
    struct ZOOM_options_entry **e;

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

const char *ZOOM_options_get (ZOOM_options opt, const char *name)
{
    const char *v = 0;
    if (!opt)
	return 0;
    if (opt->callback_func)
	v = (*opt->callback_func)(opt->callback_handle, name);
    if (!v)
    {
	struct ZOOM_options_entry *e;
	for (e = opt->entries; e; e = e->next)
	    if (!strcmp(e->name, name))
	    {
		v = e->value;
		break;
	    }
    }
    if (!v)
	return ZOOM_options_get(opt->parent, name);
    return v;
}

int ZOOM_options_get_bool (ZOOM_options opt, const char *name, int defa)
{
    const char *v = ZOOM_options_get (opt, name);

    if (!v)
	return defa;
    if (!strcmp (v, "1") || !strcmp(v, "T"))
	return 1;
    return 0;
}

int ZOOM_options_get_int (ZOOM_options opt, const char *name, int defa)
{
    const char *v = ZOOM_options_get (opt, name);

    if (!v || !*v)
	return defa;
    return atoi(v);
}
