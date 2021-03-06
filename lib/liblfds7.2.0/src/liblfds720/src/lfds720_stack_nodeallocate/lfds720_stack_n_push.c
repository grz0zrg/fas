/***** includes *****/
#include "lfds720_stack_n_internal.h"





/****************************************************************************/
void lfds720_stack_n_threadsafe_push( struct lfds720_stack_n_state *ss,
                         struct lfds720_stack_n_element *se )
{
  char unsigned
    result;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  struct lfds720_stack_n_element LFDS720_PAL_ALIGN(LFDS720_PAL_DOUBLE_POINTER_LENGTH_IN_BYTES)
    *new_top[LFDS720_MISC_PAC_SIZE],
    *volatile original_top[LFDS720_MISC_PAC_SIZE];

  LFDS720_PAL_ASSERT( ss != NULL );
  LFDS720_PAL_ASSERT( se != NULL );

  new_top[LFDS720_MISC_POINTER] = se;

  original_top[LFDS720_MISC_COUNTER] = ss->top[LFDS720_MISC_COUNTER];
  original_top[LFDS720_MISC_POINTER] = ss->top[LFDS720_MISC_POINTER];

  do
  {
    se->next = original_top[LFDS720_MISC_POINTER];
    LFDS720_MISC_BARRIER_STORE;

    new_top[LFDS720_MISC_COUNTER] = original_top[LFDS720_MISC_COUNTER] + 1;
    LFDS720_PAL_ATOMIC_DWCAS( ss->top, original_top, new_top, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( ss->push_backoff, backoff_iteration );
  }
  while( result == 0 );

  LFDS720_BACKOFF_AUTOTUNE( ss->push_backoff, backoff_iteration );

  return;
}

