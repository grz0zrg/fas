/***** includes *****/
#include "lfds720_hash_n_internal.h"





/****************************************************************************/
enum lfds720_hash_n_insert_result lfds720_hash_n_insert( struct lfds720_hash_n_state *has,
                                                         struct lfds720_hash_n_element *hae,
                                                         struct lfds720_hash_n_element **existing_hae )
{
  enum lfds720_hash_n_insert_result
    apr = LFDS720_HASH_N_PUT_RESULT_SUCCESS;

  enum lfds720_btree_nu_insert_result
    alr;

  lfds720_pal_uint_t
    hash = 0;

  struct lfds720_btree_nu_element
    *existing_baue;

  LFDS720_PAL_ASSERT( has != NULL );
  LFDS720_PAL_ASSERT( hae != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &hae->value % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  // TRD : existing_hae can be NULL

  // TRD : alignment checks
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &hae->baue % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );

  has->key_hash_function( hae->key, &hash );

  LFDS720_BTREE_NU_SET_KEY_IN_ELEMENT( hae->baue, hae->key );
  LFDS720_BTREE_NU_SET_VALUE_IN_ELEMENT( hae->baue, hae );

  alr = lfds720_btree_nu_insert( has->baus_array + (hash % has->array_size), &hae->baue, &existing_baue );

  switch( alr )
  {
    case LFDS720_BTREE_NU_INSERT_RESULT_FAILURE_EXISTING_KEY:
      if( existing_hae != NULL )
        *existing_hae = LFDS720_BTREE_NU_GET_VALUE_FROM_ELEMENT( *existing_baue );

      apr = LFDS720_HASH_N_PUT_RESULT_FAILURE_EXISTING_KEY;
    break;

    case LFDS720_BTREE_NU_INSERT_RESULT_SUCCESS_OVERWRITE:
      apr = LFDS720_HASH_N_PUT_RESULT_SUCCESS_OVERWRITE;
    break;

    case LFDS720_BTREE_NU_INSERT_RESULT_SUCCESS:
      apr = LFDS720_HASH_N_PUT_RESULT_SUCCESS;
    break;
  }

  return apr;
}

