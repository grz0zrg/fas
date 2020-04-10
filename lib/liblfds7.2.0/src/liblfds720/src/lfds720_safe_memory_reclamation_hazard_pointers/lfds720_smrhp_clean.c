/***** includes *****/
#include "lfds720_smrhp_internal.h"





/****************************************************************************/
lfds720_pal_uint_t lfds720_smrhp_reclaim_reclaimable_allocations_submitted_by_current_thread( struct lfds720_smrhp_state *smrhps,
                                                                                              struct lfds720_smrhp_per_thread_state *smrhppts )
{
  char unsigned
    result;

  enum lfds720_smrhppts_state
    status;

  lfds720_pal_uint_t
    number_elements,
    reclaimed_count = 0;

  struct lfds720_list_nsu_element
    *lasue = NULL;

  struct lfds720_smrhp_per_thread_state
    *smrhppts_temp;

  LFDS720_PAL_ASSERT( smrhps != NULL );
  LFDS720_PAL_ASSERT( smrhps->state_has_been_initialized_safety_check_bitpattern == LFDS720_MISC_INIT_HAS_BEEN_CALLED_SAFETY_CHECK_BITPATTERN );
  LFDS720_PAL_ASSERT( smrhppts != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  reclaimed_count = lfds720_smrhp_internal_reclaim_reclaimable_allocations_submitted_by_current_thread( smrhps, smrhppts );

  // TRD : now scan the thread state list for retired threads and try to finish them off

  while( LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrhps->list_of_smrhp_per_thread_states,lasue) )
  {
    smrhppts_temp = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *lasue );

    status = smrhppts_temp->status;

    if( status == LFDS720_SMRHPPTS_STATE_RETIRED )
    {
      LFDS720_PAL_ATOMIC_CAS( smrhppts_temp->status, status, LFDS720_SMRHPPTS_STATE_RETIRING, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
      if( result == 1 )
      {
        reclaimed_count += lfds720_smrhp_internal_reclaim_reclaimable_allocations_submitted_by_current_thread( smrhps, smrhppts_temp );
        stds_list_du_query( &smrhppts_temp->list_of_allocations_pending_reclamation, STDS_LIST_DU_QUERY_NUMBER_ELEMENTS, NULL, &number_elements );
        if( number_elements == 0 )
          smrhppts_temp->status = LFDS720_SMRHPPTS_STATE_AVAILABLE;
        else
          smrhppts_temp->status = LFDS720_SMRHPPTS_STATE_RETIRED;
      }
    }
  }

  return reclaimed_count;
}





/****************************************************************************/
lfds720_pal_uint_t lfds720_smrhp_internal_reclaim_reclaimable_allocations_submitted_by_current_thread( struct lfds720_smrhp_state *smrhps,
                                                                                                       struct lfds720_smrhp_per_thread_state *smrhppts )
{
  lfds720_pal_uint_t
    index,
    reclaimed_count = 0;

  struct lfds720_smrhp_allocation_state
    *smrhpas;

  struct lfds720_list_nsu_element
    *lasue = NULL;

  struct lfds720_smrhp_per_thread_state
    *smrhppts_temp;

  struct stds_list_du_element
    *le,
    *le_prev;

  LFDS720_PAL_ASSERT( smrhps != NULL );
  LFDS720_PAL_ASSERT( smrhps->state_has_been_initialized_safety_check_bitpattern == LFDS720_MISC_INIT_HAS_BEEN_CALLED_SAFETY_CHECK_BITPATTERN );
  LFDS720_PAL_ASSERT( smrhppts != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  /* TRD : iterate over all the hazard pointers held by all other threads
           for each pointer, iterate over the list of our own pending allocations
           if we see our allocation, we mark it as an element which cannot be reclamed
           once we have scanned all hazard pointers
           we then iterate again over our own list
           reclaming those allocations which can be reclamed
           and removing the cannot-reclaim mark from those elements which had it set
  */

  // TRD : first pass over allocations to set "can't reclaim" flags
  le = NULL;

  while( STDS_LIST_DU_GET_START_AND_THEN_NEXT(smrhppts->list_of_allocations_pending_reclamation,le) )
  {
    smrhpas = STDS_LIST_DU_GET_VALUE_FROM_ELEMENT( *le );
    smrhpas->pointed_at_flag = LFDS720_MISC_FLAG_LOWERED;
  }

  while( LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrhps->list_of_smrhp_per_thread_states,lasue) )
  {
    smrhppts_temp = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *lasue );

    if( smrhppts_temp->status == LFDS720_SMRHPPTS_STATE_ACTIVE )
    {
      le = NULL;

      while( STDS_LIST_DU_GET_START_AND_THEN_NEXT(smrhppts->list_of_allocations_pending_reclamation,le) )
      {
        smrhpas = STDS_LIST_DU_GET_VALUE_FROM_ELEMENT( *le );

        for( index = 0 ; smrhpas->pointed_at_flag == LFDS720_MISC_FLAG_LOWERED and index < LFDS720_SMRHP_SMRHPPTS_NUMBER_HAZARD_POINTERS ; ++index )
          if( smrhppts_temp->hazard_pointers[index] == smrhpas->allocation )
            smrhpas->pointed_at_flag = LFDS720_MISC_FLAG_RAISED;
      }
    }
  }

  // TRD : now reclaim everything which can be reclamed
  le = NULL;

  while( STDS_LIST_DU_GET_START_AND_THEN_NEXT(smrhppts->list_of_allocations_pending_reclamation,le) )
  {
    smrhpas = STDS_LIST_DU_GET_VALUE_FROM_ELEMENT( *le );

    if( smrhpas->pointed_at_flag == LFDS720_MISC_FLAG_LOWERED )
    {
      le_prev = STDS_LIST_DU_GET_PREV( *le );
      stds_list_du_remove_element( &smrhppts->list_of_allocations_pending_reclamation, le );
      le = le_prev;
      LFDS720_MISC_BARRIER_STORE;
      // TRD : the function call must happen *after* the list remove, or we may make the element public again and someone else could use our list_du_element before we do!
      --smrhpas->count;
      smrhpas->allocation_reclaimed_callback( smrhppts, smrhpas, smrhpas->allocation, smrhps->smrhps_user_state, smrhpas->smrhpas_user_state );
      ++reclaimed_count;
    }
  }

  return reclaimed_count;
}

