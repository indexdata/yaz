/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 *
 *  $Id: nfatest1.c,v 1.6 2006-05-10 13:58:47 heikki Exp $
 *
 */


#include <stdio.h>
#include <string.h>
#include <yaz/nfa.h>
#include <yaz/nmem.h>
#include <yaz/test.h>


char *printfunc(void *result) {
    static char buf[200];
    sprintf(buf, "\"%s\"", (char*) result);
    return buf;
}

char *printfunc2(void *result) {
    static char buf[200];
    sprintf(buf,"(%p)",  result);
    return buf;
}

void test_match(yaz_nfa *n, 
        yaz_nfa_char *buf, size_t buflen, 
        int expcode, char *expstr) {
    yaz_nfa_char *c = buf;
    yaz_nfa_char *cp1, *cp2;
    void *resptr = 0;
    int i, bi;
    size_t buflen2 = buflen;
    i = yaz_nfa_match(n,&c, &buflen2,&resptr);
    if (yaz_test_get_verbosity()>3)
        printf("\n'%s' returned %d. Moved c by %d, and resulted in '%s'\n",
            expstr, i, (c-buf),(char*)resptr);
    YAZ_CHECK_EQ(buflen-buflen2, c-buf);
    YAZ_CHECK_EQ(i, expcode);
    if (i==0)
        YAZ_CHECK_EQ(strcmp(expstr,(char*)resptr), 0);
    i = 0;
    bi = 0;
    while((bi!=2) && (yaz_test_get_verbosity()>3)){
        bi = yaz_nfa_get_backref(n, i,&cp1,&cp2);
        if (bi==0 && ( cp1 || cp2 ) ) {
            printf("  got backref %d of %d chars (%p to %p): '",
                    i, cp2-cp1+1, cp1, cp2);
            while (cp2-cp1 >= 0 )
                printf("%c", *cp1++);
            printf("'\n");
        }
        i++;
    }
}

void construction_test() {
    yaz_nfa* n= yaz_nfa_init();
    yaz_nfa_char *cp, *cp1, *cp2;
    yaz_nfa_state *s, *s0, *s1, *s2, *s3, *s4, *s5;
    int i;
    yaz_nfa_char seq1[]={'p', 'r', 'e', 'f', 'i', 'x', 0};
    yaz_nfa_char seq2[]={'p', 'r', 'e', 'l', 'i', 'm', 0};
    yaz_nfa_char tst1[]={'c', 0};
    yaz_nfa_char tst2[]={'c', 'k', 0};
    yaz_nfa_char tst3[]={'c', 'x', 0};
    yaz_nfa_char tst4[]={'z', 'k', 0};
    yaz_nfa_char tst5[]={'y', 'k', 'l', 'k', 'k', 'l', 'k', 'd', 0};
    yaz_nfa_char tst6[]={'x', 'z', 'k', 'a', 'b', 0};
    void *p;
    size_t sz;

    YAZ_CHECK(n);

    s = yaz_nfa_get_first(n);
    YAZ_CHECK(!s);

    s0 = yaz_nfa_add_state(n);

    s = yaz_nfa_get_first(n);
    YAZ_CHECK(s);
    s = yaz_nfa_get_next(n, s);
    YAZ_CHECK(!s);

    s1 = yaz_nfa_add_state(n);
    i = yaz_nfa_set_result(n, s1, "first");
    YAZ_CHECK_EQ(i, 0);

    i = yaz_nfa_set_result(n, s1, "DUPLICATE");
    YAZ_CHECK_EQ(i, YAZ_NFA_ALREADY);

    p = yaz_nfa_get_result(n, s1);
    YAZ_CHECK(p);
    YAZ_CHECK( strcmp((char*)p, "first")==0 );

    i = yaz_nfa_set_result(n, s1, 0);
    YAZ_CHECK_EQ(i, 0);
    p = yaz_nfa_get_result(n, s1);
    YAZ_CHECK(!p);
    i = yaz_nfa_set_result(n, s1, "first");
    YAZ_CHECK_EQ(i, 0);
    
    s2 = yaz_nfa_add_state(n);
    s3 = yaz_nfa_add_state(n);
    yaz_nfa_set_result(n, s3, "a-k, x-z");

    s = yaz_nfa_get_first(n);
    YAZ_CHECK(s);
    s = yaz_nfa_get_next(n, s);
    YAZ_CHECK(s);

    
    yaz_nfa_add_transition(n, s0, s1, 'a', 'k');
    yaz_nfa_add_transition(n, s1, s1, 'k', 'k');
    yaz_nfa_add_transition(n, s0, s2, 'p', 'p');
    yaz_nfa_add_transition(n, s1, s3, 'x', 'z');

    s = yaz_nfa_add_range(n, 0, 'k', 's' );
    yaz_nfa_set_result(n, s, "K-S");

    s4 = yaz_nfa_add_range(n, s2, 'l', 'r' );
    s5 = yaz_nfa_add_range(n, s2, 'l', 'r' );
    YAZ_CHECK((s4==s5));
    s = yaz_nfa_add_range(n, 0, 'c', 'c' );

    s = yaz_nfa_add_range(n, 0, 'z', 'z' );
    yaz_nfa_add_empty_transition(n, s, s);
    yaz_nfa_set_result(n, s, "loop");

    s = yaz_nfa_add_range(n, 0, 'y', 'y' );
    yaz_nfa_set_backref_point(n, s, 1, 1);
    s1 = yaz_nfa_add_state(n);
    yaz_nfa_add_empty_transition(n, s, s1);
    s = s1;
    yaz_nfa_add_transition(n, s, s, 'k', 'l');
    s = yaz_nfa_add_range(n, s, 'd', 'd' );
    yaz_nfa_set_result(n, s, "y k+ d");
    yaz_nfa_set_backref_point(n, s, 1, 0);

    s = yaz_nfa_add_sequence(n, 0, seq1,6 ); 
    yaz_nfa_set_result(n, s, "PREFIX");
    s = yaz_nfa_add_sequence(n, 0, seq2,6 ); 
    yaz_nfa_set_result(n, s, "PRELIM");

    s = yaz_nfa_add_range(n, 0, 'x', 'x' );
    i=yaz_nfa_set_backref_point(n, s, 2, 0);
    YAZ_CHECK_EQ(i,YAZ_NFA_NOSTART);
    i=yaz_nfa_set_backref_point(n, s, 2, 1);
    YAZ_CHECK_EQ(i,YAZ_NFA_SUCCESS);
    i=yaz_nfa_set_backref_point(n, s, 2, 1);
    YAZ_CHECK_EQ(i,YAZ_NFA_ALREADY);
    s1 = yaz_nfa_add_sequence(n, s, tst4,2);
    yaz_nfa_set_backref_point(n, s1, 2, 0);
    yaz_nfa_set_result(n, s1, "xzk");

    /* check return codes before doing any matches */
    i = yaz_nfa_get_backref(n, 0, &cp1, &cp2 );
    YAZ_CHECK_EQ(i, YAZ_NFA_NOMATCH);
    i = yaz_nfa_get_backref(n, 3, &cp1, &cp2 );
    YAZ_CHECK_EQ(i, YAZ_NFA_NOSUCHBACKREF );
    i = yaz_nfa_get_backref(n, 1, &cp1, &cp2 );
    YAZ_CHECK_EQ(i, YAZ_NFA_NOMATCH );

    
    if (yaz_test_get_verbosity()>3)
        yaz_nfa_dump(0, n, printfunc);

    test_match(n, seq2, 3, YAZ_NFA_OVERRUN, "K-S");
    test_match(n, seq2, 6, YAZ_NFA_SUCCESS, "PRELIM");
    test_match(n, tst1, 3, YAZ_NFA_SUCCESS, "first");
    test_match(n, tst2, 3, YAZ_NFA_SUCCESS, "first");
    test_match(n, tst3, 3, YAZ_NFA_SUCCESS, "a-k, x-z");
    test_match(n, tst4, 9, YAZ_NFA_LOOP, "loop");
    test_match(n, tst5, 9, YAZ_NFA_SUCCESS, "y k+ d");

    cp = tst6;  /* xzkab */
    sz = 8;
    i = yaz_nfa_match(n, &cp, &sz, &p);
    YAZ_CHECK_EQ(i, YAZ_NFA_SUCCESS); 
    i = yaz_nfa_get_backref(n, 2, &cp1, &cp2 );
    YAZ_CHECK_EQ(i, 0);
    YAZ_CHECK_EQ(cp2-cp1+1,2); 
    YAZ_CHECK_EQ(*cp1, 'z' );
    YAZ_CHECK_EQ(*cp2, 'k' );
    if (yaz_test_get_verbosity()>3)
        printf("backref from %p '%c' to %p '%c' is %d long. sz is now %d\n",
            cp1, *cp1,  cp2, *cp2,  cp2-cp1+1, sz );

    yaz_nfa_destroy(n);
}

void converter_test() {
    yaz_nfa* n= yaz_nfa_init();
    yaz_nfa_converter *c1, *c2, *c3;
    yaz_nfa_char str1[]={'a','b','c'};
    yaz_nfa_char seq1[]={'A','B','C'};
    yaz_nfa_char seq2[]={'k','m','n','m','x','P','Q','X',0};
    yaz_nfa_char outbuf[1024];
    yaz_nfa_char *outp, *cp, *cp1, *cp2;
    yaz_nfa_state *s, *s2;
    void *vp;
    int i;
    size_t sz;

    c1=yaz_nfa_create_string_converter(n,str1,3);

    for(i=0;i<1024;i++)
        outbuf[i]=10000+i;
    outp=outbuf;
    sz=1;
    i=yaz_nfa_run_converters(n, c1, &outp, &sz);
    YAZ_CHECK_EQ(i,4); /* overrun */
    YAZ_CHECK_EQ(outbuf[0],'a');
    YAZ_CHECK_EQ(outbuf[1],10000+1);

    for(i=0;i<1024;i++)
        outbuf[i]=10000+i;
    outp=outbuf;
    sz=3;
    i=yaz_nfa_run_converters(n, c1, &outp, &sz);
    YAZ_CHECK_EQ(i,0); 
    YAZ_CHECK_EQ(outbuf[0],'a');
    YAZ_CHECK_EQ(outbuf[1],'b');
    YAZ_CHECK_EQ(outbuf[2],'c');
    YAZ_CHECK_EQ(outbuf[3],10000+3);
    YAZ_CHECK_EQ(sz,0);
    
    c2=yaz_nfa_create_string_converter(n,str1,2);
    yaz_nfa_append_converter(n,c1,c2);

    for(i=0;i<1024;i++)
        outbuf[i]=10000+i;
    outp=outbuf;
    sz=10;
    i=yaz_nfa_run_converters(n, c1, &outp, &sz);
    YAZ_CHECK_EQ(i,0); 
    YAZ_CHECK_EQ(outbuf[0],'a');
    YAZ_CHECK_EQ(outbuf[1],'b');
    YAZ_CHECK_EQ(outbuf[2],'c');
    YAZ_CHECK_EQ(outbuf[3],'a');
    YAZ_CHECK_EQ(outbuf[4],'b');
    YAZ_CHECK_EQ(outbuf[5],10000+5);
    YAZ_CHECK_EQ(sz,5);
    
    /* ABC -> abcab */
    (void) yaz_nfa_add_state(n);/* start state */
    s=yaz_nfa_add_state(n);
    yaz_nfa_add_empty_transition(n,0,s);
    yaz_nfa_set_backref_point(n,s,1,1);
    s=yaz_nfa_add_sequence(n, s, seq1,3 ); 
    yaz_nfa_set_result(n,s,c1);
    yaz_nfa_set_backref_point(n,s,1,0);

    /* ([k-o][m-n]*)x -> \1 */
    s=yaz_nfa_add_state(n);
    yaz_nfa_add_empty_transition(n,0,s);
    yaz_nfa_set_backref_point(n,s,2,1);
    s2=yaz_nfa_add_state(n);
    yaz_nfa_add_transition(n,s,s2,'k','o');
    yaz_nfa_add_transition(n,s2,s2,'m','n');
    s=yaz_nfa_add_state(n);
    yaz_nfa_add_transition(n,s2,s,'x','x');
    yaz_nfa_set_backref_point(n,s,2,0);

    c1=yaz_nfa_create_backref_converter(n,2);
    yaz_nfa_set_result(n,s,c1);

    if (yaz_test_get_verbosity()>3)
        yaz_nfa_dump(0,n, printfunc2);

    cp=seq2;
    sz=18;
    i=yaz_nfa_match(n,&cp,&sz,&vp);
    c2=vp;
    YAZ_CHECK_EQ(i,YAZ_NFA_SUCCESS); 
    i=yaz_nfa_get_backref(n, 2, &cp1, &cp2 );
    if (yaz_test_get_verbosity()>3)
        printf("backref from %p '%c' to %p '%c' is %d long. sz is now %d\n",
            cp1, *cp1,  cp2, *cp2,  cp2-cp1+1, sz );
    YAZ_CHECK_EQ(i,0);
    YAZ_CHECK_EQ((int)c1,(int)c2);  /* got our pointer back from nfa */
    for(i=0;i<1024;i++)
        outbuf[i]=10000+i;
    outp=outbuf;
    sz=11;
    i=yaz_nfa_run_converters(n, c2, &outp, &sz);
    YAZ_CHECK_EQ(i,0); 
    YAZ_CHECK_EQ(outbuf[0],'k');
    YAZ_CHECK_EQ(outbuf[1],'m');
    YAZ_CHECK_EQ(outbuf[2],'n');
    YAZ_CHECK_EQ(outbuf[3],'m');
    YAZ_CHECK_EQ(outbuf[4],'x');
    YAZ_CHECK_EQ(outbuf[5],10000+5);
    YAZ_CHECK_EQ(sz,11-5);

    c3=yaz_nfa_create_range_converter(n,2, 'a', 'A' );
    for(i=0;i<1024;i++)
        outbuf[i]=10000+i;
    outp=outbuf;
    sz=11;
    i=yaz_nfa_run_converters(n, c3, &outp, &sz);
    YAZ_CHECK_EQ(i,0); 
    YAZ_CHECK_EQ(outbuf[0],'K');
    YAZ_CHECK_EQ(outbuf[1],'M');
    YAZ_CHECK_EQ(outbuf[2],'N');
    YAZ_CHECK_EQ(outbuf[3],'M');
    YAZ_CHECK_EQ(outbuf[4],'X');
    YAZ_CHECK_EQ(outbuf[5],10000+5);
    YAZ_CHECK_EQ(sz,11-5);

    yaz_nfa_destroy(n);
}

yaz_nfa_char *makebuff(NMEM nmem, char *in) {
    yaz_nfa_char *buff = nmem_malloc(nmem, strlen(in)*sizeof(yaz_nfa_char));
    yaz_nfa_char *op=buff;
    while ( (*op++ = *in++) )
        ;
    return buff;
}

void dumpbuff(char *msg, yaz_nfa_char *start, yaz_nfa_char *end) {
    if (yaz_test_get_verbosity()>3) {
        printf("%s\"",msg);
        while (start!=end)
            printf("%c",*start++);
        printf("\"\n");
    }
}

void chkbuff( yaz_nfa_char *start, yaz_nfa_char *end, char *exp) {
    char *orig_exp=exp;
    while (start!=end)
        if ( *start++ != *exp++ ) {
            if (yaz_test_get_verbosity()>3) {
                start--;
                exp--;
                printf ("chkbuff: unexpected conversion '%c' != '%c' \n"
                        "\"%s\"\n", *start, *exp, orig_exp );
            }
            YAZ_CHECK(!"conversion differs! ");
            return;
        }

}

void high_level_test() {
    NMEM nmem=nmem_create();
    yaz_nfa_char from1[] = {'f','o','o','b','a','r'};
    yaz_nfa_char to1[] = {'f','u','b','a','r'};
    yaz_nfa_char tospace[] = {' '};
    yaz_nfa_char todot[] = {'.'};
    char *fromtext =
            "It was a Dark and Rainy Night, when alpha and beta "
            "fixme - FIND better names ?? !! ##  - "
            "went out to fix the foobar "
            "that was all foo.";
    char *expected = 
            "IT WAS A DARK AND RAINY NIGHT. WHEN ALPHA AND b "
            "to-be-fixed-later . FIND BETTER NAMES .. .. ..  . "
            "WENT OUT TO (fix) THE fubar "
            "THAT WAS ALL FOO.";
    yaz_nfa_char *from3 = makebuff(nmem,fromtext);
    yaz_nfa_char *to3 = nmem_malloc(nmem, 1024*sizeof(yaz_nfa_char));
    yaz_nfa_char *fromp=from3;
    yaz_nfa_char *top=to3;
    size_t insize=strlen(fromtext);
    size_t outsize=1024;
    size_t prev_insize=0;

    yaz_nfa *n = yaz_nfa_init();
    int i;
    i = yaz_nfa_add_string_rule(n, from1, 6, to1, 5);
    YAZ_CHECK_EQ(i,0);
    i = yaz_nfa_add_string_rule(n, from1, 6, to1, 5);
    YAZ_CHECK_EQ(i,YAZ_NFA_ALREADY);
    i = yaz_nfa_add_ascii_string_rule(n,"beta","b");
    YAZ_CHECK_EQ(i,0);
    i = yaz_nfa_add_ascii_string_rule(n,"fixme","to-be-fixed-later");
    YAZ_CHECK_EQ(i,0);
    i = yaz_nfa_add_ascii_string_rule(n,"fix","(fix)");
    YAZ_CHECK_EQ(i,0);
    i = yaz_nfa_add_char_range_rule(n, 'a','z','A');
    YAZ_CHECK_EQ(i,0);
    i = yaz_nfa_add_char_string_rule(n, 0,' ', tospace,1);
    YAZ_CHECK_EQ(i,0);
    i = yaz_nfa_add_char_string_rule(n, '!','/', todot,1);
    YAZ_CHECK_EQ(i,0);
    i = yaz_nfa_add_char_string_rule(n, ':','?', todot,1);
    YAZ_CHECK_EQ(i,0);
    if (yaz_test_get_verbosity()>3)
        yaz_nfa_dump(0,n, printfunc2);

    YAZ_CHECK_EQ( *from3, 'I' ); /* just to be sure my copy func works */
    for (i=0;i<100;i++)
        to3[i]=10000+i;
    i=yaz_nfa_convert_slice(n, &fromp, &insize, &top, &outsize);
    YAZ_CHECK_EQ(i,YAZ_NFA_SUCCESS);
    YAZ_CHECK_EQ(*to3,'I');
    YAZ_CHECK_EQ(insize, strlen(fromtext)-1);
    YAZ_CHECK_EQ(outsize, 1024-1);

    while ( (i==YAZ_NFA_SUCCESS) && (insize > 0) && (prev_insize!=insize) ) {
        prev_insize=insize; /* detect dead loops if something goes wrong */
        i=yaz_nfa_convert_slice(n, &fromp, &insize, &top, &outsize);
    }
    YAZ_CHECK_EQ(i,YAZ_NFA_SUCCESS);
    YAZ_CHECK_EQ(insize,0);
    YAZ_CHECK(prev_insize != insize); /* the loop would have been endless */

    dumpbuff("Original text: ",from3, fromp);
    dumpbuff("Converted text: ",to3, top);

    chkbuff(to3, top, expected);

    yaz_nfa_destroy(n);
    nmem_destroy(nmem);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    nmem_init ();
    construction_test(); 
    converter_test();
    high_level_test();
    nmem_exit ();
    YAZ_CHECK_TERM;
}


/* 
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
