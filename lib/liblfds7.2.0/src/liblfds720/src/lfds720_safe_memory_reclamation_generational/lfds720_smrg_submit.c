/***** includes *****/
#include "lfds720_smrg_internal.h"





/****************************************************************************/
void lfds720_smrg_submit_dirty_allocation( struct lfds720_smrg_state *smrgs,
                                           struct lfds720_smrg_thread_state *smrgts,
                                           struct lfds720_smrg_allocation_state *smrgas,
                                           void (*allocation_cleaned_callback)( struct lfds720_smrg_thread_state *smrgts, struct lfds720_smrg_allocation_state *smrgas, void *value, void *smrg_user_state, void *allocation_user_state ),
                                           void *allocation,
                                           void *allocation_user_state )
{
  LFDS720_PAL_ASSERT( smrgs != NULL );
  LFDS720_PAL_ASSERT( smrgts != NULL );
  LFDS720_PAL_ASSERT( smrgas != NULL );
  LFDS720_PAL_ASSERT( allocation_cleaned_callback != NULL );
  LFDS720_PAL_ASSERT( allocation != NULL );
  // TRD : allocation_user_state can be NULL

  /* TRD : we're handed a dirty allocation
           place it into the thread-local reuse candidates list
  */

  smrgas->generation_count = LFDS720_SMRG_GET_GENERATION( smrgs->generation_count_and_status_flags );
  smrgas->allocation = allocation;
  smrgas->allocation_user_state = allocation_user_state;
  smrgas->allocation_cleaned_callback = allocation_cleaned_callback;

  // TRD : needs to be linked to the end of the list, so we reuse in the order of arrival
  STDS_LIST_DU_SET_VALUE_IN_ELEMENT( smrgas->stle, smrgas );
  stds_list_du_insert_at_end( &smrgts->allocations, &smrgas->stle );

  return;
}

