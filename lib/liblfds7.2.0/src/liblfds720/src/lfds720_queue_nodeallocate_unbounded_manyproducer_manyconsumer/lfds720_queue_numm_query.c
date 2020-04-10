/***** includes *****/
#include "lfds720_queue_numm_internal.h"

/***** private prototypes *****/
static void lfds720_queue_numm_internal_validate( struct lfds720_queue_numm_state *qumms,
                                                 struct lfds720_misc_validation_info *vi,
                                                 enum lfds720_misc_validity *lfds720_queue_numm_validity );





/****************************************************************************/
void lfds720_queue_numm_query( struct lfds720_queue_numm_state *qumms,
                               enum lfds720_queue_numm_query query_type,
                               void *query_input,
                               void *query_output )
{
  struct lfds720_queue_numm_element
    *qumme;

  LFDS720_MISC_BARRIER_LOAD;

  LFDS720_PAL_ASSERT( qumms != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_QUEUE_NUMM_QUERY_SINGLETHREADED_GET_COUNT:
      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      qumme = (struct lfds720_queue_numm_element *) qumms->dequeue[LFDS720_MISC_POINTER];

      while( qumme != NULL )
      {
        ( *(lfds720_pal_uint_t *) query_output )++;
        qumme = (struct lfds720_queue_numm_element *) qumme->next[LFDS720_MISC_POINTER];
      }

      // TRD : remember there is a dummy element in the queue
      ( *(lfds720_pal_uint_t *) query_output )--;
    break;

    case LFDS720_QUEUE_NUMM_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_queue_numm_internal_validate( qumms, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_queue_numm_internal_validate( struct lfds720_queue_numm_state *qumms,
                                                 struct lfds720_misc_validation_info *vi,
                                                 enum lfds720_misc_validity *lfds720_queue_numm_validity )
{
  lfds720_pal_uint_t
    number_elements = 0;

  struct lfds720_queue_numm_element
    *qumme_fast,
    *qumme_slow;

  LFDS720_PAL_ASSERT( qumms != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_queue_numm_validity != NULL );

  *lfds720_queue_numm_validity = LFDS720_MISC_VALIDITY_VALID;

  qumme_slow = qumme_fast = (struct lfds720_queue_numm_element *) qumms->dequeue[LFDS720_MISC_POINTER];

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

  if( qumme_slow != NULL )
    do
    {
      qumme_slow = qumme_slow->next[LFDS720_MISC_POINTER];

      if( qumme_fast != NULL )
        qumme_fast = qumme_fast->next[LFDS720_MISC_POINTER];

      if( qumme_fast != NULL )
        qumme_fast = qumme_fast->next[LFDS720_MISC_POINTER];
    }
    while( qumme_slow != NULL and qumme_fast != qumme_slow );

  if( qumme_fast != NULL and qumme_slow != NULL and qumme_fast == qumme_slow )
    *lfds720_queue_numm_validity = LFDS720_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_queue_numm_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_queue_numm_query( qumms, LFDS720_QUEUE_NUMM_QUERY_SINGLETHREADED_GET_COUNT, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_queue_numm_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_queue_numm_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

