/***** includes *****/
#include "lfds720_btree_nu_internal.h"





/****************************************************************************/
enum lfds720_btree_nu_insert_result lfds720_btree_nu_insert( struct lfds720_btree_nu_state *baus,
                                                             struct lfds720_btree_nu_element *baue,
                                                             struct lfds720_btree_nu_element **existing_baue )
{
  char unsigned 
    result = 0;

  int
    compare_result = 0;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  struct lfds720_btree_nu_element
    *compare = NULL,
    *volatile baue_next = NULL,
    *volatile baue_parent = NULL,
    *volatile baue_temp;

  LFDS720_PAL_ASSERT( baus != NULL );
  LFDS720_PAL_ASSERT( baue != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &baue->left % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &baue->right % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &baue->up % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &baue->value % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  // TRD : existing_baue can be NULL

  /* TRD : we follow a normal search for the insert node and which side to insert

           the difference is that insertion may fail because someone else inserts
           there before we do

           in this case, we resume searching for the insert node from the node
           we were attempting to insert upon

           (if we attempted to insert the root node and this failed, i.e. we thought
            the tree was empty but then it wasn't, then we start searching from the
            new root)
  */

  baue->up = baue->left = baue->right = NULL;

  LFDS720_MISC_BARRIER_LOAD;

  baue_temp = baus->root;

  LFDS720_MISC_BARRIER_LOAD;

  while( result == 0 )
  {
    // TRD : first we find where to insert
    while( baue_temp != NULL )
    {
      compare_result = baus->key_compare_function( baue->key, baue_temp->key );

      if( compare_result == 0 )
      {
        if( existing_baue != NULL )
          *existing_baue = baue_temp;

        switch( baus->existing_key )
        {
          case LFDS720_BTREE_NU_EXISTING_KEY_OVERWRITE:
            LFDS720_BTREE_NU_SET_VALUE_IN_ELEMENT( *baue_temp, baue->value );
            return LFDS720_BTREE_NU_INSERT_RESULT_SUCCESS_OVERWRITE;
          break;

          case LFDS720_BTREE_NU_EXISTING_KEY_FAIL:
            return LFDS720_BTREE_NU_INSERT_RESULT_FAILURE_EXISTING_KEY;
          break;
        }
      }

      if( compare_result < 0 )
        baue_next = baue_temp->left;

      if( compare_result > 0 )
        baue_next = baue_temp->right;

      baue_parent = baue_temp;
      baue_temp = baue_next;
      if( baue_temp != NULL )
        LFDS720_MISC_BARRIER_LOAD;
    }

    /* TRD : second, we actually insert

             at this point baue_temp has come to NULL
             and baue_parent is the element to insert at
             and result of the last compare indicates
             the direction of insertion

             it may be that another tree has already inserted an element with
             the same key as ourselves, or other elements which mean our position
             is now wrong

             in this case, it is either inserted in the position we're trying
             to insert in now, in which case our insert will fail

             or, similarly, other elements will have come in where we are,
             and our insert will fail
    */

    if( baue_parent == NULL )
    {
      compare = NULL;
      baue->up = baus->root;
      LFDS720_MISC_BARRIER_STORE;
      LFDS720_PAL_ATOMIC_CAS( baus->root, compare, baue, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

      if( result == 0 )
        baue_temp = baus->root;
    }

    if( baue_parent != NULL )
    {
      if( compare_result <= 0 )
      {
        compare = NULL;
        baue->up = baue_parent;
        LFDS720_MISC_BARRIER_STORE;
        LFDS720_PAL_ATOMIC_CAS( baue_parent->left, compare, baue, LFDS720_MISC_CAS_STRENGTH_WEAK, result );
      }

      if( compare_result > 0 )
      {
        compare = NULL;
        baue->up = baue_parent;
        LFDS720_MISC_BARRIER_STORE;
        LFDS720_PAL_ATOMIC_CAS( baue_parent->right, compare, baue, LFDS720_MISC_CAS_STRENGTH_WEAK, result );
      }

      // TRD : if the insert fails, then resume searching at the insert node
      if( result == 0 )
        baue_temp = baue_parent;
    }

    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( baus->insert_backoff, backoff_iteration );
  }

  LFDS720_BACKOFF_AUTOTUNE( baus->insert_backoff, backoff_iteration );

  // TRD : if we get to here, we added (not failed or overwrite on exist) a new element
  if( existing_baue != NULL )
    *existing_baue = NULL;

  return LFDS720_BTREE_NU_INSERT_RESULT_SUCCESS;
}

