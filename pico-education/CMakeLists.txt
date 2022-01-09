#Set the mimum required version of CMake
cmake_minimum_required(VERSION 3.12)

#Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

#Set the project name, the programming language
project(project1 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#Initialize the SDK
pico_sdk_init()

#Add an executable target to be build from the source files
add_executable(pico_education 
                main.c 
                lcd_1602a.c 
                calendar.c 
                thermometer.c)

# pull in common dependencies and additional i2c hardware support
target_link_libraries(pico_education 
                    pico_stdlib 
                    hardware_i2c
                    hardware_adc)

#Enable USB output, disable UART output
pico_enable_stdio_usb(pico_education 1)
pico_enable_stdio_uart(pico_education 0)                    

#Create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(pico_education)
