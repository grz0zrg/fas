/***** includes *****/
#include "lfds720_stack_np_internal.h"

/***** private prototypes *****/
static void lfds720_stack_np_internal_stack_np_validate( struct lfds720_stack_np_state *ss,
                                                               struct lfds720_misc_validation_info *vi,
                                                               enum lfds720_misc_validity *lfds720_stack_np_validity );





/****************************************************************************/
void lfds720_stack_np_query( struct lfds720_stack_np_state *ss,
                                enum lfds720_stack_np_query query_type,
                                void *query_input,
                                void *query_output )
{
  LFDS720_PAL_ASSERT( ss != NULL );
  // TRD : query_type can be any value in its range

  LFDS720_MISC_BARRIER_LOAD;

  switch( query_type )
  {
    case LFDS720_STACK_NP_QUERY_SINGLETHREADED_GET_COUNT:
    {
      ptrdiff_t
        se;

      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      // TRD : count the elements on the stack_np
      se = ss->top[LFDS720_MISC_OFFSET];

      while( se != 0 )
      {
        ( *(lfds720_pal_uint_t *) query_output )++;
        se = LFDS720_MISC_OFFSET_TO_POINTER( ss, se, struct lfds720_stack_np_element )->next;
      }
    }
    break;

    case LFDS720_STACK_NP_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_stack_np_internal_stack_np_validate( ss, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_stack_np_internal_stack_np_validate( struct lfds720_stack_np_state *ss,
                                                               struct lfds720_misc_validation_info *vi,
                                                               enum lfds720_misc_validity *lfds720_stack_np_validity )
{
  lfds720_pal_uint_t
    number_elements = 0;

  ptrdiff_t
    se_slow,
    se_fast;

  LFDS720_PAL_ASSERT( ss != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_stack_np_validity != NULL );

  *lfds720_stack_np_validity = LFDS720_MISC_VALIDITY_VALID;

  se_slow = se_fast = ss->top[LFDS720_MISC_OFFSET];

  /* TRD : first, check for a loop
           we have two pointers
           both of which start at the top of the stack_np
           we enter a loop
           and on each iteration
           we advance one pointer by one element
           and the other by two

           we exit the loop when both pointers are NULL
           (have reached the end of the stack_np)

           or

           if we fast pointer 'sees' the slow pointer
           which means we have a loop
  */

  if( se_slow != 0 )
    do
    {
      // se_slow = ( (struct lfds720_stack_np_element *) ( (void *) ss + se_slow ) )->next;
      se_slow = LFDS720_MISC_OFFSET_TO_POINTER( ss, se_slow, struct lfds720_stack_np_element )->next;

      if( se_fast != 0 )
        // se_fast = ( (struct lfds720_stack_np_element *) ( (void *) ss + se_fast ) )->next;
        se_fast = LFDS720_MISC_OFFSET_TO_POINTER( ss, se_fast, struct lfds720_stack_np_element )->next;

      if( se_fast != 0 )
        // se_fast = ( (struct lfds720_stack_np_element *) ( (void *) ss + se_fast ) )->next;
        se_fast = LFDS720_MISC_OFFSET_TO_POINTER( ss, se_fast, struct lfds720_stack_np_element )->next;
    }
    while( se_slow != 0 and se_fast != se_slow );

  if( se_fast != 0 and se_slow != 0 and se_fast == se_slow )
    *lfds720_stack_np_validity = LFDS720_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_stack_np_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_stack_np_query( ss, LFDS720_STACK_NP_QUERY_SINGLETHREADED_GET_COUNT, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_stack_np_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_stack_np_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

