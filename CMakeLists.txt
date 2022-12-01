cmake_minimum_required(VERSION 3.23)
project(crafting C)

set(CMAKE_C_STANDARD 99)

add_executable(crafting
        main.c
        chunk.c
        memory.c
        debug.c
        value.c
        vm.c
        compiler.c
        scanner.c


        object.c
        table.c )
