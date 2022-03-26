if(NOT DUNE_VERSION_EXECUTE)
    if(NOT DUNE_GENERATED_TMP_DIR)
        set(DUNE_GENERATED_TMP_DIR ${CMAKE_BINARY_DIR}/generated/tmp)
    endif()
    if(NOT DUNE_GENERATED_INCLUDE_DIR)
        set(DUNE_GENERATED_INCLUDE_DIR ${CMAKE_BINARY_DIR}/generated/include)
    endif()

    function(create_git_version_target)
        # Use a target that depends on the output of the command to
        # prevent the command from running more than once.
        # https://gitlab.kitware.com/cmake/cmake/-/issues/16767
        # We add a dummy output to force the command to run
        # every build.
        # https://www.mattkeeter.com/blog/2018-01-06-versioning/
        add_custom_command(
        OUTPUT ${DUNE_GENERATED_INCLUDE_DIR}/dune_version.h
               ${DUNE_GENERATED_TMP_DIR}/_dune_version.h
        COMMAND
            ${CMAKE_COMMAND}
                    -D GIT=${GIT_EXECUTABLE}
                    -D DUNE_GENERATED_TMP_DIR=${DUNE_GENERATED_TMP_DIR}
		            -D DUNE_GENERATED_INCLUDE_DIR=${DUNE_GENERATED_INCLUDE_DIR}
		            -D DUNE_VERSION_EXECUTE=true
		            -P cmake/GitVersion.cmake
        WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}
        )
        add_custom_target(
            git_version_intermediate
            DEPENDS ${DUNE_GENERATED_INCLUDE_DIR}/dune_version.h
        )
        add_library(git_version INTERFACE)
        add_dependencies(git_version git_version_intermediate)
        target_include_directories(git_version INTERFACE ${DUNE_GENERATED_INCLUDE_DIR})
  endfunction()

  return()
endif()


# Executed during build (NOT configuration) to create/update the
# dune_version.h header.  It will only update it if there was
# a change.

set(template "#define DUNE_GIT_LONG \"%H\"%n#define DUNE_GIT_TIME \"%cd\"%n" )
string(APPEND template "#cmakedefine DUNE_GIT_REPO_URL \"@DUNE_GIT_REPO_URL@\"%n#cmakedefine DUNE_GIT_REPO_BRANCH \"@DUNE_GIT_REPO_BRANCH@\"%n" )
string(APPEND template "#cmakedefine DUNE_GIT_DESCRIBE \"@DUNE_GIT_DESCRIBE@\"%n" )
string(APPEND template "#cmakedefine DUNE_GIT_MASTER_MERGE_BASE \"@DUNE_GIT_MASTER_MERGE_BASE@\"%n" )
string(APPEND template "#cmakedefine DUNE_GIT_DESCRIBE_MERGE_BASE \"@DUNE_GIT_DESCRIBE_MERGE_BASE@\"%n" )

if(WIN32 )
    string(APPEND template "#cmakedefine DUNE_WINDOWS_SDK_VERSION \"@DUNE_WINDOWS_SDK_VERSION@\"%n" )

    set(DUNE_WINDOWS_SDK_VERSION $ENV{WindowsSDKVersion})
    string(REPLACE "\\" "" DUNE_WINDOWS_SDK_VERSION "${DUNE_WINDOWS_SDK_VERSION}" )  # Sometimes it has trailing slash (!)
endif()

file(MAKE_DIRECTORY ${DUNE_GENERATED_TMP_DIR})
file(MAKE_DIRECTORY ${DUNE_GENERATED_INCLUDE_DIR})

execute_process(
   COMMAND
      ${GIT} show -s "--format=${template}"
   OUTPUT_FILE
      ${DUNE_GENERATED_TMP_DIR}/dune_version.h.in
   OUTPUT_STRIP_TRAILING_WHITESPACE
   ERROR_QUIET
 )

execute_process(
   COMMAND
      ${GIT} config --get remote.origin.url
   OUTPUT_VARIABLE DUNE_GIT_REPO_URL
   OUTPUT_STRIP_TRAILING_WHITESPACE
   ERROR_QUIET
 )

execute_process(
   COMMAND
      ${GIT} symbolic-ref --short HEAD
   OUTPUT_VARIABLE DUNE_GIT_REPO_BRANCH
   OUTPUT_STRIP_TRAILING_WHITESPACE
   ERROR_QUIET
 )

execute_process(
   COMMAND
      ${GIT} describe --tags --always --dirty --exclude "latest-*" --exclude "build-*"
   OUTPUT_VARIABLE DUNE_GIT_DESCRIBE
   OUTPUT_STRIP_TRAILING_WHITESPACE
   ERROR_QUIET
 )

if(NOT "${DUNE_GIT_REPO_BRANCH}" STREQUAL "master" )
    execute_process(
       COMMAND
          ${GIT} merge-base origin/master HEAD
       OUTPUT_VARIABLE DUNE_GIT_MASTER_MERGE_BASE
       OUTPUT_STRIP_TRAILING_WHITESPACE
       ERROR_QUIET
    )
    if(NOT ${DUNE_GIT_MASTER_MERGE_BASE} STREQUAL "" )
        execute_process(
           COMMAND
              ${GIT} describe --tags --always ${DUNE_GIT_MASTER_MERGE_BASE}
           OUTPUT_VARIABLE DUNE_GIT_DESCRIBE_MERGE_BASE
           OUTPUT_STRIP_TRAILING_WHITESPACE
           ERROR_QUIET
        )
    endif()
endif()

message(STATUS "DUNE_GIT_REPO_URL: ${DUNE_GIT_REPO_URL}" )
message(STATUS "DUNE_GIT_REPO_BRANCH: ${DUNE_GIT_REPO_BRANCH}" )
message(STATUS "DUNE_GIT_DESCRIBE: ${DUNE_GIT_DESCRIBE}" )
message(STATUS "DUNE_GIT_MASTER_MERGE_BASE: ${DUNE_GIT_MASTER_MERGE_BASE}" )
message(STATUS "DUNE_GIT_DESCRIBE_MERGE_BASE: ${DUNE_GIT_DESCRIBE_MERGE_BASE}" )

if(WIN32)
    message(STATUS "DUNE_WINDOWS_SDK_VERSION: ${DUNE_WINDOWS_SDK_VERSION}" )
endif()

configure_file(${DUNE_GENERATED_TMP_DIR}/dune_version.h.in ${DUNE_GENERATED_INCLUDE_DIR}/dune_version.h )
