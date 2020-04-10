/***** enums *****/
enum lfds720_queue_nuss_query
{
  LFDS720_QUEUE_NUSS_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
  LFDS720_QUEUE_NUSS_QUERY_VALIDATE
};

/***** structures *****/
struct lfds720_queue_nuss_element
{
  struct lfds720_queue_nuss_element
    *next;

  void
    *key,
    *value,
    *user_state;
};

struct lfds720_queue_nuss_state
{
  struct lfds720_queue_nuss_element
    *enqueue_writer_writes,
    *dequeue_writer_writes,
    *dequeue_reader_writes;

  void
    (*dequeued_element_callback)( struct lfds720_queue_nuss_state *qusss, struct lfds720_queue_nuss_element *qusse ),
    *user_state;
};

/***** public macros and prototypes *****/
void lfds720_queue_nuss_init_valid_on_current_logical_core( struct lfds720_queue_nuss_state *qusss,
                                                            struct lfds720_queue_nuss_element *qusse_dummy_element,
                                                            void (*dequeued_element_callback)( struct lfds720_queue_nuss_state *qusss, struct lfds720_queue_nuss_element *qusse ),
                                                            void *user_state );
  /* TRD : the enqueue function is responsible for removing queue elements from the queue after they have been dequeued
           when you dequeue, you will always get the correct *value*
           but the queue element remains in the queue until an *enqueue* operation occurs
           at which point all lingering dequeue elements are removed, and passed to the "dequeued_element_callback" function
  */

void lfds720_queue_nuss_cleanup( struct lfds720_queue_nuss_state *qusss,
                                void (*element_cleanup_callback)( struct lfds720_queue_nuss_state *qusss,
                                                                  struct lfds720_queue_nuss_element *qusse,
                                                                  enum lfds720_misc_flag dummy_element_flag ) );

#define LFDS720_QUEUE_NUSS_GET_KEY_FROM_ELEMENT( queue_nuss_element )             ( (queue_nuss_element).key )
#define LFDS720_QUEUE_NUSS_SET_KEY_IN_ELEMENT( queue_nuss_element, new_key )      ( (queue_nuss_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_QUEUE_NUSS_GET_VALUE_FROM_ELEMENT( queue_nuss_element )           ( (queue_nuss_element).value )
#define LFDS720_QUEUE_NUSS_SET_VALUE_IN_ELEMENT( queue_nuss_element, new_value )  ( (queue_nuss_element).value = (void *) (lfds720_pal_uint_t) (new_value) )
#define LFDS720_QUEUE_NUSS_GET_USER_STATE_FROM_STATE( queue_nuss_state )          ( (queue_nuss_state).user_state )

#define LFDS720_QUEUE_NUSS_GET_USER_STATE_FROM_ELEMENT( queue_nuss_element )                ( (queue_nuss_element).user_state )
#define LFDS720_QUEUE_NUSS_SET_USER_STATE_IN_ELEMENT( queue_nuss_element, new_user_state )  ( (queue_nuss_element).user_state = (new_user_state) )

void lfds720_queue_nuss_enqueue( struct lfds720_queue_nuss_state *qusss,
                                struct lfds720_queue_nuss_element *qusse );

void lfds720_queue_nuss_enqueue_inside_dequeue_element_callback( struct lfds720_queue_nuss_state *qusss,
                                                                struct lfds720_queue_nuss_element *qusse );
  // TRD : the normal lfds720_queue_nuss_enqueue() function MUST not be used inside the dequeued_element_callback() function passed to lfds720_queue_nuss_init_valid_on_current_logical_core()

int lfds720_queue_nuss_dequeue( struct lfds720_queue_nuss_state *qusss,
                               void **key,
                               void **value );

void lfds720_queue_nuss_flush_dequeued_elements( struct lfds720_queue_nuss_state *qusss );
  // TRD : the user never needs to call this function; it exists for the test programme

void lfds720_queue_nuss_query( struct lfds720_queue_nuss_state *qusss,
                              enum lfds720_queue_nuss_query query_type,
                              void *query_input,
                              void *query_output );

