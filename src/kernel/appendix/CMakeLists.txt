add_library(appendix.o OBJECT crti.S
                              crtn.S
                              cxa_purevirtual.cpp
                              cxa_guard.cpp
                              icxxabi.cpp)