#########################################
# Copied from Boost CMake (BSD Licensed)
# https://github.com/Orphis/boost-cmake
#########################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(NodeJS-Http-Parser VERSION 2.8.1 LANGUAGES C)

option(HTTP_PARSER_DOWNLOAD_TO_BINARY_DIR "Prefer downloading http-parser to the binary directory instead of source directory" OFF)

# Downloading to the binary dir provides a better isolation
# But in some circomstances, you want to share the download between multiple build folders
if(HTTP_PARSER_DOWNLOAD_TO_BINARY_DIR)
  set(DOWNLOAD_PREFIX "${CMAKE_CURRENT_BINARY_DIR}")
else()
  set(DOWNLOAD_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

if(IS_DIRECTORY ${HTTP_PARSER_ARCHIVE_DIRECTORY})
  message(STATUS "Using local directory to search for http-parser archive: ${HTTP_PARSER_ARCHIVE_DIRECTORY}")
  set(USING_LOCAL_ARCHIVE TRUE)
  set(ARCHIVE_PREFIX ${HTTP_PARSER_ARCHIVE_DIRECTORY})
else()
  set(USING_LOCAL_ARCHIVE FALSE)
  set(ARCHIVE_PREFIX ${DOWNLOAD_PREFIX})
endif()

get_filename_component(HTTP_PARSER_URL_FILENAME "${HTTP_PARSER_URL}" NAME)
set(DOWNLOAD_PATH "${ARCHIVE_PREFIX}/${HTTP_PARSER_URL_FILENAME}")
set(EXTRACT_PATH "${DOWNLOAD_PREFIX}/http-parser")

# Check if the provided folder points to a valid installation
if(HTTP_PARSER_SOURCE AND NOT IS_DIRECTORY "${HTTP_PARSER_SOURCE}")
  message(WARNING "HTTP_PARSER_SOURCE is pointing to a non existing folder, trying to download instead")
  unset(HTTP_PARSER_SOURCE CACHE)
endif()

if(HTTP_PARSER_SOURCE AND NOT EXISTS "${HTTP_PARSER_SOURCE}/http_parser.h")
  message(FATAL_ERROR "Folder ${HTTP_PARSER_SOURCE} points to an invalid http-parser source folder")
endif()

if(NOT HTTP_PARSER_SOURCE OR NOT EXISTS "${HTTP_PARSER_SOURCE}")
  # Check existing file
  if(EXISTS "${DOWNLOAD_PATH}")
    file(SHA256 "${DOWNLOAD_PATH}" LOCAL_SHA256)
    if(NOT LOCAL_SHA256 STREQUAL "${HTTP_PARSER_URL_SHA256}")
      # If the user specified a local archive, we fail permanently if the SHA doesn't match.
      # Furthermore, we do not remove local archives since we do not own them (they are maintained
      # by the user). We will not attempt to download archives if a local archive was specified.
      set(_error_msg "File ${DOWNLOAD_PATH} doesn't match the expected SHA256.\n Local ${LOCAL_SHA256} != Expected ${HTTP_PARSER_URL_SHA256}")
      if(NOT USING_LOCAL_ARCHIVE)
        message(WARNING ${_error_msg})
        file(REMOVE "${DOWNLOAD_PATH}")
      else()
        message(FATAL_ERROR ${_error_msg})
      endif()
    endif()
  endif()

  if(NOT EXISTS "${DOWNLOAD_PATH}")
  # Remove partially downloaded file
  file(REMOVE "${DOWNLOAD_PATH}.tmp")

  message(STATUS "Downloading http-parser from ${HTTP_PARSER_URL}")
  file(DOWNLOAD ${HTTP_PARSER_URL} "${DOWNLOAD_PATH}.tmp"
       EXPECTED_HASH SHA256=${HTTP_PARSER_URL_SHA256}
       STATUS download_status
       LOG download_log
       SHOW_PROGRESS
  )
  list(GET download_status 0 download_status_code)
  list(GET download_status 1 download_status_message)
  if(download_status_code)
    file(REMOVE "${DOWNLOAD_PATH}.tmp")
    message(FATAL_ERROR "Download failed. Status: ${download_status_code} ${download_status_message}\nLog: ${download_log}")
  endif()

  file(RENAME "${DOWNLOAD_PATH}.tmp" "${DOWNLOAD_PATH}")
endif()

set(HTTP_PARSER_LOCAL_SHA256 "${HTTP_PARSER_URL_SHA256}" CACHE STRING "Current SHA256 of http-parser archive used" FORCE)
mark_as_advanced(HTTP_PARSER_LOCAL_SHA256)

if(NOT IS_DIRECTORY "${EXTRACT_PATH}")
  message(STATUS "Extracting http-parser archive...")
  if(EXISTS "${EXTRACT_PATH}.tmp")
    file(REMOVE_RECURSE "${EXTRACT_PATH}.tmp")
  endif()
  file(MAKE_DIRECTORY "${EXTRACT_PATH}.tmp")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xfv ${DOWNLOAD_PATH}
    WORKING_DIRECTORY "${EXTRACT_PATH}.tmp"
    OUTPUT_QUIET
  )
  file(RENAME "${EXTRACT_PATH}.tmp" "${EXTRACT_PATH}")
  endif()

  set(HTTP_PARSER_SOURCE "${EXTRACT_PATH}" CACHE STRING "http-parser location" FORCE)
endif()

file(READ "${HTTP_PARSER_SOURCE}/Makefile" http_parser_makefile)
string(REGEX MATCH "SOMAJOR\s+=\s+([0-9]*)" _ ${http_parser_makefile})
set(HTTP_PARSER_VERS_MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "SOMINOR\s+=\s+([0-9]*)" _ ${http_parser_makefile})
set(HTTP_PARSER_VERS_MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "SOREV\s+=\s+([0-9]*)" _ ${http_parser_makefile})
set(HTTP_PARSER_VERS_PATCH ${CMAKE_MATCH_1})

set(HTTP_PARSER_VERSION "${HTTP_PARSER_VERS_MAJOR}.${HTTP_PARSER_VERS_MINOR}.${HTTP_PARSER_VERS_PATCH}")

message(STATUS "http-parser found: ${HTTP_PARSER_VERSION} ${HTTP_PARSER_SOURCE}")

add_library(nodejs_http_parser STATIC ${HTTP_PARSER_SOURCE}/http_parser.c)
add_library(NodeJS::http-parser ALIAS nodejs_http_parser)

target_include_directories(nodejs_http_parser ${HTTP_PARSER_SOURCE})
