cmake_minimum_required(VERSION 3.24)

project(mongoose_memory_leak)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RULE_MESSAGES ON)
set(CMAKE_INSTALL_MESSAGE LAZY)

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()

list(APPEND extra_flags
    -Wcast-align=strict
    -Wformat=2
    -Wlogical-op
    -Wmissing-include-dirs
    -Wredundant-decls
    -Wshadow
    -Wswitch-default
    -Wundef
    -Wunused
    -Wswitch-enum
    -Wnull-dereference
    -Wno-error=null-dereference
    -Wshift-overflow=2
    -Walloc-zero
    -Walloca
    -Wduplicated-branches
    -Wduplicated-cond
    -Wdate-time
    -Wno-ignored-qualifiers
)

list(APPEND extra_c_flags
    -Wjump-misses-init
    -Wnested-externs
    # -Wc++-compat
    # -Wno-error=c++-compat
)

list(APPEND extra_cxx_flags
    -Wctor-dtor-privacy
    -Wnoexcept
    -Woverloaded-virtual
    -Wsign-promo
    -Wuseless-cast
    -Wextra-semi
    -Wmultiple-inheritance
)

add_compile_options(
    -Wall -Wextra -Werror
    ${extra_flags}
    "$<$<COMPILE_LANGUAGE:C>:${extra_c_flags}>"
    "$<$<COMPILE_LANGUAGE:CXX>:${extra_cxx_flags}>"
)

# Fixes the use of the __FILE__ macro to not use the absolute workspace path
add_compile_options(-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.)

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "-O2 -ggdb")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/../dist)

# List all used subdirs
list(APPEND components 
    mongoose
    httpd
)

foreach(comp ${components})
    # Make the project aware of all the named subdirs
    # This must be done one by one (hence the loop)
    add_subdirectory(${comp})
endforeach()
