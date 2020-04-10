/***** includes *****/
#include "lfds720_hash_n_internal.h"

/***** private prototypes*****/
static void btree_nu_element_cleanup_function( struct lfds720_btree_nu_state *baus,
                                               struct lfds720_btree_nu_element *baue );





/****************************************************************************/
void lfds720_hash_n_cleanup( struct lfds720_hash_n_state *has,
                             void (*element_cleanup_callback)(struct lfds720_hash_n_state *has, struct lfds720_hash_n_element *hae) )
{
  lfds720_pal_uint_t
    loop;

  LFDS720_PAL_ASSERT( has != NULL );
  // TRD : element_cleanup_callback can be NULL

  if( element_cleanup_callback == NULL )
    return;

  LFDS720_MISC_BARRIER_LOAD;

  has->element_cleanup_callback = element_cleanup_callback;

  for( loop = 0 ; loop < has->array_size ; loop++ )
    lfds720_btree_nu_cleanup( has->baus_array+loop, btree_nu_element_cleanup_function );

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

static void btree_nu_element_cleanup_function( struct lfds720_btree_nu_state *baus,
                                               struct lfds720_btree_nu_element *baue )
{
  struct lfds720_hash_n_state
    *has;

  struct lfds720_hash_n_element
    *hae;

  LFDS720_PAL_ASSERT( baus != NULL );
  LFDS720_PAL_ASSERT( baue != NULL );

  hae = (struct lfds720_hash_n_element *) LFDS720_BTREE_NU_GET_VALUE_FROM_ELEMENT( *baue );
  has = (struct lfds720_hash_n_state *) LFDS720_BTREE_NU_GET_USER_STATE_FROM_STATE( *baus );

  has->element_cleanup_callback( has, hae );

  return;
}

#pragma warning( default : 4100 )

