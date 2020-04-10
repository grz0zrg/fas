/***** includes *****/
#include "lfds720_smrg_internal.h"

/***** private prototypes *****/
static void lfds720_smrg_internal_thread_validate( struct lfds720_smrg_thread_state *smrgts, struct lfds720_smrg_thread_validation_info *smrtvi, enum lfds720_misc_validity *lfds720_smrg_thread_validity );
static void lfds720_smrg_internal_validate( struct lfds720_smrg_state *smrgs, struct lfds720_smrg_validation_info *smrvi, enum lfds720_misc_validity *lfds720_smrg_validity );
static int lfds720_smrg_internal_compare_struct_lfds720_smrg_thread_state_counts( struct lfds720_smrg_thread_state_counts *s1, struct lfds720_smrg_thread_state_counts *s2 );





/****************************************************************************/
void lfds720_smrg_query( struct lfds720_smrg_state *smrgs, enum lfds720_smrg_query query_type, void *query_input, void *query_output )
{
  LFDS720_PAL_ASSERT( smrgs != NULL );
  // TRD : query_type can be any value in its range

  LFDS720_MISC_BARRIER_LOAD;

  switch( query_type )
  {
    case LFDS720_SMRG_QUERY_SINGLETHREADED_COUNT_THREAD_STATES:
    {
      struct lfds720_list_nsu_element
        *asle = NULL;

      struct lfds720_smrg_thread_state
        *smrgts;

      struct lfds720_smrg_thread_state_counts
        *smrgtsc;

      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      smrgtsc = (struct lfds720_smrg_thread_state_counts *) query_output;

      smrgtsc->available_states = 0;
      smrgtsc->active_new_states = 0;
      smrgtsc->active_states = 0;
      smrgtsc->retired_states = 0;

      while( LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT(smrgs->smrg_thread_states, asle) )
      {
        smrgts = LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( *asle );

        switch( smrgts->generation_count_and_status_flags & LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_REDUCED_MASK )
        {
          case LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_AVAILABLE:
            smrgtsc->available_states++;
          break;

          case LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE_NEW:
            smrgtsc->active_new_states++;
          break;

          case LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_ACTIVE:
            smrgtsc->active_states++;
          break;

          case LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_RETIRED:
            smrgtsc->retired_states++;
          break;
        }
      }
    }
    break;

    case LFDS720_SMRG_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_smrg_internal_validate( smrgs, (struct lfds720_smrg_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
void lfds720_smrg_thread_query( struct lfds720_smrg_thread_state *smrgts, enum lfds720_smrg_thread_query query_type, void *query_input, void *query_output )
{
  LFDS720_PAL_ASSERT( smrgts != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_SMRG_THREAD_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_smrg_internal_thread_validate( smrgts, (struct lfds720_smrg_thread_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_smrg_internal_thread_validate( struct lfds720_smrg_thread_state *smrgts, struct lfds720_smrg_thread_validation_info *smrtvi, enum lfds720_misc_validity *lfds720_smrg_thread_validity )
{
  lfds720_pal_uint_t
    loop;

  LFDS720_PAL_ASSERT( smrgts != NULL );
  LFDS720_PAL_ASSERT( smrtvi != NULL );
  LFDS720_PAL_ASSERT( lfds720_smrg_thread_validity != NULL );

  *lfds720_smrg_thread_validity = LFDS720_MISC_VALIDITY_VALID;

  if( smrtvi->check_expected_generation_count_flag == LFDS720_MISC_FLAG_RAISED )
    if( smrtvi->expected_generation_count != LFDS720_SMRG_GET_GENERATION(smrgts->generation_count_and_status_flags) )
      *lfds720_smrg_thread_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;

  if( *lfds720_smrg_thread_validity == LFDS720_MISC_VALIDITY_VALID and smrtvi->check_expected_expected_number_of_allocations_flag == LFDS720_MISC_FLAG_RAISED )
    stds_list_du_query( &smrgts->allocations, STDS_LIST_DU_QUERY_VALIDATE, &smrtvi->expected_number_of_allocations, lfds720_smrg_thread_validity );

  if( smrtvi->check_expected_numa_node_id_flag == LFDS720_MISC_FLAG_RAISED )
    if( smrtvi->expected_numa_node_id != smrgts->numa_node_id )
      *lfds720_smrg_thread_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;

  for( loop = 0 ; loop < LFDS720_SMRG_SMRGTS_FLAG_THREAD_STATE_COUNT ; loop++ )
    if( smrtvi->check_expected_status_flags_bitmask & ((lfds720_pal_uint_t) 0x1 << loop) )
      if( (LFDS720_SMRG_GET_FLAGS(smrgts->generation_count_and_status_flags) & ((lfds720_pal_uint_t) 0x1 << loop)) != (smrtvi->expected_status_flags_bitmask & ((lfds720_pal_uint_t) 0x1 << loop)) )
        *lfds720_smrg_thread_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;

  return;
}





/****************************************************************************/
static void lfds720_smrg_internal_validate( struct lfds720_smrg_state *smrgs, struct lfds720_smrg_validation_info *smrvi, enum lfds720_misc_validity *lfds720_smrg_validity )
{
  lfds720_pal_uint_t
    loop,
    smrgs_flags,
    smrgs_generation;

  struct lfds720_smrg_thread_state_counts
    local_smrgtsc;

  LFDS720_PAL_ASSERT( smrgs != NULL );
  LFDS720_PAL_ASSERT( smrvi != NULL );
  LFDS720_PAL_ASSERT( lfds720_smrg_validity != NULL );

  *lfds720_smrg_validity = LFDS720_MISC_VALIDITY_VALID;

  if( smrvi->check_expected_generation_count_flag == LFDS720_MISC_FLAG_RAISED )
  {
    smrgs_generation = LFDS720_SMRG_GET_GENERATION( smrgs->generation_count_and_status_flags );
    smrgs_flags = LFDS720_SMRG_GET_FLAGS( smrgs->generation_count_and_status_flags );

    if( smrvi->expected_generation_count != smrgs_generation )
      *lfds720_smrg_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;
  }

  if( smrvi->check_expected_smrgtsc_flag == LFDS720_MISC_FLAG_RAISED )
  {
    lfds720_smrg_query( smrgs, LFDS720_SMRG_QUERY_SINGLETHREADED_COUNT_THREAD_STATES, NULL, &local_smrgtsc );

    if( 0 == lfds720_smrg_internal_compare_struct_lfds720_smrg_thread_state_counts(&local_smrgtsc, &smrvi->expected_smrgtsc) )
      *lfds720_smrg_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;
  }

  for( loop = 0 ; loop < LFDS720_SMRG_SMRGS_FLAG_STATUS_COUNT ; loop++ )
    if( smrvi->check_expected_status_flags_bitmask & ((lfds720_pal_uint_t) 0x1 << loop) )
      if( (LFDS720_SMRG_GET_FLAGS(smrgs->generation_count_and_status_flags) & ((lfds720_pal_uint_t) 0x1 << loop)) != (smrvi->expected_status_flags_bitmask & ((lfds720_pal_uint_t) 0x1 << loop)) )
        *lfds720_smrg_validity = LFDS720_MISC_VALIDITY_INVALID_TEST_DATA;

  return;
}





/****************************************************************************/
static int lfds720_smrg_internal_compare_struct_lfds720_smrg_thread_state_counts( struct lfds720_smrg_thread_state_counts *s1, struct lfds720_smrg_thread_state_counts *s2 )
{
  LFDS720_PAL_ASSERT( s1 != NULL );
  LFDS720_PAL_ASSERT( s2 != NULL );

  if( s1->available_states == s2->available_states and
      s1->active_new_states == s2->active_new_states and
      s1->active_states == s2->active_states and
      s1->retired_states == s2->retired_states )
    return( 1 );

  return( 0 );
}

