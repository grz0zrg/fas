/***** enums *****/
enum lfds720_list_nsu_position
{
  LFDS720_LIST_NSU_POSITION_START,
  LFDS720_LIST_NSU_POSITION_END,
  LFDS720_LIST_NSU_POSITION_AFTER
};

enum lfds720_list_nsu_query
{
  LFDS720_LIST_NSU_QUERY_GET_POTENTIALLY_INACCURATE_COUNT,
  LFDS720_LIST_NSU_QUERY_SINGLETHREADED_VALIDATE
};

/***** structures *****/
struct lfds720_list_nsu_element
{
  struct lfds720_list_nsu_element LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile next;

  void LFDS720_PAL_ALIGN(LFDS720_PAL_SINGLE_POINTER_LENGTH_IN_BYTES)
    *volatile value;

  void
    *key;
};

struct lfds720_list_nsu_state
{
  struct lfds720_list_nsu_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    dummy_element;

  struct lfds720_list_nsu_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *volatile end;

  struct lfds720_list_nsu_element LFDS720_PAL_ALIGN(LFDS720_PAL_ATOMIC_ISOLATION_LENGTH_IN_BYTES)
    *start;

  void
    *user_state;

  struct lfds720_misc_backoff_state
    after_backoff,
    end_backoff,
    start_backoff;
};

/***** public macros and prototypes *****/
void lfds720_list_nsu_init_valid_on_current_logical_core( struct lfds720_list_nsu_state *lasus,
                                                          void *user_state );
  // TRD : used in conjunction with the #define LFDS720_MISC_MAKE_VALID_ON_CURRENT_LOGICAL_CORE_INITS_COMPLETED_BEFORE_NOW_ON_ANY_OTHER_PHYSICAL_CORE

void lfds720_list_nsu_cleanup( struct lfds720_list_nsu_state *lasus,
                               void (*element_cleanup_callback)(struct lfds720_list_nsu_state *lasus, struct lfds720_list_nsu_element *lasue) );

#define LFDS720_LIST_NSU_GET_START( list_nsu_state )                                             ( LFDS720_MISC_BARRIER_LOAD, (list_nsu_state).start->next )
#define LFDS720_LIST_NSU_GET_NEXT( list_nsu_element )                                            ( LFDS720_MISC_BARRIER_LOAD, (list_nsu_element).next )
#define LFDS720_LIST_NSU_GET_START_AND_THEN_NEXT( list_nsu_state, pointer_to_list_nsu_element )  ( (pointer_to_list_nsu_element) == NULL ? ( (pointer_to_list_nsu_element) = LFDS720_LIST_NSU_GET_START(list_nsu_state) ) : ( (pointer_to_list_nsu_element) = LFDS720_LIST_NSU_GET_NEXT(*(pointer_to_list_nsu_element)) ) )

#define LFDS720_LIST_NSU_GET_KEY_FROM_ELEMENT( list_nsu_element )                                ( (list_nsu_element).key )
#define LFDS720_LIST_NSU_SET_KEY_IN_ELEMENT( list_nsu_element, new_key )                         ( (list_nsu_element).key = (void *) (lfds720_pal_uint_t) (new_key) )
#define LFDS720_LIST_NSU_GET_VALUE_FROM_ELEMENT( list_nsu_element )                              ( LFDS720_MISC_BARRIER_LOAD, (list_nsu_element).value )
#define LFDS720_LIST_NSU_SET_VALUE_IN_ELEMENT( list_nsu_element, new_value )                     { LFDS720_PAL_ATOMIC_SET( (list_nsu_element).value, (void *) (lfds720_pal_uint_t) (new_value) ); }
#define LFDS720_LIST_NSU_GET_USER_STATE_FROM_STATE( list_nsu_state )                             ( (list_nsu_state).user_state )

void lfds720_list_nsu_insert_at_position( struct lfds720_list_nsu_state *lasus,
                                          struct lfds720_list_nsu_element *lasue,
                                          struct lfds720_list_nsu_element *lasue_predecessor,
                                          enum lfds720_list_nsu_position position );

void lfds720_list_nsu_insert_at_start( struct lfds720_list_nsu_state *lasus,
                                       struct lfds720_list_nsu_element *lasue );

void lfds720_list_nsu_insert_at_end( struct lfds720_list_nsu_state *lasus,
                                     struct lfds720_list_nsu_element *lasue );

void lfds720_list_nsu_insert_after_element( struct lfds720_list_nsu_state *lasus,
                                            struct lfds720_list_nsu_element *lasue,
                                            struct lfds720_list_nsu_element *lasue_predecessor );

int lfds720_list_nsu_get_by_key( struct lfds720_list_nsu_state *lasus,
                                 int (*key_compare_function)(void const *new_key, void const *existing_key),
                                 void *key, 
                                 struct lfds720_list_nsu_element **lasue );

void lfds720_list_nsu_query( struct lfds720_list_nsu_state *lasus,
                             enum lfds720_list_nsu_query query_type,
                             void *query_input,
                             void *query_output );

