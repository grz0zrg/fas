/***** private prototypes *****/
#include "stds_list_du_internal.h"





/****************************************************************************/
void stds_list_du_remove_element( struct stds_list_du_state *ldus, struct stds_list_du_element *ldue )
{
  enum stds_list_du_unlink_operation
    unlink_operation = STDS_LIST_DU_UNLINK_OPERATION_UNKNOWN;

  STDS_PAL_ASSERT( ldus != NULL );
  STDS_PAL_ASSERT( ldue != NULL );

  if( ldue->prev == NULL and ldue->next == NULL )
    unlink_operation = STDS_LIST_DU_UNLINK_OPERATION_PREV_NULL_NEXT_NULL;

  if( ldue->prev == NULL and ldue->next != NULL )
    unlink_operation = STDS_LIST_DU_UNLINK_OPERATION_PREV_NULL_NEXT_NOT_NULL;

  if( ldue->prev != NULL and ldue->next == NULL )
    unlink_operation = STDS_LIST_DU_UNLINK_OPERATION_PREV_NOT_NULL_NEXT_NULL;

  if( ldue->prev != NULL and ldue->next != NULL )
    unlink_operation = STDS_LIST_DU_UNLINK_OPERATION_PREV_NOT_NULL_NEXT_NOT_NULL;

  switch( unlink_operation )
  {
    case STDS_LIST_DU_UNLINK_OPERATION_UNKNOWN:
      // TRD : to eliminate compiler warning
    break;

    case STDS_LIST_DU_UNLINK_OPERATION_PREV_NULL_NEXT_NULL:
      ldus->start = ldus->end = NULL;
    break;

    case STDS_LIST_DU_UNLINK_OPERATION_PREV_NULL_NEXT_NOT_NULL:
      ldus->start = ldue->next;
      ldue->next->prev = NULL;
    break;

    case STDS_LIST_DU_UNLINK_OPERATION_PREV_NOT_NULL_NEXT_NULL:
      ldus->end = ldue->prev;
      ldue->prev->next = NULL;
    break;

    case STDS_LIST_DU_UNLINK_OPERATION_PREV_NOT_NULL_NEXT_NOT_NULL:
      ldue->prev->next = ldue->next;
      ldue->next->prev = ldue->prev;
    break;
  }

  ldus->number_elements--;

  return;
}

