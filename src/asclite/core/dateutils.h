/*
 * asclite
 * Author: Jerome Ajot, Nicolas Radde, Chris Laprun
 *
 * This software was developed at the National Institute of Standards and Technology by
 * employees of the Federal Government in the course of their official duties.  Pursuant to
 * Title 17 Section 105 of the United States Code this software is not subject to copyright
 * protection within the United States and is in the public domain. asclite is
 * an experimental system.  NIST assumes no responsibility whatsoever for its use by any party.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS."  With regard to this software, NIST MAKES NO EXPRESS
 * OR IMPLIED WARRANTY AS TO ANY MATTER WHATSOEVER, INCLUDING MERCHANTABILITY,
 * OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef DATE_UTILS
#define DATE_UTILS

#include "stdinc.h"

/**
 * A class with date-related methods.
 */
class DateUtils 
{
public:
	inline static string GetDateString()
    {
		time_t rawtime;
		time ( &rawtime );		
		return strtok( ctime( &rawtime ), "\n" ); // needed to remove terminating \n...
	}
};

#endif