/***** includes *****/
#include "lfds720_stack_n_internal.h"

/***** private prototypes *****/
static void lfds720_stack_n_internal_stack_n_validate( struct lfds720_stack_n_state *ss,
                                                   struct lfds720_misc_validation_info *vi,
                                                   enum lfds720_misc_validity *lfds720_stack_n_validity );





/****************************************************************************/
void lfds720_stack_n_query( struct lfds720_stack_n_state *ss,
                          enum lfds720_stack_n_query query_type,
                          void *query_input,
                          void *query_output )
{
  struct lfds720_stack_n_element
    *se;

  LFDS720_MISC_BARRIER_LOAD;

  LFDS720_PAL_ASSERT( ss != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_STACK_N_QUERY_SINGLETHREADED_GET_COUNT:
      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      se = (struct lfds720_stack_n_element *) ss->top[LFDS720_MISC_POINTER];

      while( se != NULL )
      {
        ( *(lfds720_pal_uint_t *) query_output )++;
        se = (struct lfds720_stack_n_element *) se->next;
      }
    break;

    case LFDS720_STACK_N_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_stack_n_internal_stack_n_validate( ss, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_stack_n_internal_stack_n_validate( struct lfds720_stack_n_state *ss,
                                                   struct lfds720_misc_validation_info *vi,
                                                   enum lfds720_misc_validity *lfds720_stack_n_validity )
{
  lfds720_pal_uint_t
    number_elements = 0;

  struct lfds720_stack_n_element
    *se_fast,
    *se_slow;

  LFDS720_PAL_ASSERT( ss != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_stack_n_validity != NULL );

  *lfds720_stack_n_validity = LFDS720_MISC_VALIDITY_VALID;

  se_slow = se_fast = (struct lfds720_stack_n_element *) ss->top[LFDS720_MISC_POINTER];

  /* TRD : first, check for a loop
           we have two pointers
           both of which start at the top of the stack_n
           we enter a loop
           and on each iteration
           we advance one pointer by one element
           and the other by two

           we exit the loop when both pointers are NULL
           (have reached the end of the stack_n)

           or

           if we fast pointer 'sees' the slow pointer
           which means we have a loop
  */

  if( se_slow != NULL )
    do
    {
      se_slow = se_slow->next;

      if( se_fast != NULL )
        se_fast = se_fast->next;

      if( se_fast != NULL )
        se_fast = se_fast->next;
    }
    while( se_slow != NULL and se_fast != se_slow );

  if( se_fast != NULL and se_slow != NULL and se_fast == se_slow )
    *lfds720_stack_n_validity = LFDS720_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_stack_n_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_stack_n_query( ss, LFDS720_STACK_N_QUERY_SINGLETHREADED_GET_COUNT, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_stack_n_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_stack_n_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

