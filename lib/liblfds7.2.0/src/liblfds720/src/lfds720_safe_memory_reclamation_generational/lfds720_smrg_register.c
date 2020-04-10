/***** includes *****/
#include "lfds720_smrg_internal.h"





/****************************************************************************/
void lfds720_smrg_register_thread_using_new_smrg_thread_state( struct lfds720_smrg_state *smrgs,
                                                               struct lfds720_smrg_thread_state *smrgts )
{
  LFDS720_PAL_ASSERT( smrgs != NULL );
  LFDS720_PAL_ASSERT( smrgts != NULL );
  LFDS720_PAL_ASSERT( smrgts->generation_count_and_status_flags == LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE );

  smrgts->smrgs = smrgs;
  smrgts->generation_count_and_status_flags = LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW;

  LFDS720_MISC_BARRIER_STORE;

  LFDS720_LIST_NSU_SET_KEY_IN_ELEMENT( smrgts->asle, NULL );
  LFDS720_LIST_NSU_SET_VALUE_IN_ELEMENT( smrgts->asle, smrgts );
  lfds720_list_nsu_insert_at_start( &smrgs->smrg_thread_states, &smrgts->asle );

  return;
}





/****************************************************************************/
void lfds720_smrg_deregister_thread( struct lfds720_smrg_state *smrgs,
                                    struct lfds720_smrg_thread_state *smrgts )
{
  lfds720_pal_uint_t
    number_elements = 0;

  LFDS720_PAL_ASSERT( smrgs != NULL );
  LFDS720_PAL_ASSERT( smrgts != NULL );
  LFDS720_PAL_ASSERT( (smrgts->generation_count_and_status_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE) or (smrgts->generation_count_and_status_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW) );

  lfds720_smrg_release_all_cleaned_allocations_submitted_by_current_thread( smrgs, smrgts );

  stds_list_du_query( &smrgts->allocations, STDS_LIST_DU_QUERY_NUMBER_ELEMENTS, NULL, &number_elements );

  /* TRD : we're about to tell other threads we're available or retired
           so we need to ensure all the state which reflects this
           is published before we tell them
  */

  LFDS720_MISC_BARRIER_STORE;

  if( number_elements == 0 )
    smrgts->generation_count_and_status_flags = LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE;

  if( number_elements > 0 )
    smrgts->generation_count_and_status_flags = LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED;

  return;
}





/****************************************************************************/
int lfds720_smrg_register_thread_using_existing_available_smrg_thread_state( struct lfds720_smrg_state *smrgs,
                                                                             lfds720_pal_uint_t numa_node_id,
                                                                             struct lfds720_smrg_thread_state **smrgts )
{
  char unsigned 
    result = 0;

  lfds720_pal_uint_t LFDS720_PAL_ALIGN(LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES)
    compare;

  struct lfds720_list_nsu_element
    *asle = NULL;

  LFDS720_PAL_ASSERT( smrgs != NULL );
  // TRD : numa_node_id can be any value in its range
  LFDS720_PAL_ASSERT( smrgts != NULL );

  /* TRD : search for an available thread state
           claim it if we find it
  */

  while( result == 0 and LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrgs->smrg_thread_states, asle) )
  {
    *smrgts = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *asle );

    /* TRD : assuming threads are not registered/deregistered very often
             use the current value as a hint
             and only try CAS if available
    */

    if( ((*smrgts)->generation_count_and_status_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE) and (*smrgts)->numa_node_id == numa_node_id )
    {
      compare = LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE;
      LFDS720_PAL_ATOMIC_CAS( (*smrgts)->generation_count_and_status_flags, compare, (lfds720_pal_uint_t) LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
    }
  }

  return( (int) result );
}

