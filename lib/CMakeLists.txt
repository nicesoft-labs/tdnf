add_library(${LIB_TDNF_PROGRESS} STATIC
    progress.c
)

target_include_directories(${LIB_TDNF_PROGRESS} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
