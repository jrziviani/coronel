add_library(amd64_bootstrap.o STATIC boot.S
                                     gdt.S
                                     isr.S
                                     segments.cpp)