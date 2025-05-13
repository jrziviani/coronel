include_directories(libs)

add_subdirectory(arch)
add_subdirectory(appendix)
add_subdirectory(memory)

set(MAX_PAGE_SIZE 0x1000)
set(LINKER_SCRIPT "coronel.ld")
set(LFLAGS "-fno-PIC -fno-pie -fno-exceptions -fno-rtti -mno-red-zone -mcmodel=kernel -nostdlib -lgcc -Wl,-z,max-page-size=${MAX_PAGE_SIZE}")

add_executable(coronel
               libs/new.cpp
               main.cpp)

set_target_properties(coronel PROPERTIES
                      LINK_FLAGS "-T ${LINKER_SCRIPT} ${LFLAGS}")

target_link_libraries(coronel LINK_PUBLIC amd64.o
                                          appendix.o
                                          memory.o)

add_custom_command(TARGET coronel PRE_LINK
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                   COMMAND ${CMAKE_COMMAND} -E echo "Copying LD script"
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/kernel/arch/amd64/bootstrap/coronel.ld ./src/kernel)