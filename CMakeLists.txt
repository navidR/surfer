cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(surfer)

# Compiler Options
set (CMAKE_CXX_STANDARD 11)
set (_JOB 16)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(main_executable surfer)
set(thirdparty ${CMAKE_SOURCE_DIR}/thirdparty)
set(linux_libs ssh2 ssl crypto z)

# Threads
#find_package(CURL)

# Boost Library
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS filesystem log program_options system log_setup date_time thread regex chrono atomic REQUIRED)

message(STATUS "Boost library directories : ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost library : ${Boost_LIBRARIES}")
message(STATUS "Boost Include directories: ${Boost_INCLUDE_DIRS}")

#link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

# Add test executable target
add_executable(${main_executable} ${CMAKE_SOURCE_DIR}/src/main.cpp)

# Enable ExternalProject CMake module
include(ExternalProject)

# Set the build type if it isn't already
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# curl version
if(NOT CURL_VERSION)
  set(CURL_VERSION 7_52_1)
  message(STATUS "set libcurl version to default version : ${CURL_VERSION}")
endif()
message(STATUS "libcurl version set : ${CURL_VERSION}" )

# Config for curl
set(BUILD_CURL_EXE off)
set(CURL_STATICLIB on)
set(ENABLE_ARES off)
set(CURL_DISABLE_LDAP on)
set(CURL_DISABLE_TELNET on)
set(CURL_DISABLE_TFTP on)
set(CURL_DISABLE_LDAPS on)
set(CURL_DISABLE_RTSP on)
set(CURL_DISABLE_POP3 on)
set(CURL_DISABLE_IMAP on)
set(CURL_DISABLE_SMTP on)
set(CURL_DISABLE_GOPHER on)
set(CURL_DISABLE_CRYPTO_AUTH on)
set(ENABLE_IPV6 off)
set(ENABLE_MANUAL off)

set(curl_configure_options "--disable-shared --without-ssl --disable-ares --disable-cookies --disable-crypto-auth --disable-ipv6 --disable-manual --disable-versioned-symbols --without-librtmp --without-zlib")

# Download and unzip libcurl
set(curl_zip "curl-${CURL_VERSION}.zip")
set(curl_folder libcurl)
set(curl_url "https://github.com/curl/curl/archive/${curl_zip}")
set(download_path ${thirdparty}/${curl_zip})
set(extract_path ${thirdpaty}/${curl_folder})

if (NOT EXISTS "${download_path}")
    file(DOWNLOAD "${curl_url}" "${download_path}")
endif()

add_custom_command(
    OUTPUT "${extract_path}"
    COMMAND unzip
    DEPENDS "${download_path}")

# https://github.com/curl/curl/archive/curl-7_52_1.zip Library
# Add Curl
ExternalProject_Add(
    libcurl
    SOURCE_DIR ${thirdparty}/libcurl
    URL ${thirdparty}/curl-${CURL_VERSION}.zip
    TMP_DIR ${CMAKE_SOURCE_DIR}/build
    #GIT_REPOSITORY git@github.com:google/libcurl.git
    #GIT_TAG release-${CURL_VERSION}
    #GIT_SHALLOW
    #TIMEOUT 10
    #BUILD_ALWAYS 1
    #UPDATE_DISCONNECTED 1
    # Force separate output paths for debug and release builds to allow easy
    # identification of correct lib in subsequent TARGET_LINK_LIBRARIES commands
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
              -DBUILD_CURL_EXE=${BUILD_CURL_EXE}
              -DCURL_STATICLIB=${CURL_STATICLIB}
              -DENABLE_ARES=${ENABLE_ARES}
              -DCURL_DISABLE_LDAP=${CURL_DISABLE_LDAP}
              -DCURL_DISABLE_TELNET=${CURL_DISABLE_TELNET}
              -DCURL_DISABLE_TFTP=${CURL_DISABLE_TFTP}
              -DCURL_DISABLE_LDAPS=${CURL_DISABLE_LDAPS}
              -DCURL_DISABLE_RTSP=${CURL_DISABLE_RTSP}
              -DCURL_DISABLE_POP3=${CURL_DISABLE_POP3}
              -DCURL_DISABLE_IMAP=${CURL_DISABLE_IMAP}
              -DCURL_DISABLE_SMTP=${CURL_DISABLE_SMTP}
              -DCURL_DISABLE_GOPHER=${CURL_DISABLE_GOPHER}
              -DCURL_DISABLE_CRYPTO_AUTH=${CURL_DISABLE_CRYPTO_AUTH}
              -DENABLE_IPV6=${ENABLE_IPV6}
              -DENABLE_MANUAL=${ENABLE_MANUAL}
               #-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=curl_debug_output
               #-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=curl_release_output
    # Disable install step
    INSTALL_COMMAND ""
    BUILD_COMMAND "make" -j${_JOB}
    UPDATE_COMMAND ""
    # Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

# Specify include dir
ExternalProject_Get_Property(libcurl source_dir)
include_directories(${source_dir}/libcurl/include)

# Create dependency of MainTest on libcurl
add_dependencies(${main_executable} libcurl)

# Specify MainTest's link libraries
ExternalProject_Get_Property(libcurl binary_dir)

set(source_dir_libcurl ${source_dir})
set(binary_dir_libcurl ${binary_dir})
message(STATUS "libcurl source directory : ${source_dir_libcurl}")
message(STATUS "libcurl binary directory : ${binary_dir_libcurl}")

# Final Executable Config
if(UNIX)
    set(suffix ".a")
elseif(WIN32)
    set(suffix ".lib")
endif()

message(STATUS "suffix for this platform : ${suffix}")

if(APPLE)
    message(STATUS "Link and Compile on Apple")
    target_link_libraries(
        ${main_executable}
        ${Boost_LIBRARIES}
        optimized ${binary_dir_libcurl}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}curl${suffix}
        debug ${binary_dir_libcurl}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}curl${suffix}
        ssh2 ssl crypto z
    )
endif()

if(UNIX AND NOT APPLE)
    message(STATUS "Link and Compile on UNIX")
    target_link_libraries(
        ${main_executable}
        ${Boost_LIBRARIES}
        optimized ${binary_dir_libcurl}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}curl${suffix}
        debug ${binary_dir_libcurl}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}curl${suffix}
        ${linux_libs}
    )
endif()

add_custom_target(
  run
  COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${main_executable}
)