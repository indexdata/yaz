/*
 * Copyright (c) 1995, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The names of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 */

/*
 * Biased-choice External for Z39.50.
 */

struct Z_External
{
    Odr_oid *direct_reference;
    int *indirect_reference;
    char *descriptor;
    enum
    {
	/* Generic types */
	Z_External_single = 0,
	Z_External_octet,
	Z_External_arbitrary,

	/* Specific types */
	Z_External_sutrs,
	Z_External_explainRecord,
	Z_External_resourceReport1,
	Z_External_resourceReport2,
	Z_External_promptObject1,
	Z_External_grs1
    } which;
    union
    {
	/* Generic types */
	Odr_any *single_ASN1_type;
	Odr_oct *octet_aligned;
	Odr_bitmask *arbitrary;

	/* Specific types */
	Z_SUTRS *sutrs;
	Z_ExplainRecord *explainRecord;
	Z_ResourceReport1 *resourceReport1;
	Z_ResourceReport2 *resourceReport2;
	Z_PromptObject1 *promptObject1;
	Z_GenericRecord *grs1;
    } u;
};

int z_External(ODR o, Z_External **p, int opt);
