/***** includes *****/
#include "lfds720_hash_n_internal.h"

/***** private prototypes *****/
static void lfds720_hash_n_internal_validate( struct lfds720_hash_n_state *has,
                                              struct lfds720_misc_validation_info *vi,
                                              enum lfds720_misc_validity *lfds720_hash_n_validity );





/****************************************************************************/
void lfds720_hash_n_query( struct lfds720_hash_n_state *has,
                           enum lfds720_hash_n_query query_type,
                           void *query_input,
                           void *query_output )
{
  LFDS720_PAL_ASSERT( has != NULL );
  // TRD : query_type can be any value in its range

  LFDS720_MISC_BARRIER_LOAD;

  switch( query_type )
  {
    case LFDS720_HASH_N_QUERY_GET_POTENTIALLY_INACCURATE_COUNT:
    {
      struct lfds720_hash_n_iterate
        ai;

      struct lfds720_hash_n_element
        *hae;

      LFDS720_PAL_ASSERT( query_input == NULL );
      LFDS720_PAL_ASSERT( query_output != NULL );

      *(lfds720_pal_uint_t *) query_output = 0;

      lfds720_hash_n_iterate_init( has, &ai );

      while( lfds720_hash_n_iterate(&ai, &hae) )
        ( *(lfds720_pal_uint_t *) query_output )++;
    }
    break;

    case LFDS720_HASH_N_QUERY_SINGLETHREADED_VALIDATE:
      // TRD: query_input can be any value in its range
      LFDS720_PAL_ASSERT( query_output != NULL );

      lfds720_hash_n_internal_validate( has, (struct lfds720_misc_validation_info *) query_input, (enum lfds720_misc_validity *) query_output );
    break;
  }

  return;
}





/****************************************************************************/
static void lfds720_hash_n_internal_validate( struct lfds720_hash_n_state *has,
                                              struct lfds720_misc_validation_info *vi,
                                              enum lfds720_misc_validity *lfds720_hash_n_validity )
{
  lfds720_pal_uint_t
    lfds720_hash_n_total_number_elements = 0,
    lfds720_btree_nu_total_number_elements = 0,
    number_elements;

  lfds720_pal_uint_t
    loop;

  LFDS720_PAL_ASSERT( has!= NULL );
  // TRD : vi can be NULL
  LFDS720_PAL_ASSERT( lfds720_hash_n_validity != NULL );

  /* TRD : validate every btree_addonly_unbalanced in the addonly_hash
           sum elements in each btree_addonly_unbalanced
           check matches expected element counts (if vi is provided)
  */

  *lfds720_hash_n_validity = LFDS720_MISC_VALIDITY_VALID;

  for( loop = 0 ; *lfds720_hash_n_validity == LFDS720_MISC_VALIDITY_VALID and loop < has->array_size ; loop++ )
    lfds720_btree_nu_query( has->baus_array+loop, LFDS720_BTREE_NU_QUERY_SINGLETHREADED_VALIDATE, NULL, (void *) lfds720_hash_n_validity );

  if( *lfds720_hash_n_validity == LFDS720_MISC_VALIDITY_VALID )
  {
    for( loop = 0 ; loop < has->array_size ; loop++ )
    {
      lfds720_btree_nu_query( has->baus_array+loop, LFDS720_BTREE_NU_QUERY_GET_POTENTIALLY_INACCURATE_COUNT, NULL, (void *) &number_elements );
      lfds720_btree_nu_total_number_elements += number_elements;
    }

    // TRD : first, check btree_addonly_unbalanced total vs the addonly_hash total
    lfds720_hash_n_query( has, LFDS720_HASH_N_QUERY_GET_POTENTIALLY_INACCURATE_COUNT, NULL, &lfds720_hash_n_total_number_elements );

    // TRD : the btree_addonly_unbalanceds are assumed to speak the truth
    if( lfds720_hash_n_total_number_elements < lfds720_btree_nu_total_number_elements )
      *lfds720_hash_n_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;

    if( lfds720_hash_n_total_number_elements > lfds720_btree_nu_total_number_elements )
      *lfds720_hash_n_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

    // TRD : second, if we're still valid and vi is provided, check the btree_addonly_unbalanced total against vi
    if( *lfds720_hash_n_validity == LFDS720_MISC_VALIDITY_VALID and vi != NULL )
    {
      if( lfds720_btree_nu_total_number_elements < vi->min_elements )
        *lfds720_hash_n_validity = LFDS720_MISC_VALIDITY_INVALID_MISSING_ELEMENTS;

      if( lfds720_btree_nu_total_number_elements > vi->max_elements )
        *lfds720_hash_n_validity = LFDS720_MISC_VALIDITY_INVALID_ADDITIONAL_ELEMENTS;
    }
  }

  return;
}

