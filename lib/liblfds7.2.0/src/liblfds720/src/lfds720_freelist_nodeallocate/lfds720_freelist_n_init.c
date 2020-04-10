/***** includes *****/
#include "lfds720_freelist_n_internal.h"





/****************************************************************************/
void lfds720_freelist_n_init_valid_on_current_logical_core( struct lfds720_freelist_n_state *fs,
                                                            void *user_state )
{
  LFDS720_PAL_ASSERT( fs != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) fs->top % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  // TRD : user_state can be NULL

  fs->top[LFDS720_MISC_POINTER] = NULL;
  fs->top[LFDS720_MISC_COUNTER] = 0;
  fs->elimination_array = NULL;
  fs->elimination_array_number_of_lines = 0;
  fs->user_state = user_state;

  lfds720_misc_internal_backoff_init( &fs->pop_backoff );
  lfds720_misc_internal_backoff_init( &fs->push_backoff );

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

