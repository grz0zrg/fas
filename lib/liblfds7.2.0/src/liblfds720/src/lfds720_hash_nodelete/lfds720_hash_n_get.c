/***** includes *****/
#include "lfds720_hash_n_internal.h"





/****************************************************************************/
int lfds720_hash_n_get_by_key( struct lfds720_hash_n_state *has,
                               int (*key_compare_function)(void const *new_key, void const *existing_key),
                               void (*key_hash_function)(void const *key, lfds720_pal_uint_t *hash),
                               void *key,
                               struct lfds720_hash_n_element **hae )
{
  int
    rv;

  lfds720_pal_uint_t
    hash = 0;

  struct lfds720_btree_nu_element
    *baue;

  LFDS720_PAL_ASSERT( has != NULL );
  // TRD : key_compare_function can be NULL
  // TRD : key_hash_function can be NULL
  // TRD : key can be NULL
  LFDS720_PAL_ASSERT( hae != NULL );

  if( key_compare_function == NULL )
    key_compare_function = has->key_compare_function;

  if( key_hash_function == NULL )
    key_hash_function = has->key_hash_function;

  key_hash_function( key, &hash );

  rv = lfds720_btree_nu_get_by_key( has->baus_array + (hash % has->array_size), key_compare_function, key, &baue );

  if( rv == 1 )
    *hae = LFDS720_BTREE_NU_GET_VALUE_FROM_ELEMENT( *baue );
  else
    *hae = NULL;

  return rv;
}

