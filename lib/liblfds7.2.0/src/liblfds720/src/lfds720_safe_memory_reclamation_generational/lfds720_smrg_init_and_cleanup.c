/***** includes *****/
#include "lfds720_smrg_internal.h"





/****************************************************************************/
void lfds720_smrg_init_smr( struct lfds720_smrg_state *smrgs, void *smrg_user_state )
{
  LFDS720_PAL_ASSERT( smrgs != NULL );
  // TRD : smrg_user_state can be NULL

  // TRD : alignment checks
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &smrgs->generation_count_and_status_flags % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );

  lfds720_list_nsu_init_valid_on_current_logical_core( &smrgs->smrg_thread_states, smrgs );

  smrgs->smrg_user_state = smrg_user_state;

  smrgs->generation_count_and_status_flags = LFDS720_SMRG_SMRGS_FLAG_STATUS_SETTING;

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}





/****************************************************************************/
void lfds720_smrg_init_smrg_thread( struct lfds720_smrg_thread_state *smrgts, lfds720_pal_uint_t numa_node_id )
{
  LFDS720_PAL_ASSERT( smrgts != NULL );
  // TRD : numa_node_id can be any value in its range

  // TRD : alignment checks
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &smrgts->generation_count_and_status_flags % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &smrgts->numa_node_id % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );

  smrgts->generation_count_and_status_flags = LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE;
  smrgts->numa_node_id = numa_node_id;
  stds_list_du_init( &smrgts->allocations, smrgts );

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}





/****************************************************************************/
void lfds720_smrg_cleanup( struct lfds720_smrg_state *smrgs,
                          void (*thread_state_deallocation_callback)(struct lfds720_smrg_thread_state *smrgts) )
{
  LFDS720_PAL_ASSERT( smrgs != NULL );
  // TRD : thread_state_deallocation_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  smrgs->thread_state_deallocation_callback = thread_state_deallocation_callback;

  lfds720_list_nsu_cleanup( &smrgs->smrg_thread_states, lfds720_smrg_internal_thread_state_cleanup_function );

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds720_smrg_internal_thread_state_cleanup_function( struct lfds720_list_nsu_state *lasus, struct lfds720_list_nsu_element *lasue )
{
  struct lfds720_smrg_state
    *smrgs;

  struct lfds720_smrg_thread_state
    *smrgts;

  LFDS720_PAL_ASSERT( lasus != NULL );
  LFDS720_PAL_ASSERT( lasue != NULL );

  smrgts = (struct lfds720_smrg_thread_state *) LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *lasue );
  smrgs  = (struct lfds720_smrg_state *) LFDS720_LIST_NSU_GET_USER_STATE_FROM_STATE( *lasus );

  stds_list_du_cleanup( &smrgts->allocations, lfds720_smrg_internal_allocation_cleanup_function );

  if( smrgs->thread_state_deallocation_callback != NULL )
    smrgs->thread_state_deallocation_callback( smrgts );

  return;
}

#pragma warning( default : 4100 )

