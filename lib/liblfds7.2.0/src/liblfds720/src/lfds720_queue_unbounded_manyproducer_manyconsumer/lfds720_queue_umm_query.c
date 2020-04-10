/***** includes *****/
#include "lfds720_queue_umm_internal.h"

/***** private prototypes *****/
static void lfds720_queue_umm_internal_validate( struct lfds720_queue_umm_state *qummhps,
                                                       struct lfds720_misc_validation_info *vi,
                                                       enum lfds720_misc_validity *lfds720_queue_umm_validity );





/****************************************************************************/
void lfds720_queue_umm_query( struct lfds720_queue_umm_state *qummhps,
                                  enum lfds720_queue_umm_query query_type,
                                  void *query_input,
                                  void *query_output )
{
  struct lfds720_queue_umm_element
    *qummhpe;

  LFDS720_MISC_BARRIER_LOAD;

  LFDS720_PAL_ASSERT( qummhps != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_QUEUE_UMM_QUERY_SINGLETHREADED_GET_COUNT:
      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      qummhpe = (struct lfds720_queue_umm_element *) qummhps->dequeue;

      while( qummhpe != NULL )
      {
        ( *(lfds720_pal_uint_t *) query_output )++;
        qummhpe = (struct lfds720_queue_umm_element *) qummhpe->next;
      }

      // TRD : remember there is a dummy element in the queue
      ( *(lfds720_pal_uint_t *) query_output )--;
    break;

    case LFDS720_QUEUE_UMM_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_queue_umm_internal_validate( qummhps, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_queue_umm_internal_validate( struct lfds720_queue_umm_state *qummhps,
                                                       struct lfds720_misc_validation_info *vi,
                                                       enum lfds720_misc_validity *lfds720_queue_umm_validity )
{
  lfds720_pal_uint_t
    number_elements = 0;

  struct lfds720_queue_umm_element
    *qummhpe_fast,
    *qummhpe_slow;

  LFDS720_PAL_ASSERT( qummhps != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_queue_umm_validity != NULL );

  *lfds720_queue_umm_validity = LFDS720_MISC_VALIDITY_VALID;

  qummhpe_slow = qummhpe_fast = (struct lfds720_queue_umm_element *) qummhps->dequeue;

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

  if( qummhpe_slow != NULL )
    do
    {
      qummhpe_slow = qummhpe_slow->next;

      if( qummhpe_fast != NULL )
        qummhpe_fast = qummhpe_fast->next;

      if( qummhpe_fast != NULL )
        qummhpe_fast = qummhpe_fast->next;
    }
    while( qummhpe_slow != NULL and qummhpe_fast != qummhpe_slow );

  if( qummhpe_fast != NULL and qummhpe_slow != NULL and qummhpe_fast == qummhpe_slow )
    *lfds720_queue_umm_validity = LFDS720_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_queue_umm_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_queue_umm_query( qummhps, LFDS720_QUEUE_UMM_QUERY_SINGLETHREADED_GET_COUNT, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_queue_umm_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_queue_umm_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

