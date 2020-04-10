/***** includes *****/
#include "lfds720_list_singlylinked_unordered_internal.h"





/****************************************************************************/
void lfds720_list_so_init_valid_on_current_logical_core( struct lfds720_list_so_state *lsos,
                                                         int (*key_compare_function)(void const *new_key, void const *existing_key),
                                                         enum lfds720_list_so_existing_key existing_key,
                                                         struct lfds720_smrhp_state *smrhps,
                                                         void *user_state );
{
  LFDS720_PAL_ASSERT( lsos != NULL );
  LFDS720_PAL_ASSERT( key_compare_function != NULL );
  // TRD : existing_key can be any value in its range
  LFDS720_PAL_ASSERT( smrhps != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lsos->dummy_element % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lsos->end % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lsos->start % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  // TRD : user_state can be NULL

  // TRD : dummy start element - makes code easier when you can always use ->next
  lsos->start = &lsos->dummy_element;

  lsos->start->next = NULL;
  lsos->user_state = user_state;

  lsos->key_compare_function = key_compare_function;
  lsos->existing_key = existing_key;
  lsos->smrhps = smrhps;

  lfds720_misc_internal_backoff_init( &lsos->insert_backoff );

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

