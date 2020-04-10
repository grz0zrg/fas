#include "stds_freelist_internal.h"





/****************************************************************************/
void stds_freelist_push_array_of_new_elements( struct stds_freelist_state *fs, void *array, stds_pal_uint_t element_size_in_bytes, stds_pal_uint_t offsetof_freelist_element, stds_pal_uint_t number_elements )
{
  struct stds_freelist_element
    *fe,
    *fe_next,
    *fe_last;

  stds_pal_uint_t
    index,
    loop;

  void
    *value;

  STDS_PAL_ASSERT( fs != NULL );
  STDS_PAL_ASSERT( array != NULL );
  STDS_PAL_ASSERT( element_size_in_bytes >= sizeof(struct stds_freelist_element) );
  STDS_PAL_ASSERT( offsetof_freelist_element <= element_size_in_bytes - sizeof(struct stds_freelist_element) );
  STDS_PAL_ASSERT( number_elements >= 1 );

  /* TRD : starting with the final element, we iterate backwards over the array
           extracting the freelist struct from each element
           and setting its next pointer to the following element
           (NULL for the final element)
           and also setting the counter
  */

  index = number_elements - 1;

  value = ( (unsigned char *) array + element_size_in_bytes * index );
  fe = (struct stds_freelist_element *)( (unsigned char *) array + element_size_in_bytes * index + offsetof_freelist_element );

  fe->next = NULL;
  fe->value = value;

  fe_last = fe_next = fe;

  /* TRD : we loop the correct number of times
           but we use index, which decrements, to compute the offset
           into the array
  */

  for( loop = 0 ; loop < (number_elements-1) ; loop++ )
  {
    value = ( (unsigned char *) array + element_size_in_bytes * --index );
    fe = (struct stds_freelist_element *)( (unsigned char *) array + element_size_in_bytes * index + offsetof_freelist_element );

    fe->next = fe_next;
    fe->value = value;
    fe_next = fe;
  }

  /* TRD : fe now points to first element in array
           so we set fe_last->next to current freelist head
           (e.g. place the entire new array on top of the existing freelist)
           set freelist head to fe
           where fe is the first element in our array of new elements
  */

  fe_last->next = fs->top;
  fs->top = fe;

  return;
}

