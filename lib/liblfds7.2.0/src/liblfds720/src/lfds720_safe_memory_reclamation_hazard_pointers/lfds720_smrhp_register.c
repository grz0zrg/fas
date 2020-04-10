/***** includes *****/
#include "lfds720_smrhp_internal.h"





/****************************************************************************/
void lfds720_smrhp_register_thread_using_new_smrhp_per_thread_state( struct lfds720_smrhp_state *smrhps,
                                                                     struct lfds720_smrhp_per_thread_state *smrhppts )
{
  LFDS720_PAL_ASSERT( smrhps != NULL );
  LFDS720_PAL_ASSERT( smrhps->state_has_been_initialized_safety_check_bitpattern == LFDS720_MISC_INIT_HAS_BEEN_CALLED_SAFETY_CHECK_BITPATTERN );
  LFDS720_PAL_ASSERT( smrhppts != NULL );
  LFDS720_PAL_ASSERT( smrhppts->status == LFDS720_SMRHPPTS_STATE_AVAILABLE );

  smrhppts->smrhps = smrhps;
  smrhppts->status = LFDS720_SMRHPPTS_STATE_ACTIVE;

  LFDS720_MISC_BARRIER_STORE;

  LFDS720_LIST_NSU_SET_VALUE_IN_ELEMENT( smrhppts->asle, smrhppts );
  lfds720_list_nsu_insert_at_start( &smrhps->list_of_smrhp_per_thread_states, &smrhppts->asle );

  return;
}





/****************************************************************************/
void lfds720_smrhp_deregister_thread( struct lfds720_smrhp_state *smrhps,
                                      struct lfds720_smrhp_per_thread_state *smrhppts )
{
  lfds720_pal_uint_t
    loop,
    number_elements;

  LFDS720_PAL_ASSERT( smrhps != NULL );
  LFDS720_PAL_ASSERT( smrhps->state_has_been_initialized_safety_check_bitpattern == LFDS720_MISC_INIT_HAS_BEEN_CALLED_SAFETY_CHECK_BITPATTERN );
  LFDS720_PAL_ASSERT( smrhppts != NULL );
  LFDS720_PAL_ASSERT( smrhppts->status == LFDS720_SMRHPPTS_STATE_ACTIVE );

  for( loop = 0 ; loop < LFDS720_SMRHP_SMRHPPTS_NUMBER_HAZARD_POINTERS ; loop++ )
    smrhppts->hazard_pointers[loop] = NULL;

  lfds720_smrhp_internal_reclaim_reclaimable_allocations_submitted_by_current_thread( smrhps, smrhppts );

  LFDS720_MISC_BARRIER_STORE;

  stds_list_du_query( &smrhppts->list_of_allocations_pending_reclamation, STDS_LIST_DU_QUERY_NUMBER_ELEMENTS, NULL, &number_elements );

  if( number_elements == 0 )
    smrhppts->status = LFDS720_SMRHPPTS_STATE_AVAILABLE;
  else
    smrhppts->status = LFDS720_SMRHPPTS_STATE_RETIRED;

  return;
}





/****************************************************************************/
int lfds720_smrhp_register_thread_using_existing_available_smrhp_per_thread_state( struct lfds720_smrhp_state *smrhps,
                                                                                   lfds720_pal_uint_t numa_node_id,
                                                                                   struct lfds720_smrhp_per_thread_state **smrhppts )
{
  char unsigned 
    result = 0;

  enum lfds720_smrhppts_state LFDS720_PAL_ALIGN(LFDS720_PAL_CACHE_LINE_LENGTH_IN_BYTES)
    compare;

  struct lfds720_list_nsu_element
    *asle = NULL;

  LFDS720_PAL_ASSERT( smrhps != NULL );
  LFDS720_PAL_ASSERT( smrhps->state_has_been_initialized_safety_check_bitpattern == LFDS720_MISC_INIT_HAS_BEEN_CALLED_SAFETY_CHECK_BITPATTERN );
  // TRD : numa_node_id can be any value in its range
  LFDS720_PAL_ASSERT( smrhppts != NULL );

  /* TRD : search for an available thread state
           claim it if we find it
  */

  while( result == 0 and LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrhps->list_of_smrhp_per_thread_states, asle) )
  {
    *smrhppts = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *asle );

    /* TRD : assuming threads are not registered/deregistered very often
             use the current value as a hint
             and only try CAS if available
    */

    if( ((*smrhppts)->status == LFDS720_SMRHPPTS_STATE_AVAILABLE) and (*smrhppts)->numa_node_id == numa_node_id )
    {
      compare = LFDS720_SMRHPPTS_STATE_AVAILABLE;
      LFDS720_PAL_ATOMIC_CAS( (*smrhppts)->status, compare, (lfds720_pal_uint_t) LFDS720_SMRHPPTS_STATE_ACTIVE, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
    }
  }

  return( (int) result );
}

