/***** includes *****/
#include "lfds720_smrg_internal.h"





/****************************************************************************/
void lfds720_smrg_flush( struct lfds720_smrg_state *smrgs )
{
  lfds720_pal_uint_t
    smrgs_generation_count_and_status_flags,
    smrgs_generation,
    smrgs_flags;

  lfds720_pal_uint_t
    number_elements;

  struct lfds720_list_nsu_element
    *asle = NULL;

  struct lfds720_smrg_thread_state
    *smrgts;

  LFDS720_PAL_ASSERT( smrgs != NULL );

  /* TRD : this function is used by the test application
           it flushes the SMR completely

           it is not thread safe and so cannot be used in normal operation
           if used, all other SMR-using threads need to re-issue lfds720_smrg_use()
           so that they are guaranteed to see the new SMR state

           we keep elements for two generations,
           so we artifically pop a single extra generation
           as lfds720_smrg_scan() will pop another

           we then need to iterate over each active and retired thread state
           and purge the reuse candidate list
  */

  LFDS720_MISC_BARRIER_LOAD;

  smrgs_generation_count_and_status_flags = smrgs->generation_count_and_status_flags;
  smrgs_generation = LFDS720_SMRG_GET_GENERATION( smrgs_generation_count_and_status_flags );
  smrgs_flags = LFDS720_SMRG_GET_FLAGS( smrgs_generation_count_and_status_flags );

  smrgs->generation_count_and_status_flags = LFDS720_SMRG_COMPOSE_GENERATION_AND_FLAGS( smrgs_generation+2, smrgs_flags );

  while( LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrgs->smrg_thread_states, asle) )
  {
    smrgts = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *asle );

    switch( smrgts->generation_count_and_status_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_REDUCED_MASK )
    {
      case LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE:
        smrgts->generation_count_and_status_flags &= ~LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_EXITED_LOCKFREE;
      break;

      case LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED:
      {
        lfds720_smrg_release_all_cleaned_allocations_submitted_by_current_thread( smrgs, smrgts );

        stds_list_du_query( &smrgts->allocations, STDS_LIST_DU_QUERY_NUMBER_ELEMENTS, NULL, &number_elements );

        if( number_elements == 0 )
          smrgts->generation_count_and_status_flags = LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE;
      }
    }
  }

  // TRD : and now release cleaned elements for every thread state, active or retired

  asle = NULL;

  while( LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrgs->smrg_thread_states, asle) )
  {
    smrgts = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *asle );

    switch( smrgts->generation_count_and_status_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_REDUCED_MASK )
    {
      case LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE:
        lfds720_smrg_release_all_cleaned_allocations_submitted_by_current_thread( smrgs, smrgts );
      break;

      case LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED:
      {
        // TRD : this function call is really in its nature singlethreaded, as opposed to genuinely "by current thread"
        lfds720_smrg_release_all_cleaned_allocations_submitted_by_current_thread( smrgs, smrgts );

        // TRD : liberate the retired state
        smrgts->generation_count_and_status_flags = LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE;
      }
    }
  }

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

