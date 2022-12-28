if(NOT GITVERSION_EXECUTE)
  find_package(Git)

  if(Git_FOUND)
    set(GITVERSION_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")

    function(gitversion_create_target name)
      message(STATUS "Creating gitversion target ${name}")

      set(GITVERSION_GENERATED_INCLUDE_DIR "${GITVERSION_GENERATED_DIR}/include")
      set(GITVERSION_GENERATED_DUMMY_FILE "${GITVERSION_GENERATED_INCLUDE_DIR}/_${name}.h")
      set(GITVERSION_GENERATED_FILE "${GITVERSION_GENERATED_INCLUDE_DIR}/${name}.h")

      # Use a target that depends on the output of the command to prevent the
      # command from running more than once.
      # https://gitlab.kitware.com/cmake/cmake/-/issues/16767 We add a dummy
      # output to force the command to run every build.
      # https://www.mattkeeter.com/blog/2018-01-06-versioning/
      add_custom_command(
        OUTPUT
          "${GITVERSION_GENERATED_FILE}"
          "${GITVERSION_GENERATED_DUMMY_FILE}"
        COMMENT "Generating gitversion header."
        COMMAND
          "${CMAKE_COMMAND}"
            -D GIT=\"${GIT_EXECUTABLE}\"
            -D GITVERSION_NAME=${name}
            -D GITVERSION_GENERATED_DIR=\"${GITVERSION_GENERATED_DIR}\"
            -D GITVERSION_GENERATED_FILE=\"${GITVERSION_GENERATED_FILE}\"
            -D GITVERSION_EXECUTE=true
            -P cmake/GitVersion.cmake
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      )

      add_custom_target(${name}_intermediate
        DEPENDS "${GITVERSION_GENERATED_FILE}"
      )
      add_library(${name} INTERFACE)
      add_dependencies(${name} ${name}_intermediate)

      target_include_directories(${name}
        INTERFACE "$<BUILD_INTERFACE:${GITVERSION_GENERATED_INCLUDE_DIR}>"
      )

      string(TOUPPER ${name} upper_name)
      target_compile_definitions(${name}
        INTERFACE "$<BUILD_INTERFACE:${upper_name}_GITVERSION=1>")
    endfunction()

    function(gitversion_set_version name prefix)
      set(_describe "")

      execute_process(
        COMMAND
          "${GIT_EXECUTABLE}" describe --tags --match "${prefix}*" --abbrev=0
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _describe
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _result
        ERROR_QUIET
      )

      if(_describe MATCHES "${prefix}*")
        string(REPLACE "${prefix}" "" _version "${_describe}")
        set(${name} "${_version}" PARENT_SCOPE)
      else()
        message(WARNING "Unable to determine version.")
        unset(${name} PARENT_SCOPE)
      endif()

    endfunction()
  else()
    message(WARNING "Git not found.")

    function(gitversion_set_version name prefix)
      unset(${name} PARENT_SCOPE)
    endfunction()
  endif()

  return()
endif()

# Executed during build (NOT configuration) to create/update the generated
# header.  It will only update it if there was a change.

set(GITVERSION_GENERATED_TMP_DIR "${GITVERSION_GENERATED_DIR}/tmp")
set(GITVERSION_GENERATED_INCLUDE_DIR "${GITVERSION_GENERATED_DIR}/include")

set(GITVERSION_GENERATED_IN_FILE
    "${GITVERSION_GENERATED_TMP_DIR}/${GITVERSION_NAME}.h.in")

file(MAKE_DIRECTORY ${GITVERSION_GENERATED_TMP_DIR})
file(MAKE_DIRECTORY ${GITVERSION_GENERATED_INCLUDE_DIR})

string(TOUPPER "${GITVERSION_NAME}" upper_name)

# Create the template file for "git show"
set(template
"#define ${upper_name}_LONG \"%H\"
#define ${upper_name}_TIME \"%cd\"
#cmakedefine ${upper_name}_REPO_URL \"@${upper_name}_REPO_URL@\"
#cmakedefine ${upper_name}_REPO_BRANCH \"@${upper_name}_REPO_BRANCH@\"
#cmakedefine ${upper_name}_DESCRIBE \"@${upper_name}_DESCRIBE@\"
#cmakedefine ${upper_name}_MASTER_MERGE_BASE \"@${upper_name}_MASTER_MERGE_BASE@\"
#cmakedefine ${upper_name}_DESCRIBE_MERGE_BASE \"@${upper_name}_DESCRIBE_MERGE_BASE@\"
")

if(WIN32)
  string(APPEND template
      "#cmakedefine ${upper_name}_WINDOWS_SDK_VERSION \"@${upper_name}_WINDOWS_SDK_VERSION@\"%n" )

  set(GITVERSION_WINDOWS_SDK_VERSION $ENV{WindowsSDKVersion})
  string(REPLACE "\\" "" GITVERSION_WINDOWS_SDK_VERSION "${GITVERSION_WINDOWS_SDK_VERSION}" )  # Sometimes it has trailing slash (!)
endif()

execute_process(
  COMMAND
    "${GIT}" show -s "--format=${template}"
  OUTPUT_FILE
    "${GITVERSION_GENERATED_IN_FILE}"
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
 )

execute_process(
  COMMAND
    "${GIT}" config --get remote.origin.url
  OUTPUT_VARIABLE GITVERSION_REPO_URL
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
 )

execute_process(
  COMMAND
    "${GIT}" symbolic-ref --short HEAD
  OUTPUT_VARIABLE GITVERSION_REPO_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
 )

execute_process(
  COMMAND
    "${GIT}" describe --tags --always --dirty --exclude "latest-*" --exclude "build-*"
  OUTPUT_VARIABLE GITVERSION_DESCRIBE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
 )

if(NOT "${GITVERSION_REPO_BRANCH}" STREQUAL "master"
   AND NOT "${GITVERSION_REPO_BRANCH}" STREQUAL "main")
  execute_process(
    COMMAND
      "${GIT}" merge-base origin/master HEAD
    OUTPUT_VARIABLE GITVERSION_MASTER_MERGE_BASE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if(NOT ${GITVERSION_MASTER_MERGE_BASE} STREQUAL "")
    execute_process(
      COMMAND
        "${GIT}" describe --tags --always ${GITVERSION_MASTER_MERGE_BASE}
      OUTPUT_VARIABLE GITVERSION_DESCRIBE_MERGE_BASE
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
  endif()
endif()

message(STATUS "GITVERSION_REPO_URL: ${GITVERSION_REPO_URL}")
message(STATUS "GITVERSION_REPO_BRANCH: ${GITVERSION_REPO_BRANCH}")
message(STATUS "GITVERSION_DESCRIBE: ${GITVERSION_DESCRIBE}")
message(STATUS "GITVERSION_MASTER_MERGE_BASE: ${GITVERSION_MASTER_MERGE_BASE}")
message(STATUS "GITVERSION_DESCRIBE_MERGE_BASE: ${GITVERSION_DESCRIBE_MERGE_BASE}")

if(WIN32)
  message(STATUS "GITVERSION_WINDOWS_SDK_VERSION: ${GITVERSION_WINDOWS_SDK_VERSION}")
endif()

set(${upper_name}_REPO_URL "${GITVERSION_REPO_URL}")
set(${upper_name}_REPO_BRANCH "${GITVERSION_REPO_BRANCH}")
set(${upper_name}_DESCRIBE "${GITVERSION_DESCRIBE}")
set(${upper_name}_MASTER_MERGE_BASE "${GITVERSION_MASTER_MERGE_BASE}")
set(${upper_name}_DESCRIBE_MERGE_BASE "${GITVERSION_DESCRIBE_MERGE_BASE}")
if(WIN32)
    set(${upper_name}_WINDOWS_SDK_VERSION "${GITVERSION_WINDOWS_SDK_VERSION}")
endif()

configure_file("${GITVERSION_GENERATED_IN_FILE}" "${GITVERSION_GENERATED_FILE}")
