cmake_minimum_required(VERSION 3.10)
project (coronel VERSION 0.0.1
         DESCRIPTION "Coronel Homemade Operating System"
         LANGUAGES CXX ASM)

find_program(XORRISO xorriso)
if (NOT XORRISO)
    message(FATAL_ERROR "xorriso program not found")
endif ()

find_program(GRUBRESCUE grub2-mkrescue)
if (NOT GRUBRESCUE)
    find_program(GRUBRESCUE grub-mkrescue)
    if (NOT GRUBRESCUE)
        message(FATAL_ERROR "grub mkrescue not found")
    endif ()
endif ()

set(CUSTOM_C x86_64-elf-gcc)
set(CUSTOM_CXX x86_64-elf-g++)
set(OBJDUMP x86_64-elf-objdump)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_COMPILER ${CUSTOM_C})
set(CMAKE_CXX_COMPILER ${CUSTOM_CXX})
set(CMAKE_ASM_COMPILER ${CUSTOM_ASM})

add_subdirectory(src)