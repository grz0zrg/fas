/***** the library wide include file *****/
#include "../libstds_internal.h"

/***** enums *****/
enum stds_list_du_unlink_operation
{
  STDS_LIST_DU_UNLINK_OPERATION_UNKNOWN,
  STDS_LIST_DU_UNLINK_OPERATION_PREV_NULL_NEXT_NULL,
  STDS_LIST_DU_UNLINK_OPERATION_PREV_NULL_NEXT_NOT_NULL,
  STDS_LIST_DU_UNLINK_OPERATION_PREV_NOT_NULL_NEXT_NULL,
  STDS_LIST_DU_UNLINK_OPERATION_PREV_NOT_NULL_NEXT_NOT_NULL
};

/***** private prototypes *****/

