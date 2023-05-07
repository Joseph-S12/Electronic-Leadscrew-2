add_executable(main main.c display.c gpio.c motion.c)
target_link_libraries(main pico_stdlib)
target_link_libraries(main pico_multicore)
target_link_libraries(main pico_time)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)
