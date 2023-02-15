cmake_minimum_required(VERSION 3.23)
project(crafting C)

set(CMAKE_C_STANDARD 23)
#set(CMAKE_C_STANDARD 99)

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

        acwj/acwj1_scanner_2parser/main.c
        acwj/acwj1_scanner_2parser/scan.c
        acwj/acwj1_scanner_2parser/expression.c
        acwj/acwj1_scanner_2parser/tree.c
)

add_executable(
        cray

        "cray/chunks of bytecode/memory.c"
        "cray/chunks of bytecode/chunk.c"
        "cray/chunks of bytecode/main.c"
        "cray/chunks of bytecode/debug.c"
        "cray/chunks of bytecode/value.c"
        "cray/chunks of bytecode/vm.c"
)

add_executable(
        small
        small/codegen.c
        small/table.c
        small/getSource.c
        small/main.c
        small/compile.c)

add_executable(
        64OS
        #        64OS/Kernel32/Source/Main.c
        #        64OS/ImageMaker.

        #        64OS/Kernel32/Source/Page.c
        64OS/Kernel64/Source/Keyboard.c
        64OS/Kernel64/Source/Main.c
        64OS/Kernel64/Source/Page.c
        64OS/Kernel64/Source/Descriptor.c
        64OS/Kernel64/Source/Utility.c
        64OS/Kernel64/Source/PIC.c
        64OS/Kernel64/Source/InterruptHandler.c
        64OS/Kernel64/Source/Queue.c
        64OS/Kernel64/Source/Console.c
        64OS/Kernel64/Source/ConsoleShell.c
        64OS/Utility/Files/BTree.c
        64OS/Kernel64/Source/PIT.c
        64OS/Kernel64/Source/RTC.c
        64OS/Kernel64/Source/Task.c
        64OS/Kernel64/Source/List.c
        64OS/Kernel64/Source/Synchronization.c
        64OS/Kernel64/Source/DynamicMemory.c
        64OS/Utility/Files/Hashing.c
        64OS/Kernel64/Source/HardDisk.c
        64OS/Kernel64/Source/FileSystem.c
        64OS/Utility/Files/HashMain.c
        64OS/Utility/Files/HashPage.c
        64OS/Utility/Files/Hash.c
        64OS/Kernel64/Source/CacheManager.c
        64OS/Kernel64/Source/RAMDisk.c
        64OS/Kernel64/Source/SerialPort.c

        64OS/Kernel64/Source/MPConfigurationTable.c 64OS/Kernel64/Source/LocalAPIC.c 64OS/Kernel64/Source/MultiProcessor.c 64OS/Kernel64/Source/IOAPIC.c 64OS/Kernel64/Source/VBE.c)

