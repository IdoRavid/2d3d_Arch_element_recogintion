// *****************************************************************************
// General settings for AddOn developments
// API Development Kit 23; Mac/Win
//
// Namespaces:		Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#ifndef	_APIENVIR_H_
#define	_APIENVIR_H_


#if defined (_MSC_VER)
	#if !defined (WINDOWS)
		#define WINDOWS
	#endif
#endif

#if defined (WINDOWS)
	#include "Win32Interface.hpp"
#endif

#if defined (macintosh)
	#include <CoreServices/CoreServices.h>
#endif

#if !defined (ACExtension)
	#define ACExtension
#endif


#endif
