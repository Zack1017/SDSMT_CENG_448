cmake_minimum_required(VERSION 3.17)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

# Cross compiler tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_LD arm-none-eabi-ld)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Project settings
get_filename_component(DirName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${DirName} C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(MCPU cortex-m3)

# Output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/gcc_arm.ld)

# Compiler options
add_compile_options(
    -mcpu=${MCPU}
    -mthumb
    -mthumb-interwork
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fmessage-length=0
    -g
)

# Linker options
add_link_options(
    -mcpu=${MCPU}
    -mthumb
    -mthumb-interwork
    -T ${LINKER_SCRIPT}
    --specs=nosys.specs
    -nostartfiles
    -Wl,-gc-sections,--print-memory-usage
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include

    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/include
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM3

    ${CMAKE_SOURCE_DIR}/CMSIS/CMSIS/Core/Include
    ${CMAKE_SOURCE_DIR}/CMSIS/Device/ARM/ARMCM3/Include

    ${CMAKE_SOURCE_DIR}/PDCurses
    ${CMAKE_SOURCE_DIR}/PDCurses/common
    ${CMAKE_SOURCE_DIR}/PDCurses/ANSI/drivers
    ${CMAKE_SOURCE_DIR}/PDCurses/ANSI/drivers/FreeRTOS

    ${CMAKE_SOURCE_DIR}/ninvaders
)

# nInvaders source files
file(GLOB NINVADERS_SOURCES
    "${CMAKE_SOURCE_DIR}/ninvaders/*.c"
)
file(GLOB PDCURSES_ANSI_SOURCES
    "${CMAKE_SOURCE_DIR}/PDCurses/ANSI/drivers/ANSI_terminal.c"
    "${CMAKE_SOURCE_DIR}/PDCurses/ANSI/drivers/FreeRTOS/driver_table.c"
)
# Main project sources
set(SOURCES
    # FreeRTOS kernel
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/tasks.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/list.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/queue.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/stream_buffer.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/timers.c
    #${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/portable/MemMang/heap_3.c

    # CMSIS
    #{CMAKE_SOURCE_DIR}/CMSIS/CMSIS/Core/Source/core_cm3.c
    #${CMAKE_SOURCE_DIR}/CMSIS/CMSIS/Device/ARM/ARMCM3/Source/system_ARMCM3.c

    # Startup and linker-related
    ${CMAKE_SOURCE_DIR}/src/startup_ARMCM3.S

    # App source
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/UART_16550.c
    ${CMAKE_SOURCE_DIR}/src/hello_task.c
    ${CMAKE_SOURCE_DIR}/src/stats_task.c
    ${CMAKE_SOURCE_DIR}/src/AXI_timer.c
    ${CMAKE_SOURCE_DIR}/src/PM_test_task.c
    ${CMAKE_SOURCE_DIR}/src/firework_task.c
    ${CMAKE_SOURCE_DIR}/src/testcurs_task.c
    ${CMAKE_SOURCE_DIR}/src/LDP-001_PM_driver.c
    ${CMAKE_SOURCE_DIR}/src/newlib_functions.c
    ${CMAKE_SOURCE_DIR}/src/heap_useNewlib.c
    ${CMAKE_SOURCE_DIR}/src/sincos.c

        # nInvaders
    ${NINVADERS_SOURCES}

    # PDCurses ANSI driver
    ${PDCURSES_ANSI_SOURCES}
)

# Define our linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/gcc_arm.ld)

# Additional options for the GNU linker
add_link_options(-Wl,-gc-sections,--print-memory-usage)
#  add_link_options(-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=${MCPU} -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT} --specs=nosys.specs -nostartfiles)



# Define our target executable, and everything that it depends on
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

target_link_libraries(${PROJECT_NAME}.elf m)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/include/ 
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Kernel/portable/GCC/ARM_CM3/
    ${CMAKE_SOURCE_DIR}/CMSIS/CMSIS/Core/Include
    ${CMAKE_SOURCE_DIR}/CMSIS/Device/ARM/ARMCM3/Include
    
)#/opt/packages/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Plus-TCP/test/FreeRTOS-Kernel/include/projdefs.h
#/opt/packages/FreeRTOS-LTS/FreeRTOS/FreeRTOS-Kernel/include/projdefs.h

set(ELF_FILE ${PROJECT_NAME}.elf)
set(HEX_FILE ${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex bin/${ELF_FILE} bin/${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary bin/${ELF_FILE} bin/${BIN_FILE}
        COMMENT "Building ${HEX_FILE} Building ${BIN_FILE}")
      
set_property(
  TARGET ${PROJECT_NAME}.elf
  APPEND
  PROPERTY ADDITIONAL_CLEAN_FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map ${HEX_FILE} ${BIN_FILE}
  )


set(CMAKE_SKIP_INSTALL_RULES True)