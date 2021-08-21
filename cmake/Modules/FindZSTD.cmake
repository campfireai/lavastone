# - Find ZSTD
#
# ZSTD_INCLUDE_DIR - Where to find ZSTD/zstd.h
# ZSTD_LIBRARIES - List of libraries when using ZSTD.
# ZSTD_FOUND - True if ZSTD found.

find_path(ZSTD_INCLUDE_DIR zstd.h
  HINTS $ENV{ZSTD_ROOT}/include
  DOC "Path in which the file ZSTD/zstd.h is located." )

find_library(ZSTD_LIBRARIES ZSTD
  HINTS $ENV{ZSTD_ROOT}/lib
  DOC "Path to ZSTD library." )

mark_as_advanced(ZSTD_INCLUDE_DIR ZSTD_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZSTD DEFAULT_MSG ZSTD_LIBRARIES ZSTD_INCLUDE_DIR)
