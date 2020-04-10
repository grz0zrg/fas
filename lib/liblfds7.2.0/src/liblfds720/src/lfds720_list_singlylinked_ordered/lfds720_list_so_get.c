/***** includes *****/
#include "lfds720_list_singlylinked_unordered_internal.h"





/****************************************************************************/
struct lfds720_list_so_element *lfds720_list_so_get_start( struct lfds720_list_so_state *lsos,
                                                           struct lfds720_smrhp_per_thread_state *smrhpts )
{
  struct lfds720_list_so_element
    *lsoe;

  LFDS720_PAL_ASSERT( lsos != NULL );
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  /* TRD : there is a leading dummy element
           get its next pointer - this is the first element
           check for the unlink bit in that first element
           if it's set, we can't trust the next pointer in this element,
           so this element is no use to us

           we're after the very first element in the list, so we have to repeat
           getting the next pointer of the start element
  */

  do
  {
    LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_CURSOR_POINTER, lsos->start->next );

    lsoe = LFDS720_SMRHP_GET_HAZARD_POINTER( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_CURSOR_POINTER );

    if( lsos == NULL )
    {
      LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_CURSOR_POINTER );
      return NULL;
    }
  }
  while( (lsoe->next) & LFDS720_LIST_SO_DELETE_BIT );

  return lsoe;
}





/****************************************************************************/
struct lfds720_list_so_element *lfds720_list_so_get_next( struct lfds720_list_so_state *lsos,
                                                          struct lfds720_list_so_element *lsoe;
                                                          struct lfds720_smrhp_per_thread_state *smrhpts )
{
  enum lfds720_misc_flag
    finished_flag = LFDS720_MISC_LOWERED;

  int
    cr;

  struct lfds720_list_so_element
    *cursor,
    *next;

  /* TRD : we first try simply to get lsoe->next
           in this, we have to consider that lose may be unlinked and lsoe->next may be unlinked
           if lsoe is unlinked, then all bets are off - we need then to return to a search from the beginning of the list (more on why later)
           if lsoe->next is unlinked, we just repeat trying to get lsoe->next

           the problem with lsoe being unlinked is that we then cannot trust our next pointer
           it could for example be pointing at deallocated store
           so we have to return to searching the list from the start
           the list could of change have changed utterly; but we know the element we were *on* is lsoe
           so we have its key
           so now we need to scan the list for the first element larger than lsoe or until we hit NULL
  */

  LFDS720_PAL_ASSERT( lsos != NULL );
  LFDS720_PAL_ASSERT( lsoe != NULL );
  LFDS720_PAL_ASSERT( smrhpts != NULL );

  /* TRD : note the while clause
           it may be that lsoe->next has the delete bit set
           in which case next may be non-NULL but point to deallocated store

           so we exit this while loop in one of three states;
           1. next is NULL
              - we're done, return to user
           2. lsoe->next delete bit is set
              - time for a full search from the start
           3. lsoe->next delete bit is not set
              - we hold next and we can trust it exists, so we can return it
  */

  do
  {
    LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_NEXT_POINTER, lsoe->next );
    next = LFDS720_SMRHP_GET_HAZARD_POINTER( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_NEXT_POINTER );
  }
  while( !(lsoe->next & LFDS720_LIST_SO_DELETE_BIT) and (next != NULL) and (next->next & LFDS720_LIST_SO_DELETE_BIT) );

  // TRD : now figure out our state
  if( next == NULL and !(lsoe->next & LFDS720_LIST_SO_DELETE_BIT) )
    state = LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_LINKED_NEXT_IS_NULL;

  if( next != NULL and !(lsoe->next & LFDS720_LIST_SO_DELETE_BIT) )
    state = LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_LINKED_NEXT_IS_NOT_NULL;

  if( lsoe->next & LFDS720_LIST_SO_DELETE_BIT )
    state = LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_UNLINKED;

  switch( state )
  {
    case LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_LINKED_NEXT_IS_NULL:
      LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_CURSOR_POINTER );
      LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_NEXT_POINTER );
      return NULL;
    break;

    case LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_LINKED_NEXT_IS_NOT_NULL:
      LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_CURSOR_POINTER, next );
      LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_NEXT_POINTER );
      return next;
    break;

    case LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_UNLINKED:
    {
      LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_STORE_POINTER, lsoe );

      cursor = lfds720_list_so_get_start( lsos, smrhpts );

      while( cursor != NULL and cr <= 0 )
      {
        cr = lsos->key_compare_function( cursor->key, lsoe->key );

        if( cr > 0 )
          break;

        if( cr <= 0 )
        {
          do
          {
            LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_NEXT_POINTER, cursor->next );
            next = LFDS720_SMRHP_GET_HAZARD_POINTER( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_NEXT_POINTER );
          }
          while( !(cursor->next & LFDS720_LIST_SO_DELETE_BIT) and (next != NULL) and (next->next & LFDS720_LIST_SO_DELETE_BIT) );

          // TRD : now figure out our state
          if( next == NULL and !(cursor->next & LFDS720_LIST_SO_DELETE_BIT) )
            state = LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_LINKED_NEXT_IS_NULL;

          if( next != NULL and !(cursor->next & LFDS720_LIST_SO_DELETE_BIT) )
            state = LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_LINKED_NEXT_IS_NOT_NULL;

          if( cursor->next & LFDS720_LIST_SO_DELETE_BIT )
            state = LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_UNLINKED;

          switch( state )
          {
            case LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_LINKED_NEXT_IS_NULL:
              LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_CURSOR_POINTER );
              LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_NEXT_POINTER );
              return NULL;
            break;

            case LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_LINKED_NEXT_IS_NOT_NULL:
              LFDS720_SMRHP_MAKE_ALLOCATION_UNRECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_CURSOR_POINTER, next );
              LFDS720_SMRHP_MAKE_ALLOCATION_RECLAIMABLE( smrhpts, LFDS720_LIST_SO_SMRHP_HAZARD_POINTER_INDEX_NEXT_POINTER );
              cursor = next;
            break;

            case LFDS720_LIST_SO_GET_NEXT_STATE_CURSOR_UNLINKED:
              cursor = lfds720_list_so_get_start( lsos, smrhpts );
            break;
          }
        }
      }
    }
    break;
  }

  return cursor;
}

