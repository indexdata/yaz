/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 *
 *  $Id: nfatest1.c,v 1.1 2006-05-03 09:04:33 heikki Exp $
 *
 */


#include <stdio.h>
#include <string.h>
#include <yaz/nfa.h>
#include <yaz/nmem.h>
#include <yaz/test.h>

#define VERBOSE 1

char *printfunc(void *result) {
    static char buf[200];
    sprintf(buf,"\"%s\"", (char*) result);
    return buf;
}

void test_match(yaz_nfa *n, 
        yaz_nfa_char *buf, int buflen, 
        int expcode, char *expstr) {
    yaz_nfa_char *c=buf;
    void *resptr=0;
    int i;
    i=yaz_nfa_match(n,&c,buflen,&resptr);
#if VERBOSE    
    printf("\n'%s' returned %d. Moved c by %d, and resulted in '%s'\n",
            expstr, i, (c-buf),(char*)resptr);
#endif
    YAZ_CHECK_EQ(i,expcode);
    YAZ_CHECK_EQ(strcmp(expstr,(char*)resptr),0);
}

void construction_test() {
    yaz_nfa* n= yaz_nfa_init();
    yaz_nfa_state *s,*s0,*s1,*s2,*s3,*s4,*s5;
    int i;
    yaz_nfa_char seq1[]={'p','r','e','f','i','x',0};
    yaz_nfa_char seq2[]={'p','r','e','l','i','m',0};
    yaz_nfa_char tst1[]={'c','0'};
    yaz_nfa_char tst2[]={'c','k','0'};
    yaz_nfa_char tst3[]={'c','x','0'};
    yaz_nfa_char tst4[]={'z','k','0'};
    yaz_nfa_char tst5[]={'y','k','k','k','k','k','k','y','0'};
    void *p;

    YAZ_CHECK(n);

    s=yaz_nfa_get_first(n);
    YAZ_CHECK(!s);

    s0=yaz_nfa_add_state(n);

    s=yaz_nfa_get_first(n);
    YAZ_CHECK(s);
    s=yaz_nfa_get_next(n,s);
    YAZ_CHECK(!s);

    s1=yaz_nfa_add_state(n);
    i=yaz_nfa_set_result(n,s1,"first");
    YAZ_CHECK_EQ(i,0);

    i=yaz_nfa_set_result(n,s1,"DUPLICATE");
    YAZ_CHECK_EQ(i,1);

    p=yaz_nfa_get_result(n,s1);
    YAZ_CHECK(p);
    YAZ_CHECK( strcmp((char*)p,"first")==0 );

    i=yaz_nfa_set_result(n,s1,0);
    YAZ_CHECK_EQ(i,0);
    p=yaz_nfa_get_result(n,s1);
    YAZ_CHECK(!p);
    i=yaz_nfa_set_result(n,s1,"first");
    YAZ_CHECK_EQ(i,0);
    
    s2=yaz_nfa_add_state(n);
    s3=yaz_nfa_add_state(n);
    yaz_nfa_set_result(n,s3,"a-k,x-z");

    s=yaz_nfa_get_first(n);
    YAZ_CHECK(s);
    s=yaz_nfa_get_next(n,s);
    YAZ_CHECK(s);

    
    yaz_nfa_add_transition(n,s0,s1,'a','k');
    yaz_nfa_add_transition(n,s1,s1,'k','k');
    yaz_nfa_add_transition(n,s0,s2,'p','p');
    yaz_nfa_add_transition(n,s1,s3,'x','z');

    s=yaz_nfa_add_range(n, 0, 'k','s' );
    yaz_nfa_set_result(n,s,"K-S");

    s4=yaz_nfa_add_range(n, s2, 'l','r' );
    s5=yaz_nfa_add_range(n, s2, 'l','r' );
    YAZ_CHECK((s4==s5));
    s=yaz_nfa_add_range(n, 0, 'c','c' );

    s=yaz_nfa_add_range(n, 0, 'z','z' );
    yaz_nfa_add_empty_transition(n,s,s);
    yaz_nfa_set_result(n,s,"loop");

    s=yaz_nfa_add_range(n, 0, 'y','y' );
    s1=yaz_nfa_add_state(n);
    yaz_nfa_set_backref(n,s1,1,1);
    yaz_nfa_add_empty_transition(n,s,s1);
    s=s1;
    yaz_nfa_add_transition(n,s,s,'k','k');
    s=yaz_nfa_add_range(n, s, 'y','y' );
    yaz_nfa_set_result(n,s,"y k+ y");
    yaz_nfa_set_backref(n,s,1,0);

    s=yaz_nfa_add_sequence(n, 0, seq1 ); 
    yaz_nfa_set_result(n,s,"PREFIX");
    s=yaz_nfa_add_sequence(n, 0, seq2 ); 
    yaz_nfa_set_result(n,s,"PRELIM");

#if VERBOSE    
    yaz_nfa_dump(0,n, printfunc);
#endif

    test_match(n,seq2,3,YAZ_NFA_OVERRUN,"K-S");
    test_match(n,seq2,6,YAZ_NFA_SUCCESS,"PRELIM");
    test_match(n,tst1,3,YAZ_NFA_SUCCESS,"first");
    test_match(n,tst2,3,YAZ_NFA_SUCCESS,"first");
    test_match(n,tst3,3,YAZ_NFA_SUCCESS,"a-k,x-z");
    test_match(n,tst4,9,YAZ_NFA_LOOP,"loop");
    test_match(n,tst5,9,YAZ_NFA_SUCCESS,"y k+ y");

    yaz_nfa_destroy(n);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    nmem_init ();
    construction_test();
    nmem_exit ();
    YAZ_CHECK_TERM;
}

