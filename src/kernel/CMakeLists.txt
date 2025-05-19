include_directories(libs)

add_subdirectory(arch)
add_subdirectory(appendix)
add_subdirectory(memory)
add_subdirectory(drivers)

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
                                          memory.o
                                          drivers.o)

add_custom_command(TARGET coronel PRE_LINK
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                   COMMAND ${CMAKE_COMMAND} -E echo "Copying LD script"
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/kernel/arch/amd64/bootstrap/coronel.ld ./src/kernel)

add_custom_command(TARGET coronel POST_BUILD
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                   COMMAND ${CMAKE_COMMAND} -E echo_append "Generating ISO..."
                   COMMAND ${CMAKE_COMMAND} -E make_directory iso/boot/grub
                   COMMAND ${CMAKE_COMMAND} -E copy src/kernel/coronel iso/boot
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/contrib/grub.cfg iso/boot/grub
                   COMMAND ${GRUBRESCUE} -o coronel.iso iso > /dev/null 2>&1
                   COMMAND ${CMAKE_COMMAND} -E remove_directory iso
                   COMMAND ${OBJDUMP} -S src/kernel/coronel > coronel.asm
                   COMMAND ${OBJDUMP} -t src/kernel/coronel > coronel.sym
                   COMMAND ${CMAKE_COMMAND} -E echo "Finished")

add_custom_target(qemu
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                  COMMAND /bin/sh -c '[ ! -f hda.img ] && /usr/bin/qemu-img create -f raw hda.img 128M && /usr/sbin/mkfs.ext2 hda.img || exit 0'
                  COMMAND qemu-system-x86_64
                      -m 1G
                      -machine q35,accel=tcg
                      -cdrom coronel.iso
                      -d cpu_reset
                      -no-shutdown
                      -device ide-hd,bus=ide.0,drive=drive-sata0-0-0,id=sata0-0-0,bootindex=1
                      -drive file=hda.img,format=raw,if=none,id=drive-sata0-0-0
                  DEPENDS coronel)