# Copyright (C) 2016-2018 IPVision Canada Inc.
# Created by Hasan Ibna Akbar <bd.pothik@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
#	Note:
#		This is copied and changed from FindSQLite3.cmake file.
#		See that file for more information.

# - Find maxminddb
# Find the native LIBMAXMINDDB headers and libraries.
#
# LIBMAXMINDDB_INCLUDE_DIR	- where to find maxminddb.h, etc.
# LIBMAXMINDDB_LIBRARY	- List of libraries when using maxminddb.
# LIBMAXMINDDB_FOUND	- True if sqlite found.

# Look for the header file.
FIND_PATH(MAXMINDDB_INCLUDE_DIR NAMES maxminddb.h)

# Look for the library.
FIND_LIBRARY(MAXMINDDB_LIBRARY NAMES maxminddb)

# Handle the QUIETLY and REQUIRED arguments and set SQLITE3_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MAXMINDDB DEFAULT_MSG MAXMINDDB_LIBRARY MAXMINDDB_INCLUDE_DIR)

# Copy the results to the output variables.
IF(MAXMINDDB_FOUND)
	SET(MAXMINDDB_LIBRARIES ${MAXMINDDB_LIBRARY})
	SET(MAXMINDDB_INCLUDE_DIRS ${MAXMINDDB_INCLUDE_DIR})
ELSE(MAXMINDDB_FOUND)
	SET(MAXMINDDB_LIBRARIES)
	SET(MAXMINDDB_INCLUDE_DIRS)
ENDIF(MAXMINDDB_FOUND)

MARK_AS_ADVANCED(MAXMINDDB_INCLUDE_DIRS MAXMINDDB_LIBRARIES)
