/***** enums *****/
enum lfds720_queue_bss_query
{
  LFDS720_QUEUE_BSS_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
  LFDS720_QUEUE_BSS_QUERY_VALIDATE
};

/***** structures *****/
struct lfds720_queue_bss_element
{
  void
    *key,
    *value;
};

struct lfds720_queue_bss_state
{
  lfds720_pal_uint_t
    number_elements,
    mask;

  lfds720_pal_uint_t volatile
    read_index,
    write_index;

  struct lfds720_queue_bss_element
    *element_array;

  void
    *user_state;
};

/***** public prototypes *****/
void lfds720_queue_bss_init_valid_on_current_logical_core( struct lfds720_queue_bss_state *qbsss, 
                                                           struct lfds720_queue_bss_element *element_array,
                                                           lfds720_pal_uint_t number_elements,
                                                           void *user_state );
  // TRD : number_elements must be a positive integer power of 2
  // TRD : used in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

void lfds720_queue_bss_cleanup( struct lfds720_queue_bss_state *qbsss,
                                void (*element_cleanup_callback)(struct lfds720_queue_bss_state *qbsss, void *key, void *value) );

#define LFDS720_QUEUE_BSS_GET_USER_STATE_FROM_STATE( queue_bss_state )  ( (queue_bss_state).user_state )

int lfds720_queue_bss_enqueue( struct lfds720_queue_bss_state *qbsss,
                               void *key,
                               void *value );

int lfds720_queue_bss_dequeue( struct lfds720_queue_bss_state *qbsss,
                               void **key,
                               void **value );

void lfds720_queue_bss_query( struct lfds720_queue_bss_state *qbsss,
                              enum lfds720_queue_bss_query query_type,
                              void *query_input,
                              void *query_output );

