######## HAL ##########
file(GLOB src STM32F4xx_HAL_Driver/Src/*.c)
add_library(STM32f4xx_HAL_Driver ${src})

# There is a lot of unused variable warnings in some hal function-prototypes
target_compile_options(STM32f4xx_HAL_Driver PRIVATE -Wno-extra -Wno-pedantic)


######## CMSIS ########
add_library(CMSIS
	CMSIS/stm32f401xe/startup_stm32f401xe.s
	CMSIS/stm32f415xx/startup_stm32f415xx.s
	CMSIS/stm32f407xx/startup_stm32f407xx.s
	CMSIS/system_stm32f4xx.c
)

# There is a lot of unused variable warnings in some hal function-prototypes
target_compile_options(CMSIS PRIVATE -Wno-extra -Wno-pedantic)

######## USB DEVICE LIBRARY ########
add_subdirectory(STM32_USB_Device_Library)
# add_library(STM32_USB_Device_Library
# 	STM32_USB_Device_Library/Core/Src/usbd_core.c
# 	STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
# 	STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
# )
#
# target_compile_options(STM32_USB_Device_Library PRIVATE -Wno-unused-parameter)
