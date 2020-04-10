/***** includes *****/
#include "lfds720_stack_n_internal.h"





/****************************************************************************/
int lfds720_stack_n_threadsafe_pop( struct lfds720_stack_n_state *ss,
                                  struct lfds720_stack_n_element **se )
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

  LFDS720_MISC_BARRIER_LOAD;

  original_top[LFDS720_MISC_COUNTER] = ss->top[LFDS720_MISC_COUNTER];
  original_top[LFDS720_MISC_POINTER] = ss->top[LFDS720_MISC_POINTER];

  do
  {
    if( original_top[LFDS720_MISC_POINTER] == NULL )
    {
      *se = NULL;
      return 0;
    }

    new_top[LFDS720_MISC_COUNTER] = original_top[LFDS720_MISC_COUNTER] + 1;
    new_top[LFDS720_MISC_POINTER] = original_top[LFDS720_MISC_POINTER]->next;

    LFDS720_PAL_ATOMIC_DWCAS( ss->top, original_top, new_top, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

    if( result == 0 )
    {
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( ss->pop_backoff, backoff_iteration );
      LFDS720_MISC_BARRIER_LOAD;
    }
  }
  while( result == 0 );

  *se = original_top[LFDS720_MISC_POINTER];

  LFDS720_BACKOFF_AUTOTUNE( ss->pop_backoff, backoff_iteration );

  return 1;
}

