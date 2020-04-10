/***** includes *****/
#include "lfds720_list_singlylinked_ordered_internal.h"





/****************************************************************************/
enum lfds720_list_so_insert_result lfds720_list_so_insert( struct lfds720_list_so_state *lsos,
                                                           struct lfds720_list_so_element *lsoe,
                                                           struct lfds720_list_so_element *existing_lsoe,
                                                           struct lfds720_smrhp_per_thread_state *smrhppts )
{
  char unsigned 
    result;

  enum lfds720_misc_flag
    finished_flag = LFDS720_MISC_FLAG_LOWERED;

  int
    compare_result = 0;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  struct lfds720_list_so_element
    *volatile lsoe_temp = NULL,
    *volatile lsoe_trailing;

  LFDS720_PAL_ASSERT( lsos != NULL );
  LFDS720_PAL_ASSERT( lsoe != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lsoe->next % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lsoe->value % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  // TRD : existing_lsoe can be NULL

  LFDS720_MISC_BARRIER_LOAD;

  lsoe_trailing = lsos->start;
  lsoe_temp = lfds720_list_so_get_start( lsos, smrhppts );

  while( finished_flag == LFDS720_MISC_FLAG_LOWERED )
  {
    if( lsoe_temp == NULL )
      compare_result = -1;

    if( lsoe_temp != NULL )
    {
      LFDS720_MISC_BARRIER_LOAD;
      compare_result = lsos->key_compare_function( lsoe->key, lsoe_temp->key );
    }

    if( compare_result == 0 )
    {
      if( existing_lsoe != NULL )
        *existing_lsoe = lsoe_temp;

      switch( lsos->existing_key )
      {
        case LFDS720_LIST_SO_EXISTING_KEY_OVERWRITE:
          LFDS720_LIST_SO_SET_VALUE_IN_ELEMENT( *lsoe_temp, lsoe->value );
          return LFDS720_LIST_SO_INSERT_RESULT_SUCCESS_OVERWRITE;
        break;

        case LFDS720_LIST_SO_EXISTING_KEY_FAIL:
          return LFDS720_LIST_SO_INSERT_RESULT_FAILURE_EXISTING_KEY;
        break;
      }

      finished_flag = LFDS720_MISC_FLAG_RAISED;
    }

    if( compare_result < 0 )
    {
      lsoe->next = lsoe_temp;
      LFDS720_MISC_BARRIER_STORE;
      LFDS720_PAL_ATOMIC_CAS( lsoe_trailing->next, lsoe->next, lsoe, LFDS720_MISC_CAS_STRENGTH_WEAK, result );

      if( result == 1 )
        finished_flag = LFDS720_MISC_FLAG_RAISED;
      else
      {
        LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( lsos->insert_backoff, backoff_iteration );
        // TRD : if we fail to link, someone else has linked and so we need to redetermine our position is correct
        lsoe_temp = lsoe_trailing->next;
      }
    }

    if( compare_result > 0 )
    {
      // TRD : move trailing along by one element
      lsoe_trailing = lsoe_trailing->next;

      /* TRD : set temp as the element after trailing
               if the new element we're linking is larger than all elements in the list,
               lsoe_temp will now go to NULL and we'll link at the end
      */
      lsoe_temp = lsoe_trailing->next;
    }
  }

  LFDS720_BACKOFF_AUTOTUNE( lsos->insert_backoff, backoff_iteration );

  return LFDS720_LIST_SO_INSERT_RESULT_SUCCESS;
}

