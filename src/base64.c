/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file base64.c
 * \brief Base64 encode/decode utilities
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/base64.h>

void yaz_base64encode(const char *in, char *out)
{
    static char encoding[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char buf[3];
    long n;

    while (*in != 0)
    {
	const char *pad = 0;
        buf[0] = in[0];
	buf[1] = in[1];
	if (in[1] == 0)
        {
	    buf[2] = 0;
	    pad = "==";
	}
        else
        {
	    buf[2] = in[2];
	    if (in[2] == 0)
		pad = "=";
	}

	/* Treat three eight-bit numbers as on 24-bit number */
	n = (buf[0] << 16) + (buf[1] << 8) + buf[2];

	/* Write the six-bit chunks out as four encoded characters */
	*out++ = encoding[(n >> 18) & 63];
	*out++ = encoding[(n >> 12) & 63];
	if (in[1] != 0)
	    *out++ = encoding[(n >> 6) & 63];
	if (in[1] != 0 && in[2] != 0)
	    *out++ = encoding[n & 63];

	if (pad != 0) {
	    while (*pad != 0)
		*out++ = *pad++;
	    break;
	}
	in += 3;
    }
    *out++ = 0;
}

static int next_char(const char **in, size_t *len)
{
    const char *map = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz0123456789+/";
    const char *p;
    while (*len > 0 && strchr("\r\n\t\f ", **in))
    {
        (*len)--;
        (*in)++;
    }
    if (*len > 0 && **in == '=')
        return -2;
    if (*len > 0 && (p = strchr(map, **in)))
    {
        (*len)--;
        (*in)++;
        return p - map;
    }
    return -1;
}

int yaz_base64decode(const char *in, char *out)
{
    size_t len = strlen(in);

    while (len >= 4)
    {
	int i0, i1, i2, i3;

	i0 = next_char(&in, &len);
        if (i0 < 0)
            return -1;
	i1 = next_char(&in, &len);
        if (i1 < 0)
            return -1;
	*(out++) = i0 << 2 | i1 >> 4;
        i2 = next_char(&in, &len);
        if (i2 == -2)
            break;
        if (i2 == -1)
            return -1;
	*(out++) = i1 << 4 | i2 >> 2;
        i3 = next_char(&in, &len);
        if (i3 == -2)
            break;
        if (i3 == -1)
            return -1;
	*(out++) = i2 << 6 | i3;
    }
    *out = '\0';
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

