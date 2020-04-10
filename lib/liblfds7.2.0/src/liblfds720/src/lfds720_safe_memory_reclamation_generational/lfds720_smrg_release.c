/***** includes *****/
#include "lfds720_smrg_internal.h"





/****************************************************************************/
lfds720_pal_uint_t lfds720_smrg_release_all_cleaned_allocations_submitted_by_current_thread( struct lfds720_smrg_state *smrgs,
                                                                                             struct lfds720_smrg_thread_state *smrgts )
{
  lfds720_pal_uint_t
    difference,
    local_smrgs_generation_count,
    release_count = 0;

  struct lfds720_smrg_allocation_state
    *smrgas;

  struct stds_list_du_element
    *stle = NULL,
    *stle_prev;

  LFDS720_PAL_ASSERT( smrgs != NULL );
  LFDS720_PAL_ASSERT( smrgts != NULL );

  // TRD : smrgs->generation_count can be behind, we don't memory barrier for it, but it does no harm
  local_smrgs_generation_count = LFDS720_SMRG_GET_GENERATION( smrgs->generation_count_and_status_flags );

  while( STDS_LIST_DU_GET_START_AND_THEN_NEXT(smrgts->allocations, stle) )
  {
    smrgas = STDS_LIST_DU_GET_VALUE_FROM_ELEMENT( *stle );

    // TRD : deal with wrap-around on the generation counter
    if( smrgas->generation_count <= local_smrgs_generation_count )
      difference = local_smrgs_generation_count - smrgas->generation_count;
    else
      difference = local_smrgs_generation_count + (~((lfds720_pal_uint_t)0) - smrgas->generation_count);

    if( difference >= 2 )
    {
      stle_prev = STDS_LIST_DU_GET_PREV( *stle );
      stds_list_du_remove_element( &smrgts->allocations, stle );
      stle = stle_prev;
      // TRD : the function call MUST happen after the list remove, or we may make the element public again and someone else could use our list_du_element before we do!
      smrgas->allocation_cleaned_callback( smrgts, smrgas, smrgas->allocation, smrgts->smrgs->smrg_user_state, smrgas->allocation_user_state );
      ++release_count;
    }
  }

  return( release_count );
}





/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds720_smrg_internal_allocation_cleanup_function( struct stds_list_du_state *ldus, struct stds_list_du_element *ldue )
{
  struct lfds720_smrg_thread_state
    *smrgts;

  struct lfds720_smrg_allocation_state
    *smrgas;

  LFDS720_PAL_ASSERT( ldus != NULL );
  LFDS720_PAL_ASSERT( ldue != NULL );

  smrgas = (struct lfds720_smrg_allocation_state *) STDS_LIST_DU_GET_VALUE_FROM_ELEMENT( *ldue );
  smrgts = (struct lfds720_smrg_thread_state *) STDS_LIST_DU_GET_USER_STATE_FROM_STATE( *ldus );

  smrgas->allocation_cleaned_callback( smrgts, smrgas, smrgas->allocation, smrgts->smrgs->smrg_user_state, smrgas->allocation_user_state );

  return;
}

#pragma warning( default : 4100 )

