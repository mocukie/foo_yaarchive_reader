include(ExternalProject)

ExternalProject_Add(
    zlib
    PREFIX ${CMAKE_BINARY_DIR}
    INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/install
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.2.11
    CMAKE_ARGS
    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
    "-DBUILD_SHARED_LIBS=on"
    PATCH_COMMAND git reset --hard && git apply ${CMAKE_CURRENT_LIST_DIR}/zlib-prefix.patch
)
ExternalProject_Get_Property(zlib install_dir)
SET(ZLIB_BUILD "${install_dir}")


ExternalProject_Add(
    liblzma
    PREFIX ${CMAKE_BINARY_DIR}
    INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/install
    GIT_REPOSITORY https://github.com/xz-mirror/xz.git
    GIT_TAG v5.2.5
    CMAKE_ARGS
    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
)
ExternalProject_Get_Property(liblzma install_dir)
SET(LibLZMA_BUILD "${install_dir}")

ExternalProject_Add(
    libarchive
    PREFIX ${CMAKE_BINARY_DIR}
    INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/install
    GIT_REPOSITORY https://github.com/libarchive/libarchive.git
    GIT_TAG v3.4.3
    CMAKE_ARGS
    "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
    "-DZLIB_ROOT=${ZLIB_BUILD}"
    "-DLibLZMA_ROOT=${LibLZMA_BUILD}"
    "-DENABLE_ZLIB=on"
    "-DENABLE_LZMA=on"
    "-DENABLE_TEST=off"
    "-DENABLE_ICONV=off"
    "-DENABLE_TAR=off"
    "-DENABLE_CPIO=off"
    "-DENABLE_CAT=off"
    "-DENABLE_OPENSSL=off"
    "-DENABLE_LIBB2=off"
    "-DENABLE_LZ4=off"
    "-DENABLE_ZSTD=off"
    "-DENABLE_BZip2=off"
    "-DENABLE_LIBXML2=off"
    "-DENABLE_EXPAT=off"
    "-DENABLE_CNG=off"
    "-DENABLE_PCREPOSIX=off"
    "-DCMAKE_SHARED_LINKER_FLAGS=-static-libgcc"
)