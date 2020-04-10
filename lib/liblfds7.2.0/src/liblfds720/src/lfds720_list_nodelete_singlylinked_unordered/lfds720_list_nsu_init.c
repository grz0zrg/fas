/***** includes *****/
#include "lfds720_list_nsu_internal.h"





/****************************************************************************/
void lfds720_list_nsu_init_valid_on_current_logical_core( struct lfds720_list_nsu_state *lasus,
                                                           void *user_state )
{
  LFDS720_PAL_ASSERT( lasus != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasus->dummy_element % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasus->end % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasus->start % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  // TRD : user_state can be NULL

  // TRD : dummy start element - makes code easier when you can always use ->next
  lasus->start = lasus->end = &lasus->dummy_element;

  lasus->start->next = NULL;
  lasus->start->value = NULL;
  lasus->user_state = user_state;

  lfds720_misc_internal_backoff_init( &lasus->after_backoff );
  lfds720_misc_internal_backoff_init( &lasus->end_backoff );
  lfds720_misc_internal_backoff_init( &lasus->start_backoff );

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

