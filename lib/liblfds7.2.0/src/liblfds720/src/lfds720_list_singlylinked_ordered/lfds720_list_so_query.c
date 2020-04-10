/***** includes *****/
#include "lfds720_list_singlylinked_unordered_internal.h"

/***** private prototypes *****/
static void lfds720_list_so_internal_validate( struct lfds720_list_so_state *lsos,
                                                struct lfds720_misc_validation_info *vi,
                                                enum lfds720_misc_validity *lfds720_list_so_validity );





/****************************************************************************/
void lfds720_list_so_query( struct lfds720_list_so_state *lsos,
                             enum lfds720_list_so_query query_type,
                             void *query_input,
                             void *query_output )
{
  LFDS720_PAL_ASSERT( lsos != NULL );
  // TRD : query_type can be any value in its range

  LFDS720_MISC_BARRIER_LOAD;

  switch( query_type )
  {
    case LFDS720_LIST_SO_QUERY_GET_POTENTIALLY_INACCURATE_COUNT:
    {
      struct lfds720_list_so_element
        *lsoe = NULL;

      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      while( LFDS720_LIST_SO_GET_START_AND_THEN_NEXT(*lsos, lsoe) )
        ( *(lfds720_pal_uint_t *) query_output )++;
    }
    break;

    case LFDS720_LIST_SO_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_list_so_internal_validate( lsos, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}






/****************************************************************************/
static void lfds720_list_so_internal_validate( struct lfds720_list_so_state *lsos,
                                                struct lfds720_misc_validation_info *vi,
                                                enum lfds720_misc_validity *lfds720_list_so_validity )
{
  lfds720_pal_uint_t
    number_elements = 0;

  struct lfds720_list_so_element
    *lsoe_fast,
    *lsoe_slow;

  LFDS720_PAL_ASSERT( lsos!= NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_list_so_validity != NULL );

  *lfds720_list_so_validity = LFDS720_MISC_VALIDITY_VALID;

  lsoe_slow = lsoe_fast = lsos->start->next;

  /* TRD : first, check for a loop
           we have two pointers
           both of which start at the start of the list
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

  if( lsoe_slow != NULL )
    do
    {
      lsoe_slow = lsoe_slow->next;

      if( lsoe_fast != NULL )
        lsoe_fast = lsoe_fast->next;

      if( lsoe_fast != NULL )
        lsoe_fast = lsoe_fast->next;
    }
    while( lsoe_slow != NULL and lsoe_fast != lsoe_slow );

  if( lsoe_fast != NULL and lsoe_slow != NULL and lsoe_fast == lsoe_slow )
    *lfds720_list_so_validity = LFDS720_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_list_so_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_list_so_query( lsos, LFDS720_LIST_SO_QUERY_GET_POTENTIALLY_INACCURATE_COUNT, NULL, &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_list_so_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_list_so_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

