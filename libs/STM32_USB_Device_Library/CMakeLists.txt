file(GLOB core_src ./Core/Src/*.c)
file(GLOB class_cdc_src ./Class/CDC/Src/*.c)
add_library(STM32_USB_Device_Library ${core_src} ${class_cdc_src})
target_compile_options(STM32_USB_Device_Library PRIVATE -Wno-extra -Wno-pedantic -Wno-unused-parameter) # There is a lot of unused variable warnings in some hal function-prototypes
