FIND_PATH(LIBSOUNDTOUCH_INCLUDE_DIR NAMES SoundTouchDLL.h PATHS ${ROOT}/lib/soundtouch/source/SoundTouchDLL NO_DEFAULT_PATH)
FIND_LIBRARY(LIBSOUNDTOUCH_LIBRARIES NAMES SoundTouchDll PATHS ${ROOT}/lib/soundtouch/source/SoundTouchDLL NO_DEFAULT_PATH)

IF(LIBSOUNDTOUCH_INCLUDE_DIR AND LIBSOUNDTOUCH_LIBRARIES)
	SET(LIBSOUNDTOUCH_FOUND TRUE)
ENDIF(LIBSOUNDTOUCH_INCLUDE_DIR AND LIBSOUNDTOUCH_LIBRARIES)

IF(LIBSOUNDTOUCH_FOUND)
	IF (NOT LIBSOUNDTOUCH_FIND_QUIETLY)
		MESSAGE(STATUS "Found libsoundtouch includes: ${LIBSOUNDTOUCH_INCLUDE_DIR}/SoundTouchDLL.h")
		MESSAGE(STATUS "Found libsoundtouch library: ${LIBSOUNDTOUCH_LIBRARIES}")
	ENDIF (NOT LIBSOUNDTOUCH_FIND_QUIETLY)
ELSE(LIBSOUNDTOUCH_FOUND)
	IF (LIBSOUNDTOUCH_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could NOT find libsoundtouch development files")
	ENDIF (LIBSOUNDTOUCH_FIND_REQUIRED)
ENDIF(LIBSOUNDTOUCH_FOUND)
