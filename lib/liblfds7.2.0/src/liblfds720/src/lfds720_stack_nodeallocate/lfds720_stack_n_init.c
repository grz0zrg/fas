/***** includes *****/
#include "lfds720_stack_n_internal.h"





/****************************************************************************/
void lfds720_stack_n_init_valid_on_current_logical_core( struct lfds720_stack_n_state *ss,
                                                        void *user_state )
{
  LFDS720_PAL_ASSERT( ss != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) ss->top % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &ss->user_state % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  // TRD : user_state can be NULL

  ss->top[LFDS720_MISC_POINTER] = NULL;
  ss->top[LFDS720_MISC_COUNTER] = 0;

  ss->user_state = user_state;

  lfds720_misc_internal_backoff_init( &ss->pop_backoff );
  lfds720_misc_internal_backoff_init( &ss->push_backoff );

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

