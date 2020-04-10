/***** includes *****/
#include "lfds720_queue_npumm_internal.h"

/***** private prototypes *****/
static void lfds720_queue_npumm_internal_validate( struct lfds720_queue_npumm_state *qumms,
                                                   struct lfds720_misc_validation_info *vi,
                                                   enum lfds720_misc_validity *lfds720_queue_npumm_validity );





/****************************************************************************/
void lfds720_queue_npumm_query( struct lfds720_queue_npumm_state *qumms,
                                enum lfds720_queue_npumm_query query_type,
                                void *query_input,
                                void *query_output )
{
  ptrdiff_t
    qumme;

  LFDS720_MISC_BARRIER_LOAD;

  LFDS720_PAL_ASSERT( qumms != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_QUEUE_NPUMM_QUERY_SINGLETHREADED_GET_COUNT:
      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      qumme = qumms->dequeue[LFDS720_MISC_OFFSET];

      while( qumme != 0 )
      {
        ( *(lfds720_pal_uint_t *) query_output )++;
        qumme = LFDS720_MISC_OFFSET_TO_POINTER( qumms, qumme, struct lfds720_queue_npumm_element )->next[LFDS720_MISC_OFFSET];
      }

      // TRD : remember there is a dummy element in the queue
      ( *(lfds720_pal_uint_t *) query_output )--;
    break;

    case LFDS720_QUEUE_NPUMM_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_queue_npumm_internal_validate( qumms, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_queue_npumm_internal_validate( struct lfds720_queue_npumm_state *qumms,
                                                   struct lfds720_misc_validation_info *vi,
                                                   enum lfds720_misc_validity *lfds720_queue_npumm_validity )
{
  lfds720_pal_uint_t
    number_elements = 0;

  ptrdiff_t
    qumme_fast,
    qumme_slow;

  LFDS720_PAL_ASSERT( qumms != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_queue_npumm_validity != NULL );

  *lfds720_queue_npumm_validity = LFDS720_MISC_VALIDITY_VALID;

  qumme_slow = qumme_fast = qumms->dequeue[LFDS720_MISC_OFFSET];

  /* TRD : first, check for a loop
           we have two pointers
           both of which start at the dequeue end of the queue
           we enter a loop
           and on each iteration
           we advance one pointer by one element
           and the other by two

           we exit the loop when both pointers are NULL
           (have reached the end of the queue)

           or

           if we fast pointer 'sees' the slow pointer
           which means we have a loop
  */

  if( qumme_slow != 0 )
    do
    {
      qumme_slow = LFDS720_MISC_OFFSET_TO_POINTER( qumms, qumme_slow, struct lfds720_queue_npumm_element )->next[LFDS720_MISC_OFFSET];

      if( qumme_fast != 0 )
        qumme_fast = LFDS720_MISC_OFFSET_TO_POINTER( qumms, qumme_fast, struct lfds720_queue_npumm_element )->next[LFDS720_MISC_OFFSET];

      if( qumme_fast != 0 )
        qumme_fast = LFDS720_MISC_OFFSET_TO_POINTER( qumms, qumme_fast, struct lfds720_queue_npumm_element )->next[LFDS720_MISC_OFFSET];
    }
    while( qumme_slow != 0 and qumme_fast != qumme_slow );

  if( qumme_fast != 0 and qumme_slow != 0 and qumme_fast == qumme_slow )
    *lfds720_queue_npumm_validity = LFDS720_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_queue_npumm_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_queue_npumm_query( qumms, LFDS720_QUEUE_NPUMM_QUERY_SINGLETHREADED_GET_COUNT, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_queue_npumm_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_queue_npumm_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

