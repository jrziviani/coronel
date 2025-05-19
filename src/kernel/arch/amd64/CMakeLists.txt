add_subdirectory(bootstrap)
add_subdirectory(video)

add_library(amd64.o OBJECT amd64.cpp)

target_link_libraries(amd64.o PUBLIC amd64_bootstrap.o
                                     amd64_protected_mode.o)