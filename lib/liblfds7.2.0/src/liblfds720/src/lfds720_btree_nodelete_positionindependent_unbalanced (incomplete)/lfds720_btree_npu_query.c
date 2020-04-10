/***** includes *****/
#include "lfds720_btree_npu_internal.h"

/***** private prototypes *****/
static void lfds720_btree_npu_internal_validate( struct lfds720_btree_npu_state *abs, struct lfds720_misc_validation_info *vi, enum lfds720_misc_validity *lfds720_btree_npu_validity );





/****************************************************************************/
void lfds720_btree_npu_query( struct lfds720_btree_npu_state *baus,
                             enum lfds720_btree_npu_query query_type,
                             void *query_input,
                             void *query_output )
{
  LFDS720_PAL_ASSERT( baus != NULL );
  // TRD : query_type can be any value in its range

  LFDS720_MISC_BARRIER_LOAD;

  switch( query_type )
  {
    case LFDS720_BTREE_NPU_QUERY_GET_POTENTIALLY_INACCURATE_COUNT:
    {
      struct lfds720_btree_npu_element
        *baue = NULL;

      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      while( lfds720_btree_npu_get_by_absolute_position_and_then_by_relative_position(baus, &baue, LFDS720_BTREE_NPU_ABSOLUTE_POSITION_SMALLEST_IN_TREE, LFDS720_BTREE_NPU_RELATIVE_POSITION_NEXT_LARGER_ELEMENT_IN_ENTIRE_TREE) )
        ( *(lfds720_pal_uint_t *) query_output )++;
    }
    break;

    case LFDS720_BTREE_NPU_QUERY_SINGLETHREADED_VALIDATE:
      // TRD : query_input can be NULL
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_btree_npu_internal_validate( baus, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_btree_npu_internal_validate( struct lfds720_btree_npu_state *baus,
                                                struct lfds720_misc_validation_info *vi,
                                                enum lfds720_misc_validity *lfds720_btree_npu_validity )
{
  lfds720_pal_uint_t
    number_elements_from_query_tree = 0,
    number_elements_from_walk = 0;

  struct lfds720_btree_npu_element
    *baue = NULL,
    *baue_prev = NULL;

  LFDS720_PAL_ASSERT( baus!= NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_btree_npu_validity != NULL );

  *lfds720_btree_npu_validity = LFDS720_MISC_VALIDITY_VALID;

  /* TRD : validation is performed by;

           performing an in-order walk
           we should see every element is larger than the preceeding element
           we count elements as we go along (visited elements, that is)
           and check our tally equals the expected count
  */

  LFDS720_MISC_BARRIER_LOAD;

  while( lfds720_btree_npu_get_by_absolute_position_and_then_by_relative_position(baus, &baue, LFDS720_BTREE_NPU_ABSOLUTE_POSITION_SMALLEST_IN_TREE, LFDS720_BTREE_NPU_RELATIVE_POSITION_NEXT_LARGER_ELEMENT_IN_ENTIRE_TREE) )
  {
    // TRD : baue_prev should always be smaller than or equal to baue
    if( baue_prev != NULL )
      if( baus->key_compare_function(baue_prev->key, baue->key) > 0 )
      {
        *lfds720_btree_npu_validity = LFDS720_MISC_VALIDITY_INVALID_ORDER;
        return;
      }

    baue_prev = baue;
    number_elements_from_walk++;
  }

  if( *lfds720_btree_npu_validity == LFDS720_MISC_VALIDITY_VALID )
  {
    lfds720_btree_npu_query( (struct lfds720_btree_npu_state *) baus, LFDS720_BTREE_NPU_QUERY_GET_POTENTIALLY_INACCURATE_COUNT, NULL, &number_elements_from_query_tree );

    if( number_elements_from_walk > number_elements_from_query_tree )
      *lfds720_btree_npu_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;

    if( number_elements_from_walk < number_elements_from_query_tree )
      *lfds720_btree_npu_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;
  }

  /* TRD : now check for expected number of elements
           vi can be NULL, in which case we do not check
           we know we don't have a loop from our earlier check
  */

  if( *lfds720_btree_npu_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
  {
    lfds720_btree_npu_query( (struct lfds720_btree_npu_state *) baus, LFDS720_BTREE_NPU_QUERY_GET_POTENTIALLY_INACCURATE_COUNT, NULL, &number_elements_from_query_tree );

    if( number_elements_from_query_tree < vi->min_elements )
      *lfds720_btree_npu_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    if( number_elements_from_query_tree > vi->max_elements )
      *lfds720_btree_npu_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
  }

  return;
}

