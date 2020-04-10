/***** includes *****/
#include "lfds720_hash_n_internal.h"





/****************************************************************************/
void lfds720_hash_n_init_valid_on_current_logical_core( struct lfds720_hash_n_state *has,
                                                        struct lfds720_btree_nu_state *baus_array,
                                                        lfds720_pal_uint_t array_size,
                                                        int (*key_compare_function)(void const *new_key, void const *existing_key),
                                                        void (*key_hash_function)(void const *key, lfds720_pal_uint_t *hash),
                                                        enum lfds720_hash_n_existing_key existing_key,
                                                        void *user_state )
{
  enum lfds720_btree_nu_existing_key
    btree_nu_existing_key = LFDS720_BTREE_NU_EXISTING_KEY_OVERWRITE; // TRD : for compiler warning

  lfds720_pal_uint_t
    loop;

  LFDS720_PAL_ASSERT( has != NULL );
  LFDS720_PAL_ASSERT( baus_array != NULL );
  LFDS720_PAL_ASSERT( array_size > 0 );
  LFDS720_PAL_ASSERT( key_compare_function != NULL );
  LFDS720_PAL_ASSERT( key_hash_function != NULL );
  // TRD : existing_key can be any value in its range
  // TRD : user_state can be NULL

  has->array_size = array_size;
  has->key_compare_function = key_compare_function;
  has->key_hash_function = key_hash_function;
  has->existing_key = existing_key;
  has->baus_array = baus_array;
  has->user_state = user_state;

  if( has->existing_key == LFDS720_HASH_N_EXISTING_KEY_OVERWRITE )
    btree_nu_existing_key = LFDS720_BTREE_NU_EXISTING_KEY_OVERWRITE;

  if( has->existing_key == LFDS720_HASH_N_EXISTING_KEY_FAIL )
    btree_nu_existing_key = LFDS720_BTREE_NU_EXISTING_KEY_FAIL;

  // TRD : since the addonly_hash atomic counts, if that flag is set, the btree_addonly_unbalanceds don't have to
  for( loop = 0 ; loop < array_size ; loop++ )
    lfds720_btree_nu_init_valid_on_current_logical_core( has->baus_array+loop, key_compare_function, btree_nu_existing_key, user_state );

  LFDS720_MISC_BARRIER_STORE;

  lfds720_misc_force_store();

  return;
}

