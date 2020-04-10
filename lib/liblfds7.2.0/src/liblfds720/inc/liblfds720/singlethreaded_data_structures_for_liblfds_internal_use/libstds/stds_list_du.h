/***** enums *****/
enum stds_list_du_position
{
  STDS_LIST_DU_POSITION_START,
  STDS_LIST_DU_POSITION_END,
  STDS_LIST_DU_POSITION_AFTER,
  STDS_LIST_DU_POSITION_BEFORE
};

enum stds_list_du_query
{
  STDS_LIST_DU_QUERY_NUMBER_ELEMENTS,
  STDS_LIST_DU_QUERY_VALIDATE
};

/***** structures *****/
struct stds_list_du_element
{
  struct stds_list_du_element
    *next,
    *prev;

  void
    *key,
    *value;
};

struct stds_list_du_state
{
  stds_pal_uint_t
    number_elements;

  struct stds_list_du_element
    *start,
    *end;

  void
    *user_state;
};

/***** public macros and prototypes *****/
void stds_list_du_init( struct stds_list_du_state *ls, void *user_state );

void stds_list_du_cleanup( struct stds_list_du_state *ls,
                           void (*element_cleanup_callback)(struct stds_list_du_state *ls, struct stds_list_du_element *le) );

#define STDS_LIST_DU_COMPILE_TIME_INIT( user_state )  { 0, NULL, NULL, user_state }

#define STDS_LIST_DU_GET_START( list_du_state )                                            ( (list_du_state).start )
#define STDS_LIST_DU_GET_END( list_du_state )                                              ( (list_du_state).end )
#define STDS_LIST_DU_GET_NEXT( list_du_element )                                           ( (list_du_element).next )
#define STDS_LIST_DU_GET_PREV( list_du_element )                                           ( (list_du_element).prev )
#define STDS_LIST_DU_GET_START_AND_THEN_NEXT( list_du_state, pointer_to_list_du_element )  ( (pointer_to_list_du_element) == NULL ? ( (pointer_to_list_du_element) = STDS_LIST_DU_GET_START(list_du_state) ) : ( (pointer_to_list_du_element) = STDS_LIST_DU_GET_NEXT(*(pointer_to_list_du_element)) ) )
#define STDS_LIST_DU_GET_END_AND_THEN_PREV( list_du_state, pointer_to_list_du_element )    ( (pointer_to_list_du_element) == NULL ? ( (pointer_to_list_du_element) = STDS_LIST_DU_GET_END(list_du_state) )   : ( (pointer_to_list_du_element) = STDS_LIST_DU_GET_PREV(*(pointer_to_list_du_element)) ) )

#define STDS_LIST_DU_GET_KEY_FROM_ELEMENT( list_du_element )                               ( (list_du_element).key )
#define STDS_LIST_DU_SET_KEY_IN_ELEMENT( list_du_element, new_key )                        ( (list_du_element).key = (void *) (stds_pal_uint_t) (new_key) )
#define STDS_LIST_DU_GET_VALUE_FROM_ELEMENT( list_du_element )                             ( (list_du_element).value )
#define STDS_LIST_DU_SET_VALUE_IN_ELEMENT( list_du_element, new_value )                    ( (list_du_element).value = (void *) (stds_pal_uint_t) (new_value) )
#define STDS_LIST_DU_GET_USER_STATE_FROM_STATE( list_du_state )                            ( (list_du_state).user_state )

void stds_list_du_insert_at_start( struct stds_list_du_state *ls,
                                   struct stds_list_du_element *le );

void stds_list_du_insert_at_end( struct stds_list_du_state *ls,
                                 struct stds_list_du_element *le );

void stds_list_du_insert_before( struct stds_list_du_state *ls,
                                 struct stds_list_du_element *le_new,
                                 struct stds_list_du_element *le_target );

void stds_list_du_insert_after( struct stds_list_du_state *ls,
                                struct stds_list_du_element *le_new,
                                struct stds_list_du_element *le_target );

void stds_list_du_insert_at_position( struct stds_list_du_state *ls,
                                      struct stds_list_du_element *le_new,
                                      struct stds_list_du_element *le_target,
                                      enum stds_list_du_position position );

void stds_list_du_remove_element( struct stds_list_du_state *ls,
                                  struct stds_list_du_element *le );

void stds_list_du_query( struct stds_list_du_state *ls, enum stds_list_du_query query_type, void *query_input, void *query_output );

