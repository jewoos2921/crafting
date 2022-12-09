cmake_minimum_required(VERSION 3.23)
project(crafting C)

set(CMAKE_C_STANDARD 99)

#add_executable(crafting
#        main.c
#        chunk.c
#        memory.c
#        debug.c
#        value.c
#        vm.c
#        compiler.c
#        scanner.c
#        object.c)

add_executable(
        acwj

        acwj/acwj1_scanner/main.c
        acwj/acwj1_scanner/scan.c
        )

add_executable(
        cray
        "cray/chunks of bytecode/memory.c"
        "cray/chunks of bytecode/chunk.c"
        "cray/chunks of bytecode/main.c"
        "cray/chunks of bytecode/debug.c" )
