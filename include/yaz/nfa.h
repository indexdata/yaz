/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 *  $Id: nfa.h,v 1.1 2006-05-03 09:04:33 heikki Exp $
 */

/**
 * \file nfa.h
 * \brief NFA for character set normalizing
 *
 * The NFA is a character mathcing device. It consists of states
 * and transitions between them. Each transition has a condition, which
 * is a range of values.
 *
 * When matching, we always start at the first state, and find the longest
 * possible sequence of input characters that match the ranges in the
 * conditions, and that leads into a terminal state. 
 *
 */

#ifndef NFA_H
#define NFA_H

#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

/** \brief Internal character type */
typedef unsigned int yaz_nfa_char; 


/** \brief The NFA itself 
 * The internals are hidden in nfa.c */
typedef struct yaz_nfa yaz_nfa;

/** \brief A state of the NFA  */
typedef struct yaz_nfa_state yaz_nfa_state;

/** \brief Transition from one state to another */
typedef struct yaz_nfa_transition yaz_nfa_transition;


/** \brief Initialize the NFA without any states in it  
 *
 * \return a pointer to the newly created NFA
 *
 * */
yaz_nfa *yaz_nfa_init();

/** \brief Destroy the whole thing */
void yaz_nfa_destroy(
          yaz_nfa *n  /** the nfa to destroy */
          );

/** \brief Add a normal state to the NFA.
 * 
 * The first state added will become the starting point.
 * Returns a pointer to it, which you can safely ignore, or use in building
 * transitions.
 */
yaz_nfa_state *yaz_nfa_add_state(
        yaz_nfa *n  /** The NFA to add the state to */
        );


/** \brief Sets the result pointer to a state 
 *
 *  Call with null to clear the pointer.
 * 
 *  \retval  0 ok
 *  \retval  1 The state already has a result!
 */
int yaz_nfa_set_result(
        /** The NFA itsef */
        yaz_nfa *n,       
        /** The state to which the result is added */
        yaz_nfa_state *s, 
        /** The result. The NFA does not care what it is, just stores it */
        void *result      
        );

/**  \brief Gets the result pointer from a state
 *
 *   \retval NULL if no result set
 */
void *yaz_nfa_get_result(
         yaz_nfa *n /** The NFA itself */, 
         yaz_nfa_state *s /** The state whose result you want */);

/** \brief Set the backref number to a state.
 * 
 *  Each state can be the beginning and/or ending of a backref
 *  sequence. This call sets those flags in the states. After matching,
 *  we can get hold of the backrefs that matched, and use them in our
 *  translations.
 *
 *  \param n   the nfa
 *  \param s   the state to add to
 *  \param backref_number is the number of the back reference. 0 for clearing
 *  \param is_start is 1 for start of the backref, 0 for end
 *  \return
 *     0 for OK
 *     1 if the backref is already set
 *     
 *     
 */

int yaz_nfa_set_backref(yaz_nfa *n, yaz_nfa_state *s,
        int backref_number,
        int is_start );

/** \brief Get the backref number of a state.
 * 
 *  \param n   the nfa
 *  \param s   the state to add to
 *  \param is_start is 1 for start of the backref, 0 for end
 *  \return the backref number associated with the state, or 0 if none.
 */

int yaz_nfa_get_backref(yaz_nfa *n, yaz_nfa_state *s,
        int is_start );

/**  \brief Add a transition to the NFA.
 * 
 *   Add a transition between two existing states. The condition
 *   is (as always) a range of yaz_nfa_chars. 
 *  \param n   the nfa
 *  \param from_state  which state the transition is from
 *  \param to_state    where the transition goes to
 *  \param range_start is the beginning of the range of values
 *  \param range_end is the end of the range of values
 **/
void yaz_nfa_add_transition(yaz_nfa *n, 
            yaz_nfa_state *from_state, 
            yaz_nfa_state *to_state,
            yaz_nfa_char range_start, 
            yaz_nfa_char range_end);

/** \brief Add an empty (epsilon) transition.
 *
 *  \param n   the nfa
 *  \param from_state  which state the transition is from
 *  \param to_state    where the transition goes to
 **/
void yaz_nfa_add_empty_transition( yaz_nfa *n,
                yaz_nfa_state *from_state, 
                yaz_nfa_state *to_state);

/** \brief Add a translation from a range to the NFA.
 * 
 *  \param n   the nfa
 *  \param st the state to add this to. If null, adds to the initial state
 *  \param range_start is the beginning of the range of values
 *  \param range_end is the end of the range of values
 *
 *  Checks if we already have a transition like this. If so, does not add 
 *  a new one, but returns the target state. Otherwise creates a new state,
 *  and a transition to it.
 */
yaz_nfa_state *yaz_nfa_add_range( yaz_nfa *n, 
          yaz_nfa_state *st,
          yaz_nfa_char range_start, 
          yaz_nfa_char range_end );
          
/** \brief Add a sequence of transitions and states.
 *  
 *  Starting from state s (or from the initial state, if s is
 *  null), finds as much of seq as possible and inserts the rest. 
 *  \Return the final state
 */
yaz_nfa_state *yaz_nfa_add_sequence( yaz_nfa *n,
              yaz_nfa_state *s,
              yaz_nfa_char *seq );


/** \brief Find the longest possible match.
 * 
 *  \param n   the nfa itself
 *  \param inbuff  buffer of input data. Will be incremented when match
 *  \param incharsleft   max number of inchars to use from inbuff
 *  \param result   the result pointer from the nfa (what ever that is)
 *
 *  In case of errors, returns the best match so far,
 *  which the caller is free to ignore.
 *
 *  \retval 0 success
 *  \retval 1 no match found
 *  \retval 2 overrun'of input buffer
 *  \retval 3 looping too far
 *
 */

int yaz_nfa_match(yaz_nfa *n, yaz_nfa_char **inbuff, size_t incharsleft,
                void **result );

#define YAZ_NFA_SUCCESS 0
#define YAZ_NFA_NOMATCH 1
#define YAZ_NFA_OVERRUN 2
#define YAZ_NFA_LOOP 3


/** \brief Get the first state of the NFA. 
 *
 *  \param n   the nfa
 *
 *  Useful for iterating through all states, probably calling get_result
 *  for each, and doing something to the results (freeing memory?)
 *
 *  \returns a pointer to the first state, or NULL if none.
 */
yaz_nfa_state *yaz_nfa_get_first(yaz_nfa *n);

/** \brief Get the next state of the NFA.
 *
 *  \param n   the nfa
 *  \param s   the state to add to
 *  \return the next state, or NULL if no more.
 */
yaz_nfa_state *yaz_nfa_get_next(yaz_nfa *n, yaz_nfa_state *s);

/** \brief Dump the NFA into a file .
 *
 *  \param F   The file handle to dump into (null => stdout)
 *  \param n   the nfa
 *  \param strfunc can be used for converting the resultinfo a string.
 *
 *  strfunc is a function like 
 *     char *f( void *result);
 *  it takes the result, and converts into a printable string (which 
 *  must be allocated somewhere by the caller). If the results are 
 *  already printable, passing a null pointer here prints them with a %s
 *
 */
void yaz_nfa_dump(FILE *F, yaz_nfa *n, char *(*strfunc)(void *) ); 


YAZ_END_CDECL

#endif
