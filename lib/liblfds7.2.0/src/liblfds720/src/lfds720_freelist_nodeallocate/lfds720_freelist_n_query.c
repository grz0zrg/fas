/***** includes *****/
#include "lfds720_freelist_n_internal.h"

/***** private prototypes *****/
static void lfds720_freelist_n_internal_freelist_n_validate( struct lfds720_freelist_n_state *fs,
                                                             struct lfds720_misc_validation_info *vi,
                                                             enum lfds720_misc_validity *lfds720_freelist_n_validity );





/****************************************************************************/
void lfds720_freelist_n_query( struct lfds720_freelist_n_state *fs,
                               enum lfds720_freelist_n_query query_type,
                               void *query_input,
                               void *query_output )
{
  struct lfds720_freelist_n_element
    *fe;

  LFDS720_PAL_ASSERT( fs != NULL );
  // TRD : query_type can be any value in its range

  LFDS720_MISC_BARRIER_LOAD;

  switch( query_type )
  {
    case LFDS720_FREELIST_N_QUERY_SINGLETHREADED_GET_COUNT:
    {
      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      // TRD : count the elements on the freelist_n
      fe = (struct lfds720_freelist_n_element *) fs->top[LFDS720_MISC_POINTER];

      while( fe != NULL )
      {
        ( *(lfds720_pal_uint_t *) query_output )++;
        fe = (struct lfds720_freelist_n_element *) fe->next;
      }
    }
    break;

    case LFDS720_FREELIST_N_QUERY_SINGLETHREADED_GET_COUNT_INCLUDING_ELIMINATION_ARRAY:
    {
      lfds720_pal_uint_t
        loop,
        subloop;

      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      // TRD : count the elements in the elimination array
      for( loop = 0 ; loop < fs->elimination_array_number_of_lines ; loop++ )
        for( subloop = 0 ; subloop < LFDS720_FREELIST_N_ELIMINATION_ARRAY_USED_LINE_LENGTH_IN_FREELIST_N_POINTER_ELEMENTS ; subloop++ )
          if( fs->elimination_array[loop][subloop] != NULL )
            ( *(lfds720_pal_uint_t *) query_output )++;

      // TRD : count the elements on the freelist_n
      fe = (struct lfds720_freelist_n_element *) fs->top[LFDS720_MISC_POINTER];

      while( fe != NULL )
      {
        ( *(lfds720_pal_uint_t *) query_output )++;
        fe = (struct lfds720_freelist_n_element *) fe->next;
      }
    }
    break;

    case LFDS720_FREELIST_N_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_freelist_n_internal_freelist_n_validate( fs, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_freelist_n_internal_freelist_n_validate( struct lfds720_freelist_n_state *fs,
                                                         struct lfds720_misc_validation_info *vi,
                                                         enum lfds720_misc_validity *lfds720_freelist_n_validity )
{
  lfds720_pal_uint_t
    number_elements = 0;

  struct lfds720_freelist_n_element
    *fe_slow,
    *fe_fast;

  LFDS720_PAL_ASSERT( fs != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_freelist_n_validity != NULL );

  *lfds720_freelist_n_validity = LFDS720_MISC_VALIDITY_VALID;

  fe_slow = fe_fast = (struct lfds720_freelist_n_element *) fs->top[LFDS720_MISC_POINTER];

  /* TRD : first, check for a loop
           we have two pointers
           both of which start at the top of the freelist_n
           we enter a loop
           and on each iteration
           we advance one pointer by one element
           and the other by two

           we exit the loop when both pointers are NULL
           (have reached the end of the freelist_n)

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
    *lfds720_freelist_n_validity = LFDS720_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_freelist_n_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_freelist_n_query( fs, LFDS720_FREELIST_N_QUERY_SINGLETHREADED_GET_COUNT_INCLUDING_ELIMINATION_ARRAY, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_freelist_n_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_freelist_n_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

