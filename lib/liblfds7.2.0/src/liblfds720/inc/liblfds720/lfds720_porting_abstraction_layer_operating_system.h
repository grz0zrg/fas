/* TRD : the internal single-threaded library includes this file
         so it does not have to have the user repeat the work
         so this file has to be protected from double inclusion
         or the "more than one amtching layer" error kicks in
*/

#ifndef LFDS720_PAL_OPERATING_SYSTEM

  /****************************************************************************/
  #if( defined _WIN32 && !defined KERNEL_MODE )

    #ifdef LFDS720_PAL_OPERATING_SYSTEM
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_operating_system.h".
    #endif

    #define LFDS720_PAL_OPERATING_SYSTEM

    #include <assert.h>

    #define LFDS720_PAL_OS_STRING             "Windows"
    #define LFDS720_PAL_ASSERT( expression )  if( !(expression) ) LFDS720_MISC_DELIBERATELY_CRASH;

  #endif





  /****************************************************************************/
  #if( defined _WIN32 && defined KERNEL_MODE )

    #ifdef LFDS720_PAL_OPERATING_SYSTEM
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_operating_system.h".
    #endif

    #define LFDS720_PAL_OPERATING_SYSTEM

    #include <assert.h>
    #include <wdm.h>

    #define LFDS720_PAL_OS_STRING             "Windows"
    #define LFDS720_PAL_ASSERT( expression )  if( !(expression) ) LFDS720_MISC_DELIBERATELY_CRASH;

  #endif





  /****************************************************************************/
  #if( defined __linux__ && !defined KERNEL_MODE )

    #ifdef LFDS720_PAL_OPERATING_SYSTEM
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_operating_system.h".
    #endif

    #define LFDS720_PAL_OPERATING_SYSTEM

    #include <stddef.h>

    #define LFDS720_PAL_OS_STRING             "Linux"
    #define LFDS720_PAL_ASSERT( expression )  if( !(expression) ) LFDS720_MISC_DELIBERATELY_CRASH;

  #endif





  /****************************************************************************/
  #if( defined __linux__ && defined KERNEL_MODE )

    #ifdef LFDS720_PAL_OPERATING_SYSTEM
      #error More than one porting abstraction layer matches the current platform in "lfds720_porting_abstraction_layer_operating_system.h".
    #endif

    #define LFDS720_PAL_OPERATING_SYSTEM

    #include <linux/module.h>

    #define LFDS720_PAL_OS_STRING             "Linux"
    #define LFDS720_PAL_ASSERT( expression )  BUG_ON( expression )

  #endif





  /****************************************************************************/
  #if( !defined LFDS720_PAL_OPERATING_SYSTEM )

    #error No matching porting abstraction layer in lfds720_porting_abstraction_layer_operating_system.h

  #endif

#endif

