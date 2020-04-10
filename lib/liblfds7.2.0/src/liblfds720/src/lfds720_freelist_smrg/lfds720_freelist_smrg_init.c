/***** includes *****/
#include "lfds720_freelist_smrg_internal.h"





/****************************************************************************/
void lfds720_freelist_smrg_init_valid_on_current_logical_core( struct lfds720_freelist_smrg_state *fsgs,
                                                                struct lfds720_smrg_state *smrgs,
                                                                void *user_state )
{
  LFDS720_PAL_ASSERT( fsgs != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &fsgs->top % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &fsgs->user_state % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( smrgs != NULL );
  // TRD : user_state can be NULL

  fsgs->top = NULL;

  fsgs->smrgs = smrgs;

  fsgs->user_state = user_state;

  lfds720_misc_internal_backoff_init( &fsgs->pop_backoff );
  lfds720_misc_internal_backoff_init( &fsgs->push_backoff );

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

