/***** enums *****/
enum lfds720_queue_bmm_query
{
  LFDS720_QUEUE_BMM_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
  LFDS720_QUEUE_BMM_QUERY_SINGLETHREADED_VALIDATE
};

/***** structures *****/
struct lfds720_queue_bmm_element
{
  lfds720_pal_uint_t volatile
    sequence_number;

  void
    *volatile key,
    *volatile value;
};

struct lfds720_queue_bmm_state
{
  lfds720_pal_uint_t
    number_elements,
    mask;

  lfds720_pal_uint_t volatile LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    read_index,
    write_index;

  struct lfds720_queue_bmm_element
    *element_array;

  void
    *user_state;

  struct lfds720_misc_backoff_state
    dequeue_backoff,
    enqueue_backoff;
};

/***** public macros and prototypes *****/
void lfds720_queue_bmm_init_valid_on_current_logical_core( struct lfds720_queue_bmm_state *qbmms,
                                                           struct lfds720_queue_bmm_element *element_array,
                                                           lfds720_pal_uint_t number_elements,
                                                           void *user_state );

void lfds720_queue_bmm_cleanup( struct lfds720_queue_bmm_state *qbmms,
                                void (*element_cleanup_callback)(struct lfds720_queue_bmm_state *qbmms,
                                                                 void *key,
                                                                 void *value) );

#define LFDS720_QUEUE_BMM_GET_USER_STATE_FROM_STATE( queue_bmm_state )  ( (queue_bmm_state).user_state )

int lfds720_queue_bmm_enqueue( struct lfds720_queue_bmm_state *qbmms,
                               void *key,
                               void *value );

int lfds720_queue_bmm_dequeue( struct lfds720_queue_bmm_state *qbmms,
                                      void **key,
                                      void **value );

void lfds720_queue_bmm_query( struct lfds720_queue_bmm_state *qbmms,
                              enum lfds720_queue_bmm_query query_type,
                              void *query_input,
                              void *query_output );

