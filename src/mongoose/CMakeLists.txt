list(APPEND mongoose_sources
    ./mongoose/mongoose.c
    ./mongoose_main.cpp
)
add_executable(mongoose ${mongoose_sources})

target_compile_options(mongoose PRIVATE -w)
target_compile_definitions(mongoose PRIVATE -DMG_ENABLE_LOG=1 -DMG_ENABLE_LINES=1)

install(TARGETS mongoose)