/***** includes *****/
#include "lfds720_freelist_smrg_internal.h"

/***** private prototypes *****/
static void lfds720_freelist_smrg_internal_freelist_validate( struct lfds720_freelist_smrg_state *fsgs,
                                                       struct lfds720_misc_validation_info *vi,
                                                       enum lfds720_misc_validity *lfds720_freelist_smrg_validity );





/****************************************************************************/
void lfds720_freelist_smrg_query( struct lfds720_freelist_smrg_state *fsgs,
                          enum lfds720_freelist_smrg_query query_type,
                          void *query_input,
                          void *query_output )
{
  struct lfds720_freelist_smrg_element
    *fsge;

  LFDS720_MISC_BARRIER_LOAD;

  LFDS720_PAL_ASSERT( fsgs != NULL );
  // TRD : query_type can be any value in its range

  switch( query_type )
  {
    case LFDS720_FREELIST_SMRG_QUERY_SINGLETHREADED_GET_COUNT:
      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      fsge = (struct lfds720_freelist_smrg_element *) fsgs->top;

      while( fsge != NULL )
      {
        ( *(lfds720_pal_uint_t *) query_output )++;
        fsge = (struct lfds720_freelist_smrg_element *) fsge->next;
      }
    break;

    case LFDS720_FREELIST_SMRG_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_freelist_smrg_internal_freelist_validate( fsgs, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_freelist_smrg_internal_freelist_validate( struct lfds720_freelist_smrg_state *fsgs,
                                                       struct lfds720_misc_validation_info *vi,
                                                       enum lfds720_misc_validity *lfds720_freelist_smrg_validity )
{
  lfds720_pal_uint_t
    number_elements = 0;

  struct lfds720_freelist_smrg_element
    *fsge_fast,
    *fsge_slow;

  LFDS720_PAL_ASSERT( fsgs != NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_freelist_smrg_validity != NULL );

  *lfds720_freelist_smrg_validity = LFDS720_MISC_VALIDITY_VALID;

  fsge_slow = fsge_fast = (struct lfds720_freelist_smrg_element *) fsgs->top;

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

           if we fast pointer 'fsgees' the slow pointer
           which means we have a loop
  */

  if( fsge_slow != NULL )
    do
    {
      fsge_slow = fsge_slow->next;

      if( fsge_fast != NULL )
        fsge_fast = fsge_fast->next;

      if( fsge_fast != NULL )
        fsge_fast = fsge_fast->next;
    }
    while( fsge_slow != NULL and fsge_fast != fsge_slow );

  if( fsge_fast != NULL and fsge_slow != NULL and fsge_fast == fsge_slow )
    *lfds720_freelist_smrg_validity = LFDS720_MISC_VALIDITY_INVALID_LOOP;

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_freelist_smrg_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_freelist_smrg_query( fsgs, LFDS720_FREELIST_SMRG_QUERY_SINGLETHREADED_GET_COUNT, NULL, (void *) &number_elements );

    if( number_elements < vi->min_elements )
      *lfds720_freelist_smrg_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements > vi->max_elements )
      *lfds720_freelist_smrg_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

