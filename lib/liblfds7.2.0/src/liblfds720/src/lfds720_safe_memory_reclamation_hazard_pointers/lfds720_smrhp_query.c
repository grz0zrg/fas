/***** includes *****/
#include "lfds720_smrhp_internal.h"

/***** private prototypes *****/
static void lfds720_smrhp_internal_thread_validate( struct lfds720_smrhp_per_thread_state *smrhpts, struct lfds720_smrhp_thread_validation_info *smrhptvi, enum lfds720_misc_validity *lfds720_smrhp_thread_validity );
static void lfds720_smrhp_internal_validate( struct lfds720_smrhp_state *smrhps, struct lfds720_smrhp_validation_info *smrvi, enum lfds720_misc_validity *lfds720_smrhp_validity );
static int lfds720_smrhp_internal_compare_struct_lfds720_smrhp_per_thread_state_counts( struct lfds720_smrhp_per_thread_state_counts *s1, struct lfds720_smrhp_per_thread_state_counts *s2 );





/****************************************************************************/
void lfds720_smrhp_query( struct lfds720_smrhp_state *smrhps, enum lfds720_smrhp_query query_type, void *query_input, void *query_output )
{
  LFDS720_PAL_ASSERT( smrhps != NULL );
  // TRD : query_type can be any value in its range

  LFDS720_MISC_BARRIER_LOAD;

  switch( query_type )
  {
    case LFDS720_SMRHP_QUERY_SINGLETHREADED_COUNT_THREAD_STATES:
    {
      struct lfds720_list_nsu_element
        *asle = NULL;

      struct lfds720_smrhp_per_thread_state
        *smrhpts;

      struct lfds720_smrhp_per_thread_state_counts
        *smrhptsc;

      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      smrhptsc = (struct lfds720_smrhp_per_thread_state_counts *) query_output;

      smrhptsc->available_states = 0;
      smrhptsc->active_states = 0;
      smrhptsc->retired_states = 0;

      while( LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrhps->list_of_smrhp_per_thread_states, asle) )
      {
        smrhpts = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *asle );

        switch( smrhpts->status )
        {
          case LFDS720_SMRHPPTS_STATE_AVAILABLE:
            smrhptsc->available_states++;
          break;

          case LFDS720_SMRHPPTS_STATE_ACTIVE:
            smrhptsc->active_states++;
          break;

          case LFDS720_SMRHPPTS_STATE_RETIRED:
          case LFDS720_SMRHPPTS_STATE_RETIRING:
            smrhptsc->retired_states++;
          break;
        }
      }
    }
    break;

    case LFDS720_SMRHP_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_smrhp_internal_validate( smrhps, (struct lfds720_smrhp_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
void lfds720_smrhp_thread_query( struct lfds720_smrhp_per_thread_state *smrhpts, enum lfds720_smrhp_thread_query query_type, void *query_input, void *query_output )
{
  LFDS720_PAL_ASSERT( smrhpts != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_SMRHP_THREAD_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_smrhp_internal_thread_validate( smrhpts, (struct lfds720_smrhp_thread_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_smrhp_internal_thread_validate( struct lfds720_smrhp_per_thread_state *smrhpts, struct lfds720_smrhp_thread_validation_info *smrhptvi, enum lfds720_misc_validity *lfds720_smrhp_thread_validity )
{
  lfds720_pal_uint_t
    loop;

  LFDS720_PAL_ASSERT( smrhpts != NULL );
  LFDS720_PAL_ASSERT( smrhptvi != NULL );
  LFDS720_PAL_ASSERT( lfds720_smrhp_thread_validity != NULL );

  *lfds720_smrhp_thread_validity = LFDS720_MISC_VALIDITY_VALID;

  if( smrhptvi->check_expected_status_flag == LFDS720_MISC_FLAG_RAISED )
    if( smrhptvi->expected_status != smrhpts->status )
      *lfds720_smrhp_thread_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;

  if( *lfds720_smrhp_thread_validity == LFDS720_MISC_VALIDITY_VALID and smrhptvi->check_expected_numa_node_id_flag == LFDS720_MISC_FLAG_RAISED )
    if( smrhptvi->expected_numa_node_id != smrhpts->numa_node_id )
      *lfds720_smrhp_thread_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;

  if( *lfds720_smrhp_thread_validity == LFDS720_MISC_VALIDITY_VALID and smrhptvi->check_expected_expected_number_of_pending_reclamations_flag == LFDS720_MISC_FLAG_RAISED )
    stds_list_du_query( &smrhpts->list_of_allocations_pending_reclamation, STDS_LIST_DU_QUERY_VALIDATE, &smrhptvi->expected_number_of_pending_reclamations, lfds720_smrhp_thread_validity );

  if( *lfds720_smrhp_thread_validity == LFDS720_MISC_VALIDITY_VALID and smrhptvi->check_expected_hazard_pointers_flag == LFDS720_MISC_FLAG_RAISED )
    for( loop = 0 ; loop < LFDS720_SMRHP_SMRHPPTS_NUMBER_HAZARD_POINTERS ; loop++ )
      if( smrhptvi->expected_hazard_pointers[loop] != smrhpts->hazard_pointers[loop] )
        *lfds720_smrhp_thread_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;

  return;
}





/****************************************************************************/
static void lfds720_smrhp_internal_validate( struct lfds720_smrhp_state *smrhps, struct lfds720_smrhp_validation_info *smrvi, enum lfds720_misc_validity *lfds720_smrhp_validity )
{
  struct lfds720_smrhp_per_thread_state_counts
    local_smrhptsc;

  LFDS720_PAL_ASSERT( smrhps != NULL );
  LFDS720_PAL_ASSERT( smrvi != NULL );
  LFDS720_PAL_ASSERT( lfds720_smrhp_validity != NULL );

  *lfds720_smrhp_validity = LFDS720_MISC_VALIDITY_VALID;

  if( smrvi->check_expected_smrhptsc_flag == LFDS720_MISC_FLAG_RAISED )
  {
    lfds720_smrhp_query( smrhps, LFDS720_SMRHP_QUERY_SINGLETHREADED_COUNT_THREAD_STATES, NULL, &local_smrhptsc );

    if( 0 == lfds720_smrhp_internal_compare_struct_lfds720_smrhp_per_thread_state_counts(&local_smrhptsc, &smrvi->expected_smrhptsc) )
      *lfds720_smrhp_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;
  }

  return;
}





/****************************************************************************/
static int lfds720_smrhp_internal_compare_struct_lfds720_smrhp_per_thread_state_counts( struct lfds720_smrhp_per_thread_state_counts *s1, struct lfds720_smrhp_per_thread_state_counts *s2 )
{
  LFDS720_PAL_ASSERT( s1 != NULL );
  LFDS720_PAL_ASSERT( s2 != NULL );

  if( s1->available_states == s2->available_states and
      s1->active_states == s2->active_states and
      s1->retired_states == s2->retired_states )
    return( 1 );

  return( 0 );
}

