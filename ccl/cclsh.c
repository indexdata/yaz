/*
 * Copyright (c) 1995, the EUROPAGATE consortium (see below).
 *
 * The EUROPAGATE consortium members are:
 *
 *    University College Dublin
 *    Danmarks Teknologiske Videnscenter
 *    An Chomhairle Leabharlanna
 *    Consejo Superior de Investigaciones Cientificas
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The names of EUROPAGATE or the project partners may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 3. Users of this software (implementors and gateway operators) agree to
 * inform the EUROPAGATE consortium of their use of the software. This
 * information will be used to evaluate the EUROPAGATE project and the
 * software, and to plan further developments. The consortium may use
 * the information in later publications.
 * 
 * 4. Users of this software agree to make their best efforts, when
 * documenting their use of the software, to acknowledge the EUROPAGATE
 * consortium, and the role played by the software in their work.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL THE EUROPAGATE CONSORTIUM OR ITS MEMBERS BE LIABLE
 * FOR ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF
 * ANY KIND, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND
 * ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* CCL shell.
 * Europagate 1995
 *
 * $Log: cclsh.c,v $
 * Revision 1.6  2000-01-31 13:15:21  adam
 * Removed uses of assert(3). Cleanup of ODR. CCL parser update so
 * that some characters are not surrounded by spaces in resulting term.
 * ILL-code updates.
 *
 * Revision 1.5  1999/12/16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 * Revision 1.4  1999/03/31 11:15:37  adam
 * Fixed memory leaks in ccl_find_str and ccl_qual_rm.
 *
 * Revision 1.3  1997/04/30 08:52:07  quinn
 * Null
 *
 * Revision 1.2  1996/10/11  15:00:25  adam
 * CCL parser from Europagate Email gateway 1.0.
 *
 * Revision 1.11  1995/05/16  09:39:27  adam
 * LICENSE.
 *
 * Revision 1.10  1995/05/11  14:03:57  adam
 * Changes in the reading of qualifier(s). New function: ccl_qual_fitem.
 * New variable ccl_case_sensitive, which controls whether reserved
 * words and field names are case sensitive or not.
 *
 * Revision 1.9  1995/02/23  08:32:00  adam
 * Changed header.
 *
 * Revision 1.7  1995/02/15  17:42:16  adam
 * Minor changes of the api of this module. FILE* argument added
 * to ccl_pr_tree.
 *
 * Revision 1.6  1995/02/14  19:55:13  adam
 * Header files ccl.h/cclp.h are gone! They have been merged an
 * moved to ../include/ccl.h.
 * Node kind(s) in ccl_rpn_node have changed names.
 *
 * Revision 1.5  1995/02/14  16:20:57  adam
 * Qualifiers are read from a file now.
 *
 * Revision 1.4  1995/02/14  14:12:42  adam
 * Ranges for ordered qualfiers implemented (e.g. pd=1980-1990).
 *
 * Revision 1.3  1995/02/14  10:25:57  adam
 * The constructions 'qualifier rel term ...' implemented.
 *
 * Revision 1.2  1995/02/13  15:15:07  adam
 * Added handling of qualifiers. Not finished yet.
 *
 * Revision 1.1  1995/02/13  12:35:21  adam
 * First version of CCL. Qualifiers aren't handled yet.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <yaz/ccl.h>

static int debug = 0;
static char *prog;

int main (int argc, char **argv)
{
    CCL_bibset bibset;
    FILE *bib_inf;
    char *bib_fname;

    prog = *argv;
    bibset = ccl_qual_mk ();    
    while (--argc > 0)
    {
        if (**++argv == '-')
        {
            switch (argv[0][1])
            {
            case 'd':
                debug = 1;
                break;
            case 'b':
                if (argv[0][2])
                    bib_fname = argv[0]+2;
                else if (argc > 0)
                {
                    --argc;
                    bib_fname = *++argv;
                }
                else
                {
                    fprintf (stderr, "%s: missing bib filename\n", prog);
                    exit (1);
                }
                bib_inf = fopen (bib_fname, "r");
                if (!bib_inf)
                {
                    fprintf (stderr, "%s: cannot open %s\n", prog,
                             bib_fname);
                    exit (1);
                }
                ccl_qual_file (bibset, bib_inf);
                fclose (bib_inf);
                break;
            default:
                fprintf (stderr, "%s: unknown option '%s'\n",
                    prog, *argv);
                exit (1);
            }
        }
        else
        {
            fprintf (stderr, "%s: no filenames, please\n", prog);
            exit (1);
        }
    }
    while (1)
    {
        char buf[80];
        int i, error, pos;
        struct ccl_rpn_node *rpn;

	printf ("CCLSH>"); fflush (stdout);
	if (!fgets (buf, 79, stdin))
	    break;
        for (i = 0; i<1; i++)
        {
	    rpn = ccl_find_str (bibset, buf, &error, &pos);
            if (error)
            {
                printf ("%*s^ - ", 6+pos, " ");
                printf ("%s\n", ccl_err_msg (error));
            }
            else
            {
		if (rpn && i == 0)
		{
		    ccl_pr_tree (rpn, stdout);
		    putchar ('\n');
		}
            }
            if (rpn)
                ccl_rpn_delete(rpn);
        }
    }
    putchar ('\n');
    return 0;
}
