/***** includes *****/
#include "lfds720_list_nsu_internal.h"





/****************************************************************************/
void lfds720_list_nsu_insert_at_position( struct lfds720_list_nsu_state *lasus,
                                          struct lfds720_list_nsu_element *lasue,
                                          struct lfds720_list_nsu_element *lasue_predecessor,
                                          enum lfds720_list_nsu_position position )
{
  LFDS720_PAL_ASSERT( lasus != NULL );
  LFDS720_PAL_ASSERT( lasue != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasue->next % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasue->value % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  // TRD : lasue_predecessor asserted in the switch
  // TRD : position can be any value in its range

  switch( position )
  {
    case LFDS720_LIST_NSU_POSITION_START:
      lfds720_list_nsu_insert_at_start( lasus, lasue );
    break;

    case LFDS720_LIST_NSU_POSITION_END:
      lfds720_list_nsu_insert_at_end( lasus, lasue );
    break;

    case LFDS720_LIST_NSU_POSITION_AFTER:
      lfds720_list_nsu_insert_after_element( lasus, lasue, lasue_predecessor );
    break;
  }

  return;
}





/****************************************************************************/
void lfds720_list_nsu_insert_at_start( struct lfds720_list_nsu_state *lasus,
                                       struct lfds720_list_nsu_element *lasue )
{
  char unsigned 
    result;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  LFDS720_PAL_ASSERT( lasus != NULL );
  LFDS720_PAL_ASSERT( lasue != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasue->next % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasue->value % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );

  LFDS720_MISC_BARRIER_LOAD;

  lasue->next = lasus->start->next;

  do
  {
    LFDS720_MISC_BARRIER_STORE;
    LFDS720_PAL_ATOMIC_CAS( lasus->start->next, lasue->next, lasue, LFDS720_MISC_CAS_STRENGTH_WEAK, result );
    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( lasus->start_backoff, backoff_iteration );
  }
  while( result == 0 );

  LFDS720_BACKOFF_AUTOTUNE( lasus->start_backoff, backoff_iteration );

  return;
}





/****************************************************************************/
void lfds720_list_nsu_insert_at_end( struct lfds720_list_nsu_state *lasus,
                                     struct lfds720_list_nsu_element *lasue )
{
  char unsigned 
    result;

  enum lfds720_misc_flag
    finished_flag = LFDS720_MISC_FLAG_LOWERED;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  struct lfds720_list_nsu_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *compare;

  struct lfds720_list_nsu_element
    *volatile lasue_next,
    *volatile lasue_end;

  LFDS720_PAL_ASSERT( lasus != NULL );
  LFDS720_PAL_ASSERT( lasue != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasue->next % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasue->value % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );

  /* TRD : begin by assuming end is correctly pointing to the final element
           try to link (comparing for next being NULL)
           if we fail, move down list till we find last element
           and retry
           when successful, update end to ourselves

           note there's a leading dummy element
           so lasus->end always points to an element
  */

  LFDS720_MISC_BARRIER_LOAD;

  lasue->next = NULL;
  lasue_end = lasus->end;

  while( finished_flag == LFDS720_MISC_FLAG_LOWERED )
  {
    compare = NULL;

    LFDS720_MISC_BARRIER_STORE;
    LFDS720_PAL_ATOMIC_CAS( lasue_end->next, compare, lasue, LFDS720_MISC_CAS_STRENGTH_STRONG, result );

    if( result == 1 )
      finished_flag = LFDS720_MISC_FLAG_RAISED;
    else
    {
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( lasus->end_backoff, backoff_iteration );

      lasue_end = compare;
      lasue_next = LFDS720_LIST_NSU_GET_NEXT( *lasue_end );

      while( lasue_next != NULL )
      {
        lasue_end = lasue_next;
        lasue_next = LFDS720_LIST_NSU_GET_NEXT( *lasue_end );
      }
    }
  }

  lasus->end = lasue;

  LFDS720_BACKOFF_AUTOTUNE( lasus->end_backoff, backoff_iteration );

  return;
}





/****************************************************************************/
#pragma warning( disable : 4100 )

void lfds720_list_nsu_insert_after_element( struct lfds720_list_nsu_state *lasus,
                                            struct lfds720_list_nsu_element *lasue,
                                            struct lfds720_list_nsu_element *lasue_predecessor )
{
  char unsigned 
    result;

  lfds720_pal_uint_t
    backoff_iteration = LFDS720_BACKOFF_INITIAL_VALUE;

  LFDS720_PAL_ASSERT( lasus != NULL );
  LFDS720_PAL_ASSERT( lasue != NULL );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasue->next % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( (lfds720_pal_uint_t) &lasue->value % LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES == 0 );
  LFDS720_PAL_ASSERT( lasue_predecessor != NULL );

  LFDS720_MISC_BARRIER_LOAD;

  lasue->next = lasue_predecessor->next;

  do
  {
    LFDS720_MISC_BARRIER_STORE;
    LFDS720_PAL_ATOMIC_CAS( lasue_predecessor->next, lasue->next, lasue, LFDS720_MISC_CAS_STRENGTH_WEAK, result );
    if( result == 0 )
      LFDS720_BACKOFF_EXPONENTIAL_BACKOFF( lasus->after_backoff, backoff_iteration );
  }
  while( result == 0 );

  LFDS720_BACKOFF_AUTOTUNE( lasus->after_backoff, backoff_iteration );

  return;
}

#pragma warning( default : 4100 )

