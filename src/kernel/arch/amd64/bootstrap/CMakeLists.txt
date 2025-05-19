add_library(amd64_bootstrap.o STATIC boot.S
                                     gdt.S
                                     isr.S
                                     irq.cpp
                                     segments.cpp)