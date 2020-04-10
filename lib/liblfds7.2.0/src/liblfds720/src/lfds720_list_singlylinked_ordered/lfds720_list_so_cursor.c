/***** includes *****/
#include "lfds720_list_singlylinked_unordered_internal.h"





/****************************************************************************/
void lfds720_list_so_init_cursor( struct lfds720_list_so_state *lsos,
                                  struct lfds720_list_so_cursor *lsuc )
{
  LFDS720_PAL_ASSERT( lsos != NULL );
  LFDS720_PAL_ASSERT( lsuc != NULL );

  lfds720_smrhp_register_thread_using_new_smrhp_thread_state( lsos->smrhps, &lsuc->smrhpts );

  return;
}





/****************************************************************************/
void lfds720_list_so_cleanup_cursor( struct lfds720_list_so_state *lsos,
                                     struct lfds720_list_so_cursor *lsuc )
{
  LFDS720_PAL_ASSERT( lsos != NULL );
  LFDS720_PAL_ASSERT( lsuc != NULL );

  lfds720_smrhp_deregister_thread( lsos->smrhps, &lsuc->smrhpts );

  return;
}

