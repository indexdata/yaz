/* $Id: lexer.c,v 1.2 2003-04-11 15:53:39 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

static void putb(YYSTYPE *lval, CQL_parser cp, int c)
{
    if (lval->len >= lval->size)
    {
        char *nb = nmem_malloc(cp->nmem, (lval->size = lval->len * 2 + 20));
        memcpy (nb, lval->buf, lval->len);
        lval->buf = nb;
    }
    if (c)
        lval->buf[lval->len++] = c;
    lval->buf[lval->len] = '\0';
}

/*
 * bison lexer for CQL.
 */

int yylex(YYSTYPE *lval, void *vp)
{
    CQL_parser cp = (CQL_parser) vp;
    int c;
    do
    {
        c = cp->getbyte(cp->client_data);
        if (c == 0)
            return 0;
        if (c == '\n')
            return 0;
    } while (isspace(c));
    lval->rel = 0;
    lval->len = 0;
    lval->size = 10;
    lval->buf = nmem_malloc(cp->nmem, lval->size);
    if (strchr("()=></", c))
    {
        int c1;
        putb(lval, cp, c);
        if (c == '>')
        {
            c1 = cp->getbyte(cp->client_data);
            if (c1 == '=')
            {
                putb(lval, cp, c1);
                return GE;
            }
            else
                cp->ungetbyte(c1, cp->client_data);
        }
        else if (c == '<')
        {
            c1 = cp->getbyte(cp->client_data);
            if (c1 == '=')
            {
                putb(lval, cp, c1);
                return LE;
            }
            else if (c1 == '>')
            {
                putb(lval, cp, c1);
                return NE;
            }
            else
                cp->ungetbyte(c1, cp->client_data);
        }
        return c;
    }
    if (c == '"')
    {
        while ((c = cp->getbyte(cp->client_data)) != EOF && c != '"')
        {
            if (c == '\\')
                c = cp->getbyte(cp->client_data);
            putb(lval, cp, c);
        }
        putb(lval, cp, 0);
    }
    else
    {
        putb(lval, cp, c);
        while ((c = cp->getbyte(cp->client_data)) != 0 &&
               !strchr(" \n()=<>/", c))
        {
            if (c == '\\')
                c = cp->getbyte(cp->client_data);
            putb(lval, cp, c);
        }
#if YYDEBUG
        printf ("got %s\n", lval->buf);
#endif
        if (c != 0)
            cp->ungetbyte(c, cp->client_data);
        if (!strcmp(lval->buf, "and"))
            return AND;
        if (!strcmp(lval->buf, "or"))
            return OR;
        if (!strcmp(lval->buf, "not"))
            return NOT;
        if (!strcmp(lval->buf, "exact"))
            return EXACT;
        if (!strcmp(lval->buf, "all"))
            return ALL;
        if (!strncmp(lval->buf, "prox", 4))
            return PROX;
        if (!strcmp(lval->buf, "any"))
            return ANY;
        if (!strcmp(lval->buf, "scr"))
            return SCR;
    }
    return TERM;
}
