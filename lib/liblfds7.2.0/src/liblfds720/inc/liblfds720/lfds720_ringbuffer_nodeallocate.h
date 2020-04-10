/***** enums *****/
enum lfds720_ringbuffer_n_query
{
  LFDS720_RINGBUFFER_N_QUERY_SINGLETHREADED_GET_COUNT,
  LFDS720_RINGBUFFER_N_QUERY_SINGLETHREADED_VALIDATE
};

/***** structures *****/
struct lfds720_ringbuffer_n_element
{
  struct lfds720_freelist_n_element
    fe;

  struct lfds720_queue_numm_element
    qumme;

  struct lfds720_queue_numm_element
    *qumme_use; // TRD : hack; we need a new queue with no dummy element

  void
    *key,
    *value;
};

struct lfds720_ringbuffer_n_state
{
  struct lfds720_freelist_n_state
    fs;

  struct lfds720_queue_numm_state
    qumms;

  void
    (*element_cleanup_callback)( struct lfds720_ringbuffer_n_state *rs, void *key, void *value, enum lfds720_misc_flag unread_flag ),
    *user_state;
};

/***** public macros and prototypes *****/
void lfds720_ringbuffer_n_init_valid_on_current_logical_core( struct lfds720_ringbuffer_n_state *rs,
                                                            struct lfds720_ringbuffer_n_element *re_array_inc_dummy,
                                                            lfds720_pal_uint_t number_elements_inc_dummy,
                                                            void *user_state );
  // TRD : used in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

void lfds720_ringbuffer_n_cleanup( struct lfds720_ringbuffer_n_state *rs,
                                 void (*element_cleanup_callback)(struct lfds720_ringbuffer_n_state *rs, void *key, void *value, enum lfds720_misc_flag unread_flag) );

#define LFDS720_RINGBUFFER_N_GET_USER_STATE_FROM_STATE( ringbuffer_n_state )  ( (ringbuffer_n_state).user_state )

int lfds720_ringbuffer_n_read( struct lfds720_ringbuffer_n_state *rs,
                             void **key,
                             void **value );

void lfds720_ringbuffer_n_write( struct lfds720_ringbuffer_n_state *rs,
                               void *key,
                               void *value,
                               enum lfds720_misc_flag *overwrite_occurred_flag,
                               void **overwritten_key,
                               void **overwritten_value );

void lfds720_ringbuffer_n_query( struct lfds720_ringbuffer_n_state *rs,
                               enum lfds720_ringbuffer_n_query query_type,
                               void *query_input,
                               void *query_output );

