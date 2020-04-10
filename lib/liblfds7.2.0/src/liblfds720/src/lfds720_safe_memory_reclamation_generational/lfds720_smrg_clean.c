/***** includes *****/
#include "lfds720_smrg_internal.h"





/****************************************************************************/
void lfds720_smrg_clean_eligible_dirty_allocations_for_all_threads( struct lfds720_smrg_state *smrgs, enum lfds720_misc_flag *generation_flag, enum lfds720_misc_flag *cleaning_flag )
{
  char unsigned
    result;

  lfds720_pal_uint_t
    new_smrgs_generation_count_and_status_flags,
    new_smrgts_generation_count_and_status_flags,
    number_elements,
    smrgs_generation_count_and_status_flags,
    smrgts_generation_count_and_status_flags,
    smrgs_generation,
    smrgs_flags,
    smrgts_generation,
    smrgts_flags;

  enum lfds720_misc_flag
    setting_looking_good_flag = LFDS720_MISC_FLAG_RAISED,
    clearing_looking_good_flag = LFDS720_MISC_FLAG_RAISED;

  struct lfds720_list_nsu_element
    *asle;

  struct lfds720_smrg_thread_state
    *smrgts;

  LFDS720_PAL_ASSERT( smrgs != NULL );
  // TRD : generation_flag can be NULL
  // TRD : cleaning_occurred_flag can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  if( generation_flag != NULL )
    *generation_flag = LFDS720_MISC_FLAG_LOWERED;

  if( cleaning_flag != NULL )
    *cleaning_flag = LFDS720_MISC_FLAG_LOWERED;

  smrgs_generation_count_and_status_flags = smrgs->generation_count_and_status_flags;
  smrgs_generation = LFDS720_SMRG_GET_GENERATION( smrgs_generation_count_and_status_flags );
  smrgs_flags = LFDS720_SMRG_GET_FLAGS( smrgs_generation_count_and_status_flags );

  /* TRD : if we think we're in stage one
           then we're iterating over thread states
           checking to see if they can be moved into the ready state
  */

  if( smrgs_flags & LFDS720_SMRG_SMRGS_FLAG_STATUS_SETTING )
  {
    // TRD : by default, we expect to bump the SMR gen counter, unless we see reason not to
    asle = NULL;

    while( setting_looking_good_flag == LFDS720_MISC_FLAG_RAISED and LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrgs->smrg_thread_states, asle) )
    {
      smrgts = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *asle );

      smrgts_generation_count_and_status_flags = smrgts->generation_count_and_status_flags;
      smrgts_generation = LFDS720_SMRG_GET_GENERATION( smrgts_generation_count_and_status_flags );
      smrgts_flags = LFDS720_SMRG_GET_FLAGS( smrgts_generation_count_and_status_flags );

      // TRD : if a thread state is on the current generation and in progress and not exited, we can't advance the SMR gen counter
      if(  ( ((smrgts_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE) and (smrgts_generation == smrgs_generation)) or (smrgts_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW) ) and
           (smrgts_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_LOCKFREE_IN_PROGRESS) and
          !(smrgts_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_EXITED_LOCKFREE) )
        setting_looking_good_flag = LFDS720_MISC_FLAG_LOWERED;
    }

    if( setting_looking_good_flag == LFDS720_MISC_FLAG_RAISED )
    {
      new_smrgs_generation_count_and_status_flags = LFDS720_SMRG_COMPOSE_GENERATION_AND_FLAGS( smrgs_generation+1, LFDS720_SMRG_SMRGS_FLAG_STATUS_CLEARING );
      LFDS720_PAL_ATOMIC_CAS( smrgs->generation_count_and_status_flags, smrgs_generation_count_and_status_flags, new_smrgs_generation_count_and_status_flags, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
      if( result == 0 )
      {
        smrgs_generation = LFDS720_SMRG_GET_GENERATION( smrgs_generation_count_and_status_flags );
        smrgs_flags = LFDS720_SMRG_GET_FLAGS( smrgs_generation_count_and_status_flags );
        setting_looking_good_flag = LFDS720_MISC_FLAG_LOWERED;
      }
      if( result == 1 )
      {
        smrgs_generation_count_and_status_flags = new_smrgs_generation_count_and_status_flags;
        smrgs_generation = LFDS720_SMRG_GET_GENERATION( new_smrgs_generation_count_and_status_flags );
        smrgs_flags = LFDS720_SMRG_GET_FLAGS( new_smrgs_generation_count_and_status_flags );
      }
    }

    if( generation_flag != NULL )
      *generation_flag = setting_looking_good_flag;
  }

  /* TRD : if we think all threads are in the ready state
           but we've done the global generation increment
           then we're iterating over thread states
           performing the necessary updates
  */

  if( smrgs_flags & LFDS720_SMRG_SMRGS_FLAG_STATUS_CLEARING )
  {
    asle = NULL;

    while( clearing_looking_good_flag == LFDS720_MISC_FLAG_RAISED and LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrgs->smrg_thread_states, asle) )
    {
      smrgts = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *asle );

      smrgts_generation_count_and_status_flags = smrgts->generation_count_and_status_flags;
      smrgts_generation = LFDS720_SMRG_GET_GENERATION( smrgts_generation_count_and_status_flags );
      smrgts_flags = LFDS720_SMRG_GET_FLAGS( smrgts_generation_count_and_status_flags );

      if( ((smrgts_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE) and (smrgts_generation == smrgs_generation-1)) or
          (smrgts_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW) )
      {
        new_smrgts_generation_count_and_status_flags = LFDS720_SMRG_COMPOSE_GENERATION_AND_FLAGS( smrgs_generation, (smrgts_flags & ~(LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_LOCKFREE_IN_PROGRESS | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_EXITED_LOCKFREE | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW)) | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE );
        LFDS720_PAL_ATOMIC_CAS( smrgts->generation_count_and_status_flags, smrgts_generation_count_and_status_flags, new_smrgts_generation_count_and_status_flags, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
        if( result == 1 )
        {
          smrgts_generation = LFDS720_SMRG_GET_GENERATION( new_smrgts_generation_count_and_status_flags );
          smrgts_flags = LFDS720_SMRG_GET_FLAGS( new_smrgts_generation_count_and_status_flags );
        }
      }

      /* TRD : thread generation counts are advanced during the setting stage
               if we find a thread with a generation count greater than the SMRGS generation count we noted when we began
               someone else has finished another setting stage, which means we're behind and can safely abort

               we should never find a thread with a lower generation count (logically lower) but since the counter
               wrapers logically lower may in fact mean physically lower - so we use "!=" rather than ">"
      */

      if( (smrgts_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE) and (smrgts_generation != smrgs_generation) )
        clearing_looking_good_flag = LFDS720_MISC_FLAG_LOWERED;
    }

    if( clearing_looking_good_flag == LFDS720_MISC_FLAG_RAISED )
    {
      // TRD : we're done clearing - go back to setting
      new_smrgs_generation_count_and_status_flags = LFDS720_SMRG_COMPOSE_GENERATION_AND_FLAGS( smrgs_generation, LFDS720_SMRG_SMRGS_FLAG_STATUS_SETTING );
      LFDS720_PAL_ATOMIC_CAS( smrgs->generation_count_and_status_flags, smrgs_generation_count_and_status_flags, new_smrgs_generation_count_and_status_flags, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
    }

    if( cleaning_flag != NULL )
      *cleaning_flag = clearing_looking_good_flag;
  }

  /* TRD : finally, retired thread handling
           any thread can do this, at any time, since it only depends on the RETIRED/RETIRING flag
  */

  asle = NULL;

  while( LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrgs->smrg_thread_states, asle) )
  {
    smrgts = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *asle );

    smrgts_generation_count_and_status_flags = smrgts->generation_count_and_status_flags;
    smrgts_generation = LFDS720_SMRG_GET_GENERATION( smrgts_generation_count_and_status_flags );
    smrgts_flags = LFDS720_SMRG_GET_FLAGS( smrgts_generation_count_and_status_flags );

    if( smrgts_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED )
    {
      new_smrgts_generation_count_and_status_flags = LFDS720_SMRG_COMPOSE_GENERATION_AND_FLAGS( smrgs_generation, (smrgts_flags & ~LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED) | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRING );
      LFDS720_PAL_ATOMIC_CAS( smrgts->generation_count_and_status_flags, smrgts_generation_count_and_status_flags, new_smrgts_generation_count_and_status_flags, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
      if( result == 1 )
      {
        smrgts_generation_count_and_status_flags = new_smrgts_generation_count_and_status_flags;
        smrgts_generation = LFDS720_SMRG_GET_GENERATION( smrgts_generation_count_and_status_flags );
        smrgts_flags = LFDS720_SMRG_GET_FLAGS( smrgts_generation_count_and_status_flags );

        lfds720_smrg_release_all_cleaned_allocations_submitted_by_current_thread( smrgs, smrgts );

        stds_list_du_query( &smrgts->allocations, STDS_LIST_DU_QUERY_NUMBER_ELEMENTS, NULL, &number_elements );

        if( number_elements > 0 )
          smrgts->generation_count_and_status_flags = LFDS720_SMRG_COMPOSE_GENERATION_AND_FLAGS( smrgs_generation, (smrgts_flags & ~LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRING) | LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED );

        if( number_elements == 0 )
        {
          // TRD : must be atomic, or some threads may think it is still retired and perform retirement, even though another thread has already begun using it
          new_smrgts_generation_count_and_status_flags = LFDS720_SMRG_COMPOSE_GENERATION_AND_FLAGS( 0, LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE );
          LFDS720_PAL_ATOMIC_CAS( smrgts->generation_count_and_status_flags, smrgts_generation_count_and_status_flags, new_smrgts_generation_count_and_status_flags, LFDS720_MISC_CAS_STRENGTH_STRONG, result );
        }
      }
    }
  }

  LFDS720_MISC_BARRIER_STORE;

  return;
}

