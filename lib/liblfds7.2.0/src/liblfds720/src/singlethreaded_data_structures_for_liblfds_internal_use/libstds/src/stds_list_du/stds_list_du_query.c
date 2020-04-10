/***** includes *****/
#include "stds_list_du_internal.h"

/***** private prototypes *****/
static void stds_list_du_internal_validate( struct stds_list_du_state *ldus, struct stds_misc_validation_info *vi, enum stds_misc_validity *stds_list_validity );




/****************************************************************************/
void stds_list_du_query( struct stds_list_du_state *ldus, enum stds_list_du_query query_type, void *query_input, void *query_output )
{
  STDS_PAL_ASSERT( ldus != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case STDS_LIST_DU_QUERY_NUMBER_ELEMENTS:
    {
      // TRD : query_input can be NULL
      STDS_PAL_ASSERT( query_output != NULL );

      *(stds_pal_uint_t *) query_output = ldus->number_elements;
    }
    break;

    case STDS_LIST_DU_QUERY_VALIDATE:
      // TRD : query_input can be NULL
      STDS_PAL_ASSERT( query_output != NULL );

      stds_list_du_internal_validate( ldus, (struct stds_misc_validation_info *) query_input, (enum stds_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void stds_list_du_internal_validate( struct stds_list_du_state *ldus, struct stds_misc_validation_info *vi, enum stds_misc_validity *stds_list_validity )
{
  stds_pal_uint_t
    number_elements;

  struct stds_list_du_element
    *le_slow,
    *le_fast,
    *le_prev;

  STDS_PAL_ASSERT( ldus != NULL );
  // TRD : vi can be NULL
  STDS_PAL_ASSERT( stds_list_validity != NULL );

  *stds_list_validity = STDS_MISC_VALIDITY_VALID;

  /* TRD : check the validity of the start and end pointers in the list state
           additionally,
           the prev pointer of the start element should be NULL
           the next pointer of the end element should be NULL
  */

  if( ldus->start == NULL and ldus->end != NULL )
    *stds_list_validity = STDS_MISC_VALIDITY_INVALID_TEST_DATA;

  if( ldus->start != NULL and ldus->end == NULL )
    *stds_list_validity = STDS_MISC_VALIDITY_INVALID_TEST_DATA;

  if( ldus->start != NULL and ldus->start->prev != NULL )
    *stds_list_validity = STDS_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;

  if( ldus->end != NULL and ldus->end->next != NULL )
    *stds_list_validity = STDS_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;

  le_prev = NULL;
  le_slow = le_fast = ldus->start;

  /* TRD : first, check for a loop
           we have two pointers
           both of which start at the head of the list
           we enter a loop
           and on each iteration
           we advance one pointer by one element
           and the other by two

           we exit the loop when both pointers are NULL
           (have reached the end of the queue)

           or

           if we fast pointer 'sees' the slow pointer
           which means we have a loop

           as we traverse, always check our prev pointer
           points to the element we just came from
  */

  if( le_slow != NULL )
    do
    {
      if( le_slow->prev != le_prev )
      {
        *stds_list_validity = STDS_MISC_VALIDITY_INVALID_LOOP;
        break;
      }

      le_prev = le_slow;

      le_slow = le_slow->next;

      if( le_fast != NULL )
        le_fast = le_fast->next;

      if( le_fast != NULL )
        le_fast = le_fast->next;
    }
    while( le_slow != NULL and le_fast != le_slow );

  if( le_fast != NULL and le_slow != NULL and le_fast == le_slow )
    *stds_list_validity = STDS_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *stds_list_validity == STDS_MISC_VALIDITY_VALID and vi != NULL )
  {
    stds_list_du_query( ldus, STDS_LIST_DU_QUERY_NUMBER_ELEMENTS, NULL, &number_elements );

    if( number_elements < vi->min_elements )
      *stds_list_validity = STDS_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *stds_list_validity = STDS_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

