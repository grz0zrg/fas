/***** defines *****/

/***** enums *****/
enum stds_freelist_query_type
{
  STDS_FREELIST_QUERY_NUMBER_ELEMENTS,
  STDS_FREELIST_QUERY_VALIDATE
};

/***** structures *****/
struct stds_freelist_state
{
  struct stds_freelist_element
    *top;

  void
    *user_state;
};

struct stds_freelist_element
{
  struct stds_freelist_element
    *next;

  void
    *key,
    *value;
};

/***** public macros and prototypes *****/
void stds_freelist_init( struct stds_freelist_state *fs, void *user_state );
void stds_freelist_cleanup( struct stds_freelist_state *fs,
                            void (*element_pop_callback)(struct stds_freelist_state *fs, struct stds_freelist_element *fe) );

#define STDS_FREELIST_COMPILE_TIME_INIT( user_state )  { NULL, user_state }

#define STDS_FREELIST_GET_KEY_FROM_ELEMENT( freelist_element )             ( (freelist_element).key )
#define STDS_FREELIST_SET_KEY_IN_ELEMENT( freelist_element, new_key )      ( (freelist_element).key = (void *) (stds_pal_uint_t) (new_key) )
#define STDS_FREELIST_GET_VALUE_FROM_ELEMENT( freelist_element )           ( (freelist_element).value )
#define STDS_FREELIST_SET_VALUE_IN_ELEMENT( freelist_element, new_value )  ( (freelist_element).value = (void *) (stds_pal_uint_t) (new_value) )
#define STDS_FREELIST_GET_USER_STATE_FROM_STATE( freelist_state )          ( (freelist_state).user_state )

#define STDS_FREELIST_PUSH( freelist_state, freelist_element )             {                                                  \
                                                                             (freelist_element).next = (freelist_state).top;  \
                                                                             (freelist_state).top = &(freelist_element);      \
                                                                           }


#define STDS_FREELIST_POP( freelist_state, pointer_to_freelist_element )   {                                                          \
                                                                             if( (freelist_state).top == NULL )                       \
                                                                               (pointer_to_freelist_element) = NULL;                  \
                                                                                                                                      \
                                                                             if( (freelist_state).top != NULL )                       \
                                                                             {                                                        \
                                                                               (pointer_to_freelist_element) = (freelist_state).top;  \
                                                                               (freelist_state).top = (freelist_state).top->next;     \
                                                                             }                                                        \
                                                                           }

void stds_freelist_push_array_of_new_elements( struct stds_freelist_state *fs, void *array, stds_pal_uint_t element_size_in_bytes, stds_pal_uint_t offsetof_freelist_element, stds_pal_uint_t number_elements );

void stds_freelist_query( struct stds_freelist_state *fs, enum stds_freelist_query_type query_type, void *query_input, void *query_output );

