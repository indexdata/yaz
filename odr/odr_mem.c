/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 *
 * $Log: odr_mem.c,v $
 * Revision 1.17  2000-01-31 13:15:21  adam
 * Removed uses of assert(3). Cleanup of ODR. CCL parser update so
 * that some characters are not surrounded by spaces in resulting term.
 * ILL-code updates.
 *
 * Revision 1.16  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.15  1999/03/31 11:18:25  adam
 * Implemented odr_strdup. Added Reference ID to backend server API.
 *
 * Revision 1.14  1998/07/20 12:38:15  adam
 * More LOG_DEBUG-diagnostics.
 *
 * Revision 1.13  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.12  1995/11/08 17:41:33  quinn
 * Smallish.
 *
 * Revision 1.11  1995/11/01  13:54:43  quinn
 * Minor adjustments
 *
 * Revision 1.10  1995/10/25  16:58:19  quinn
 * Stupid bug in odr_malloc
 *
 * Revision 1.9  1995/10/13  16:08:08  quinn
 * Added OID utility
 *
 * Revision 1.8  1995/09/29  17:12:24  quinn
 * Smallish
 *
 * Revision 1.7  1995/09/27  15:02:59  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.6  1995/08/21  09:10:41  quinn
 * Smallish fixes to suppport new formats.
 *
 * Revision 1.5  1995/05/16  08:50:55  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.4  1995/05/15  11:56:09  quinn
 * More work on memory management.
 *
 * Revision 1.3  1995/04/18  08:15:21  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.2  1995/03/17  10:17:52  quinn
 * Added memory management.
 *
 * Revision 1.1  1995/03/14  10:27:40  quinn
 * Modified makefile to use common lib
 * Beginning to add memory management to odr
 *
 */

#include <stdlib.h>
#include <yaz/odr.h>
#include <yaz/xmalloc.h>

/* ------------------------ NIBBLE MEMORY ---------------------- */

/*
 * Extract the memory control block from o.
 */
NMEM odr_extract_mem(ODR o)
{
    NMEM r = o->mem;

    o->mem = 0;
    return r;
}

void *odr_malloc(ODR o, int size)
{
    if (o && !o->mem)
	o->mem = nmem_create();
    return nmem_malloc(o ? o->mem : 0, size);
}

char *odr_strdup(ODR o, const char *str)
{
    return nmem_strdup(o->mem, str);
}

int odr_total(ODR o)
{
    return o->mem ? nmem_total(o->mem) : 0;
}

/* ---------- memory management for data encoding ----------*/


int odr_grow_block(ODR b, int min_bytes)
{
    int togrow;

    if (!b->can_grow)
    	return -1;
    if (!b->size)
    	togrow = 1024;
    else
    	togrow = b->size;
    if (togrow < min_bytes)
    	togrow = min_bytes;
    if (b->size && !(b->buf =
		     (unsigned char *) xrealloc(b->buf, b->size += togrow)))
    	abort();
    else if (!b->size && !(b->buf = (unsigned char *)
			   xmalloc(b->size = togrow)))
    	abort();
#ifdef ODR_DEBUG
    fprintf(stderr, "New size for encode_buffer: %d\n", b->size);
#endif
    return 0;
}

int odr_write(ODR o, unsigned char *buf, int bytes)
{
    if (o->pos + bytes >= o->size && odr_grow_block(o, bytes))
    {
    	o->error = OSPACE;
	return -1;
    }
    memcpy(o->buf + o->pos, buf, bytes);
    o->pos += bytes;
    if (o->pos > o->top)
    	o->top = o->pos;
    return 0;
}

int odr_seek(ODR o, int whence, int offset)
{
    if (whence == ODR_S_CUR)
    	offset += o->pos;
    else if (whence == ODR_S_END)
    	offset += o->top;
    if (offset > o->size && odr_grow_block(o, offset - o->size))
    {
    	o->error = OSPACE;
	return -1;
    }
    o->pos = offset;
    return 0;
}
