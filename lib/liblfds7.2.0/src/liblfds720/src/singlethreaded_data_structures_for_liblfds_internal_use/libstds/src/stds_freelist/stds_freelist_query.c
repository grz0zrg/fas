/***** includes *****/
#include "stds_freelist_internal.h"

/***** private prototypes *****/
static void stds_freelist_internal_freelist_validate( struct stds_freelist_state *fs, struct stds_misc_validation_info *vi, enum stds_misc_validity *stds_freelist_validity );





/****************************************************************************/
void stds_freelist_query( struct stds_freelist_state *fs, enum stds_freelist_query_type query_type, void *query_input, void *query_output )
{
  struct stds_freelist_element
    *fe;

  STDS_PAL_ASSERT( fs != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case STDS_FREELIST_QUERY_NUMBER_ELEMENTS:
      STDS_PAL_ASSERT( query_input == NULL );
      STDS_PAL_ASSERT( query_output != NULL );

      *(stds_pal_uint_t *) query_output = 0;

      fe = (struct stds_freelist_element *) fs->top;

      while( fe != NULL )
      {
        ( *(stds_pal_uint_t *) query_output )++;
        fe = (struct stds_freelist_element *) fe->next;
      }
    break;

    case STDS_FREELIST_QUERY_VALIDATE:
      // TRD : query_input can be NULL
      STDS_PAL_ASSERT( query_output != NULL );

      stds_freelist_internal_freelist_validate( fs, (struct stds_misc_validation_info *) query_input, (enum stds_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void stds_freelist_internal_freelist_validate( struct stds_freelist_state *fs, struct stds_misc_validation_info *vi, enum stds_misc_validity *stds_freelist_validity )
{
  struct stds_freelist_element
    *fe_slow,
    *fe_fast;

  stds_pal_uint_t
    number_elements;

  STDS_PAL_ASSERT( fs != NULL );
  // TRD : vi can be NULL
  STDS_PAL_ASSERT( stds_freelist_validity != NULL );

  *stds_freelist_validity = STDS_MISC_VALIDITY_VALID;

  fe_slow = fe_fast = (struct stds_freelist_element *) fs->top;

  /* TRD : first, check for a loop
           we have two pointers
           both of which start at the top of the freelist
           we enter a loop
           and on each iteration
           we advance one pointer by one element
           and the other by two

           we exit the loop when both pointers are NULL
           (have reached the end of the freelist)

           or

           if we fast pointer 'sees' the slow pointer
           which means we have a loop
  */

  if( fe_slow != NULL )
    do
    {
      fe_slow = fe_slow->next;

      if( fe_fast != NULL )
        fe_fast = fe_fast->next;

      if( fe_fast != NULL )
        fe_fast = fe_fast->next;
    }
    while( fe_slow != NULL and fe_fast != fe_slow );

  if( fe_fast != NULL and fe_slow != NULL and fe_fast == fe_slow )
    *stds_freelist_validity = STDS_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *stds_freelist_validity == STDS_MISC_VALIDITY_VALID and vi != NULL )
  {
    stds_freelist_query( fs, STDS_FREELIST_QUERY_NUMBER_ELEMENTS, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *stds_freelist_validity = STDS_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *stds_freelist_validity = STDS_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

