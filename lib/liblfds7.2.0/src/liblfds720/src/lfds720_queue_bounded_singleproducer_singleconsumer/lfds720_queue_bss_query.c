/***** includes *****/
#include "lfds720_queue_bss_internal.h"

/***** private prototypes *****/
static void lfds720_queue_bss_internal_validate( struct lfds720_queue_bss_state *qbsss,
                                                 struct lfds720_misc_validation_info *vi,
                                                 enum lfds720_misc_validity *lfds720_validity );





/****************************************************************************/
void lfds720_queue_bss_query( struct lfds720_queue_bss_state *qbsss,
                              enum lfds720_queue_bss_query query_type,
                              void *query_input,
                              void *query_output )
{
  LFDS720_PAL_ASSERT( qbsss != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_QUEUE_BSS_QUERY_GET_POTENTIALLY_INACCURATE_COUNT:
    {
      lfds720_pal_uint_t
        local_read_index,
        local_write_index;

      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      LFDS720_MISC_BARRIER_LOAD;

      local_read_index = qbsss->read_index;
      local_write_index = qbsss->write_index;

      *(lfds720_pal_uint_t *) query_output = +( local_write_index - local_read_index );

      if( local_read_index > local_write_index )
        *(lfds720_pal_uint_t *) query_output = qbsss->number_elements - *(lfds720_pal_uint_t *) query_output;
    }
    break;

    case LFDS720_QUEUE_BSS_QUERY_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_queue_bss_internal_validate( qbsss, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_queue_bss_internal_validate( struct lfds720_queue_bss_state *qbsss,
                                                 struct lfds720_misc_validation_info *vi,
                                                 enum lfds720_misc_validity *lfds720_validity )
{
  LFDS720_PAL_ASSERT( qbsss != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_validity != NULL );

  *lfds720_validity = LFDS720_MISC_VALIDITY_VALID;

  if( vi != NULL )
  {
    lfds720_pal_uint_t
      number_elements;

    lfds720_queue_bss_query( qbsss, LFDS720_QUEUE_BSS_QUERY_GET_POTENTIALLY_INACCURATE_COUNT, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

