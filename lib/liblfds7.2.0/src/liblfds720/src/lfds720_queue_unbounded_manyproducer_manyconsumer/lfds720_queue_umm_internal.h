/***** the library wide include file *****/
#include "../liblfds720_internal.h"

/***** enums *****/
enum lfds720_queue_umm_queue_state
{
  LFDS720_QUEUE_UMM_QUEUE_STATE_UNKNOWN, 
  LFDS720_QUEUE_UMM_QUEUE_STATE_EMPTY,
  LFDS720_QUEUE_UMM_QUEUE_STATE_ENQUEUE_OUT_OF_PLACE,
  LFDS720_QUEUE_UMM_QUEUE_STATE_ATTEMPT_DEQUEUE
};

/***** private prototypes *****/

