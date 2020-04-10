/***** includes *****/
#include "lfds720_queue_nuss_internal.h"

/***** private prototypes *****/
static void lfds720_queue_nuss_internal_validate( struct lfds720_queue_nuss_state *qusss,
                                                 struct lfds720_misc_validation_info *vi,
                                                 enum lfds720_misc_validity *lfds720_validity );





/****************************************************************************/
void lfds720_queue_nuss_query( struct lfds720_queue_nuss_state *qusss,
                              enum lfds720_queue_nuss_query query_type,
                              void *query_input,
                              void *query_output )
{
  LFDS720_PAL_ASSERT( qusss != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_QUEUE_NUSS_QUERY_GET_POTENTIALLY_INACCURATE_COUNT:
    {
      struct lfds720_queue_nuss_element
        *qusse;

      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      LFDS720_MISC_BARRIER_LOAD;

      qusse = qusss->dequeue_reader_writes;

      while( qusse->next != NULL )
      {
        ( *(lfds720_pal_uint_t *) query_output )++;
        qusse = qusse->next;
      }
    }
    break;

    case LFDS720_QUEUE_NUSS_QUERY_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_queue_nuss_internal_validate( qusss, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_queue_nuss_internal_validate( struct lfds720_queue_nuss_state *qusss,
                                                 struct lfds720_misc_validation_info *vi,
                                                 enum lfds720_misc_validity *lfds720_validity )
{
  lfds720_pal_uint_t
    number_elements = 0;

  struct lfds720_queue_nuss_element
    *qusse_fast,
    *qusse_slow;

  LFDS720_PAL_ASSERT( qusss != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_validity != NULL );

  *lfds720_validity = LFDS720_MISC_VALIDITY_VALID;

  qusse_slow = qusse_fast = qusss->dequeue_writer_writes;

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

  if( qusse_slow != NULL )
    do
    {
      qusse_slow = qusse_slow->next;

      if( qusse_fast != NULL )
        qusse_fast = qusse_fast->next;

      if( qusse_fast != NULL )
        qusse_fast = qusse_fast->next;
    }
    while( qusse_slow != NULL and qusse_fast != qusse_slow );

  if( qusse_fast != NULL and qusse_slow != NULL and qusse_fast == qusse_slow )
    *lfds720_validity = LFDS720_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_queue_nuss_query( qusss, LFDS720_QUEUE_NUSS_QUERY_GET_POTENTIALLY_INACCURATE_COUNT, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

