/***** includes *****/
#include "lfds720_queue_bmm_internal.h"

/***** private prototypes *****/
static void lfds720_queue_bmm_internal_validate( struct lfds720_queue_bmm_state *qbmms,
                                                 struct lfds720_misc_validation_info *vi,
                                                 enum lfds720_misc_validity *lfds720_validity );





/****************************************************************************/
void lfds720_queue_bmm_query( struct lfds720_queue_bmm_state *qbmms,
                              enum lfds720_queue_bmm_query query_type,
                              void *query_input,
                              void *query_output )
{
  LFDS720_PAL_ASSERT( qbmms != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_QUEUE_BMM_QUERY_GET_POTENTIALLY_INACCURATE_COUNT:
    {
      lfds720_pal_uint_t
        local_read_index,
        local_write_index;

      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      LFDS720_MISC_BARRIER_LOAD;

      local_read_index = qbmms->read_index;
      local_write_index = qbmms->write_index;

      *(lfds720_pal_uint_t *) query_output = +( local_write_index - local_read_index );

      if( local_read_index > local_write_index )
        *(lfds720_pal_uint_t *) query_output = ((lfds720_pal_uint_t) -1) - *(lfds720_pal_uint_t *) query_output;
    }
    break;

    case LFDS720_QUEUE_BMM_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_queue_bmm_internal_validate( qbmms, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_queue_bmm_internal_validate( struct lfds720_queue_bmm_state *qbmms,
                                                 struct lfds720_misc_validation_info *vi,
                                                 enum lfds720_misc_validity *lfds720_validity )
{
  lfds720_pal_uint_t
    expected_sequence_number,
    loop,
    number_elements,
    sequence_number;

  LFDS720_PAL_ASSERT( qbmms != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_validity != NULL );

  *lfds720_validity = LFDS720_MISC_VALIDITY_VALID;

  /* TRD : starting from the read_index, we should find number_elements of incrementing sequence numbers
           we then perform a second scan from the write_index onwards, which should have (max elements in queue - number_elements) incrementing sequence numbers
  */

  lfds720_queue_bmm_query( qbmms, LFDS720_QUEUE_BMM_QUERY_GET_POTENTIALLY_INACCURATE_COUNT, NULL, (void *) &number_elements );

  expected_sequence_number = qbmms->element_array[ qbmms->read_index & qbmms->mask ].sequence_number;

  for( loop = 0 ; loop < number_elements ; loop++ )
  {
    sequence_number = qbmms->element_array[ (qbmms->read_index + loop) & qbmms->mask ].sequence_number;

    if( sequence_number != expected_sequence_number )
      *lfds720_validity = LFDS720_MISC_VALIDITY_INVALID_ORDER;

    if( sequence_number == expected_sequence_number )
      expected_sequence_number = sequence_number + 1;
  }

  // TRD : now the write_index onwards

  expected_sequence_number = qbmms->element_array[ qbmms->write_index & qbmms->mask ].sequence_number;

  for( loop = 0 ; loop < qbmms->number_elements - number_elements ; loop++ )
  {
    sequence_number = qbmms->element_array[ (qbmms->write_index + loop) & qbmms->mask ].sequence_number;

    if( sequence_number != expected_sequence_number )
      *lfds720_validity = LFDS720_MISC_VALIDITY_INVALID_ORDER;

    if( sequence_number == expected_sequence_number )
      expected_sequence_number = sequence_number + 1;
  }

  // TRD : now check against the expected number of elements

  if( *lfds720_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_pal_uint_t
      number_elements;

    lfds720_queue_bmm_query( qbmms, LFDS720_QUEUE_BMM_QUERY_GET_POTENTIALLY_INACCURATE_COUNT, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

