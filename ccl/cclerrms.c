/*
 * Europagate, 1995
 *
 * $Log: cclerrms.c,v $
 * Revision 1.1  1995-04-10 10:28:18  quinn
 * Added copy of CCL.
 *
 * Revision 1.6  1995/02/23  08:31:59  adam
 * Changed header.
 *
 * Revision 1.4  1995/02/14  16:20:54  adam
 * Qualifiers are read from a file now.
 *
 * Revision 1.3  1995/02/14  10:25:56  adam
 * The constructions 'qualifier rel term ...' implemented.
 *
 * Revision 1.2  1995/02/13  15:15:06  adam
 * Added handling of qualifiers. Not finished yet.
 *
 * Revision 1.1  1995/02/13  12:35:20  adam
 * First version of CCL. Qualifiers aren't handled yet.
 *
 */

char *err_msg_array[] = {
    "Ok",
    "Search word expected",
    "')' expected",
    "Set name expected",
    "Operator expected",
    "Unbalanced ')'",
    "Unknown qualifier",
    "Qualifiers applied twice",
    "'=' expected",
    "Bad relation",
    "Left truncation not supported",
    "Both left - and right truncation not supported",
    "Right truncation not supported"
};

const char *ccl_err_msg (int ccl_errno)
{
    return err_msg_array[ccl_errno];
}
