################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
USB_API/USB_CDC_API/UsbCdc.obj: ../USB_API/USB_CDC_API/UsbCdc.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/Texas-Instruments/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmspx --abi=eabi -O3 --include_path="/Applications/Texas-Instruments/ccsv6/ccs_base/msp430/include" --include_path="/Applications/Texas-Instruments/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID/driverlib/MSP430F5xx_6xx" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID/driverlib/MSP430F5xx_6xx/deprecated" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID/USB_config" --advice:power="none" --define=__MSP430F5529__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_API/USB_CDC_API/UsbCdc.pp" --obj_directory="USB_API/USB_CDC_API" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


