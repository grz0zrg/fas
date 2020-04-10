/***** includes *****/
#include "lfds720_list_singlylinked_unordered_internal.h"





/****************************************************************************/
enum lfds720_list_so_remove_result lfds720_list_so_remove( struct lfds720_list_so_state *lsos,
                                                           void *key,
                                                           struct lfds720_smrhp_per_thread_state *smrhpts )
{
  enum lfds720_list_so_remove_result
    rr;

  int
    cr;

  struct lfds720_list_so_element
    *cursor,
    *next;

  LFDS720_PAL_ASSERT( lsos != NULL );
  LFDS720_PAL_ASSERT( key != NULL );
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  /* TRD : it's no use saying "delete next" because given a lock-free list anything can happen
           so we need to uniquely and exactly specify which element we mean to remove - which means its key

           we iterate over the list, using a pair of pointers, side-by-side
           when the forward pointer matches the element
           we mark it as deleted
           and then we get its next
           and then CAS the backward pointer on that next
           if we find an element greater than that we're looking for, we're also good (our target element is not in the list)
  */

  cursor = lsos->start;
  next = lfds720_list_so_get_start( lsos, smrhpts );

  while( finished_flag == LFDS720_MISC_FLAG_LOWERED )
  {
    if( next == NULL )
    {
      finished_flag = LFDS720_MISC_FLAG_RAISED;
      rr = LFDS720_LIST_SO_INSERT_RESULT_NOT_FOUND;
      // TRD : clear hazard pointers
    }

    if( next != NULL )
    {
      cr = lsos->key_compare_function( key, next->key );

      if( cr == 0 )
      {
      }

      if( cr < 0 )
      {
      }

      if( cr > 0 )
      {
        finished_flag = LFDS720_MISC_FLAG_RAISED;
        rr = LFDS720_LIST_SO_INSERT_RESULT_NOT_FOUND;
        // TRD : clear hazard pointers
      }

    }


  }



  return rr;
}

