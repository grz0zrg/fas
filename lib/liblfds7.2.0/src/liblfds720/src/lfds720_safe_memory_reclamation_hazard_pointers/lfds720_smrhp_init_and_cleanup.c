/***** includes *****/
#include "lfds720_smrhp_internal.h"

/***** private prototypes *****/
static void lfds720_smrhp_internal_per_thread_state_cleanup_function( struct lfds720_list_nsu_state *lasus, struct lfds720_list_nsu_element *lasue );





/****************************************************************************/
void lfds720_smrhp_init_state( struct lfds720_smrhp_state *smrhps, void *smrhps_user_state )
{
  LFDS720_PAL_ASSERT( smrhps != NULL );
  // TRD : smrhps_user_state can be NULL

  lfds720_list_nsu_init_valid_on_current_logical_core( &smrhps->list_of_smrhp_per_thread_states, smrhps );

  smrhps->smrhps_user_state = smrhps_user_state;
  smrhps->state_has_been_initialized_safety_check_bitpattern = LFDS720_MISC_INIT_HAS_BEEN_CALLED_SAFETY_CHECK_BITPATTERN;

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}





/****************************************************************************/
void lfds720_smrhp_init_per_thread_state( struct lfds720_smrhp_per_thread_state *smrhppts, lfds720_pal_uint_t numa_node_id )
{
  lfds720_pal_uint_t
    loop;

  LFDS720_PAL_ASSERT( smrhppts != NULL );
  // TRD : numa_node_id can be any value in its range

  // TRD : alignment checks
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &smrhppts->status % LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES == 0 );

  smrhppts->status = LFDS720_SMRHPPTS_STATE_AVAILABLE;
  smrhppts->numa_node_id = numa_node_id;

  stds_list_du_init( &smrhppts->list_of_allocations_pending_reclamation, smrhppts );

  for( loop = 0 ; loop < LFDS720_SMRHP_SMRHPPTS_NUMBER_HAZARD_POINTERS ; loop++ )
    smrhppts->hazard_pointers[loop] = NULL;

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}





/****************************************************************************/
static void lfds720_smrhp_internal_per_thread_state_cleanup_function( struct lfds720_list_nsu_state *lasus, struct lfds720_list_nsu_element *lasue )
{
  struct lfds720_smrhp_state
    *smrhps;

  struct lfds720_smrhp_per_thread_state
    *smrhppts;

  LFDS720_PAL_ASSERT( lasus != NULL );
  LFDS720_PAL_ASSERT( lasue != NULL );

  smrhppts = (struct lfds720_smrhp_per_thread_state *) LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *lasue );
  smrhps  = (struct lfds720_smrhp_state *) LFDS720_LIST_NSU_GET_USER_STATE_FROM_STATE( *lasus );

  stds_list_du_cleanup( &smrhppts->list_of_allocations_pending_reclamation, lfds720_smrhp_internal_allocation_pending_reclamation_cleanup_function );

  if( smrhps->per_thread_state_deallocation_callback != NULL )
    smrhps->per_thread_state_deallocation_callback( smrhppts );

  return;
}





/****************************************************************************/
void lfds720_smrhp_internal_allocation_pending_reclamation_cleanup_function( struct stds_list_du_state *ldus, struct stds_list_du_element *ldue )
{
  struct lfds720_smrhp_per_thread_state
    *smrhppts;

  struct lfds720_smrhp_allocation_state
    *smrhpas;

  LFDS720_PAL_ASSERT( ldus != NULL );
  LFDS720_PAL_ASSERT( ldue != NULL );

  smrhppts = STDS_LIST_DU_GET_USER_STATE_FROM_STATE( *ldus );
  smrhpas = STDS_LIST_DU_GET_VALUE_FROM_ELEMENT( *ldue );

  smrhpas->allocation_reclaimed_callback( smrhppts, smrhpas, smrhpas->allocation, smrhppts->smrhps->smrhps_user_state, smrhpas->smrhpas_user_state );

  return;
}





/****************************************************************************/
void lfds720_smrhp_cleanup( struct lfds720_smrhp_state *smrhps,
                            void (*per_thread_state_deallocation_callback)(struct lfds720_smrhp_per_thread_state *smrhppts) )
{
  LFDS720_PAL_ASSERT( smrhps != NULL );
  // TRD : per_thread_state_deallocation_callback can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  smrhps->per_thread_state_deallocation_callback = per_thread_state_deallocation_callback;
  smrhps->state_has_been_initialized_safety_check_bitpattern = 0x0;

  lfds720_list_nsu_cleanup( &smrhps->list_of_smrhp_per_thread_states, lfds720_smrhp_internal_per_thread_state_cleanup_function );

  return;
}

