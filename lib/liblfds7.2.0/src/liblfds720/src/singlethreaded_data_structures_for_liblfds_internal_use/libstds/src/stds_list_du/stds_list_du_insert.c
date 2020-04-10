#include "stds_list_du_internal.h"





/****************************************************************************/
void stds_list_du_insert_at_start( struct stds_list_du_state *ldus,
                                   struct stds_list_du_element *ldue )
{
  STDS_PAL_ASSERT( ldus != NULL );
  STDS_PAL_ASSERT( ldue != NULL );

  stds_list_du_insert_at_position( ldus, ldue, NULL, STDS_LIST_DU_POSITION_START );

  return;
}





/****************************************************************************/
void stds_list_du_insert_at_end( struct stds_list_du_state *ldus,
                                 struct stds_list_du_element *ldue )
{
  STDS_PAL_ASSERT( ldus != NULL );
  STDS_PAL_ASSERT( ldue != NULL );

  stds_list_du_insert_at_position( ldus, ldue, NULL, STDS_LIST_DU_POSITION_END );

  return;
}





/****************************************************************************/
void stds_list_du_insert_before( struct stds_list_du_state *ldus,
                                 struct stds_list_du_element *ldue_new,
                                 struct stds_list_du_element *ldue_target )
{
  STDS_PAL_ASSERT( ldus != NULL );
  STDS_PAL_ASSERT( ldue_new != NULL );
  STDS_PAL_ASSERT( ldue_target != NULL );

  stds_list_du_insert_at_position( ldus, ldue_new, ldue_target, STDS_LIST_DU_POSITION_BEFORE );

  return;
}





/****************************************************************************/
void stds_list_du_insert_after( struct stds_list_du_state *ldus,
                                struct stds_list_du_element *ldue_new,
                                struct stds_list_du_element *ldue_target )
{
  STDS_PAL_ASSERT( ldus != NULL );
  STDS_PAL_ASSERT( ldue_new != NULL );
  STDS_PAL_ASSERT( ldue_target != NULL );

  stds_list_du_insert_at_position( ldus, ldue_new, ldue_target, STDS_LIST_DU_POSITION_AFTER );

  return;
}





/****************************************************************************/
void stds_list_du_insert_at_position( struct stds_list_du_state *ldus,
                                      struct stds_list_du_element *ldue_new,
                                      struct stds_list_du_element *ldue_target,
                                      enum stds_list_du_position position )
{
  STDS_PAL_ASSERT( ldus != NULL );
  STDS_PAL_ASSERT( ldue_new != NULL );
  // TRD : ldue_target can be NULL, is STDS_PAL_ASSERTed in cases
  // TRD : position can be any value in its range

  switch( position )
  {
    case STDS_LIST_DU_POSITION_START:
      STDS_PAL_ASSERT( ldue_target == NULL );

      ldue_new->next = ldus->start;
      ldue_new->prev = NULL;

      if( ldus->end == NULL )
        ldus->end = ldue_new;
      if( ldus->start != NULL )
        ldus->start->prev = ldue_new;
      ldus->start = ldue_new;
    break;

    case STDS_LIST_DU_POSITION_END:
      STDS_PAL_ASSERT( ldue_target == NULL );

      ldue_new->next = NULL;
      ldue_new->prev = ldus->end;

      if( ldus->start == NULL )
        ldus->start = ldue_new;
      if( ldus->end != NULL )
        ldus->end->next = ldue_new;
      ldus->end = ldue_new;
    break;

    case STDS_LIST_DU_POSITION_AFTER:
      STDS_PAL_ASSERT( ldue_target != NULL );

      ldue_new->next = ldue_target->next;
      ldue_new->prev = ldue_target;

      ldue_target->next = ldue_new;
      if( ldus->end == ldue_target )
        ldus->end = ldue_new;
    break;

    case STDS_LIST_DU_POSITION_BEFORE:
      STDS_PAL_ASSERT( ldue_target != NULL );

      ldue_new->next = ldue_target;
      ldue_new->prev = ldue_target->prev;

      ldue_target->prev = ldue_new;
      if( ldus->start == ldue_target )
        ldus->start = ldue_new;
    break;
  }

  ldus->number_elements++;

  return;
}

