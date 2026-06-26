
# Headless smoke test with fixture
set(HEADLESS_TEST_SOURCES
	HeadlessTestFixture.cpp
	headless_smoke_test.cpp
)

add_executable(headless_test ${HEADLESS_TEST_SOURCES})
target_include_directories(headless_test PRIVATE ../../include)
dune_link_dune_and_core(headless_test)
target_link_libraries(headless_test PRIVATE GTest::gtest GTest::gtest_main)

set(HEADLESS_DATA_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/../../data/OPENSD2.PAK"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../data/LEGACY.PAK"
)

set(HEADLESS_OUTPUT_FILES)
foreach(f ${HEADLESS_DATA_FILES})
	get_filename_component(name "${f}" NAME)
	list(APPEND HEADLESS_OUTPUT_FILES "${name}")
endforeach()

add_custom_target(copy_for_headless_tests ALL DEPENDS ${HEADLESS_OUTPUT_FILES})

add_custom_command(
	DEPENDS ${HEADLESS_DATA_FILES}
	OUTPUT ${HEADLESS_OUTPUT_FILES}
	COMMENT "Copying headless test data files"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different
	        ${HEADLESS_DATA_FILES}
	        ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM
)

add_dependencies(headless_test copy_for_headless_tests)

if(DUNE_PRECOMPILED_HEADERS)
	target_precompile_headers(headless_test PRIVATE ../../src/stdafx.h)
endif()

add_test(NAME headless COMMAND headless_test)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/HeadlessSimulationTest.cpp")
	list(APPEND HEADLESS_TEST_SOURCES HeadlessSimulationTest.cpp)
	add_executable(headless_simulation_test HeadlessTestFixture.cpp HeadlessSimulationTest.cpp)
	target_include_directories(headless_simulation_test PRIVATE ../../include)
	dune_link_dune_and_core(headless_simulation_test)
	target_link_libraries(headless_simulation_test PRIVATE GTest::gtest GTest::gtest_main)

	add_dependencies(headless_simulation_test copy_for_headless_tests)

	if(DUNE_PRECOMPILED_HEADERS)
		target_precompile_headers(headless_simulation_test PRIVATE ../../src/stdafx.h)
	endif()

	add_test(NAME headless_simulation COMMAND headless_simulation_test)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CampaignAIHeadlessTest.cpp")
	add_executable(campaign_ai_headless_test HeadlessTestFixture.cpp CampaignAIHeadlessTest.cpp)
	target_include_directories(campaign_ai_headless_test PRIVATE ../../include)
	dune_link_dune_and_core(campaign_ai_headless_test)
	target_link_libraries(campaign_ai_headless_test PRIVATE GTest::gtest GTest::gtest_main)

	add_dependencies(campaign_ai_headless_test copy_for_headless_tests)

	if(DUNE_PRECOMPILED_HEADERS)
		target_precompile_headers(campaign_ai_headless_test PRIVATE ../../src/stdafx.h)
	endif()

	add_test(NAME campaign_ai_headless COMMAND campaign_ai_headless_test)
endif()

# headless_hash_runner: spawned by the determinism test as an isolated subprocess.
# It initialises a headless game, steps N ticks, and prints the SHA-384 hex hash.
# Temporary: link dune while core/UI split is still in progress.
add_executable(headless_hash_runner headless_hash_runner.cpp)
target_include_directories(headless_hash_runner PRIVATE ../../include)
dune_link_dune_and_core(headless_hash_runner)
add_dependencies(headless_hash_runner copy_for_headless_tests)

if(DUNE_PRECOMPILED_HEADERS)
	target_precompile_headers(headless_hash_runner PRIVATE ../../src/stdafx.h)
endif()

# determinism_test: GTest that spawns headless_hash_runner twice and compares hashes.
# The runner path is injected via the HASH_RUNNER environment variable so the test
# can locate the binary regardless of working directory.
# Note: no DUNE_PRECOMPILED_HEADERS here — this target has no dune includes and
# must not inherit the dune stdafx PCH (which pulls SDL/SDL_mixer link deps).
add_executable(determinism_test DeterminismTest.cpp)
target_include_directories(determinism_test PRIVATE ../../include)
target_link_libraries(determinism_test PRIVATE GTest::gtest GTest::gtest_main)
add_dependencies(determinism_test headless_hash_runner copy_for_headless_tests)

add_test(NAME determinism COMMAND determinism_test)
set_tests_properties(determinism PROPERTIES
	ENVIRONMENT "HASH_RUNNER=$<TARGET_FILE:headless_hash_runner>"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
