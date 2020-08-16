set(FIX_MATH_SOURCES
	fixmath/fix16.c
	fixmath/fix16_exp.c
	fixmath/fix16_sqrt.c
	fixmath/fix16_str.c
	fixmath/fix16_trig.c
	fixmath/fix32.c
	fixmath/fix32_exp.c
	fixmath/fix32_sqrt.c
	fixmath/fix32_str.c
	fixmath/fix32_trig.c
	)

if(MSVC)
	list(APPEND FIX_MATH_SOURCES fixmath/fixmath.natvis)
endif()

set(FIX_MATH_HEADERS
	../../include/utility/fixmath/fix16.h
	../../include/utility/fixmath/fix16_trig_sin_lut.h
	../../include/utility/fixmath/fix32.h
	../../include/utility/fixmath/fixmath.h
	../../include/utility/fixmath/FixPoint.h
	../../include/utility/fixmath/FixPoint16.h
	../../include/utility/fixmath/FixPoint32.h
	../../include/utility/fixmath/int64.h
)

set(SOURCES
	misc/Random.cpp
	misc/string_util.cpp
)

set(HEADERS
	../../include/utility/misc/exceptions.h
	../../include/utility/misc/InputStream.h
	../../include/utility/misc/OutputStream.h
	../../include/utility/misc/RobustList.h
	../../include/utility/misc/Random.h
	../../include/utility/misc/string_util.h
	../../include/utility/misc/lemire_uniform_uint32_distribution.h
	../../include/utility/misc/random_xoroshiro128plus.h
	../../include/utility/misc/random_xorshift1024star.h
	../../include/utility/misc/random_xoshiro256starstar.h
	../../include/utility/misc/reverse.h
)

find_package(fmt CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)

set(LIBS
	digestpp
	fmt::fmt
	Microsoft.GSL::GSL
)


add_library(utility ${SOURCES} ${FIX_MATH_SOURCE} ${HEADERS} ${FIX_MATH_HEADERS})
target_include_directories(utility PUBLIC ../../include/utility)
target_link_libraries(utility PUBLIC ${LIBS})
