add_custom_target(copy_static_files ALL
    COMMAND rsync -a
    --exclude=*.txt
    .
    ${CMAKE_CURRENT_BINARY_DIR}/httpd
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/httpd TYPE BIN MESSAGE_NEVER)
