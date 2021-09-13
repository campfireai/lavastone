
find_path(ROCKSDB_ROOT_DIR
    NAMES include/rocksdb/db.h
)

find_library(ROCKSDB_LIBRARIES
    NAMES rocksdb
    HINTS ${ROCKSDB_ROOT_DIR}/lib
)

find_path(ROCKSDB_INCLUDE_DIRS
    NAMES rocksdb/db.h
    HINTS ${ROCKSDB_ROOT_DIR}/include
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ROCKSDB DEFAULT_MSG
    ROCKSDB_LIBRARIES
    ROCKSDB_INCLUDE_DIRS
)

mark_as_advanced(
    ROCKSDB_ROOT_DIR
    ROCKSDB_LIBRARIES
    ROCKSDB_INCLUDE_DIRS
)
