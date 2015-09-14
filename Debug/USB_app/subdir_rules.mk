################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
USB_app/usbConstructs.obj: ../USB_app/usbConstructs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/Texas-Instruments/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmspx --abi=eabi -O3 --opt_for_speed=2 --use_hw_mpy=F5 --include_path="/Applications/Texas-Instruments/ccsv6/ccs_base/msp430/include" --include_path="/Applications/Texas-Instruments/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID/driverlib/MSP430F5xx_6xx" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID/driverlib/MSP430F5xx_6xx/deprecated" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID/USB_config" -g --define=__MSP430F5522__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_app/usbConstructs.pp" --obj_directory="USB_app" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

USB_app/usbEventHandling.obj: ../USB_app/usbEventHandling.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/Texas-Instruments/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/bin/cl430" -vmspx --abi=eabi -O3 --opt_for_speed=2 --use_hw_mpy=F5 --include_path="/Applications/Texas-Instruments/ccsv6/ccs_base/msp430/include" --include_path="/Applications/Texas-Instruments/ccsv6/tools/compiler/ti-cgt-msp430_4.4.3/include" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID/driverlib/MSP430F5xx_6xx" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID/driverlib/MSP430F5xx_6xx/deprecated" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID" --include_path="/Users/nik/Desktop/Projects/TIWorkspace/SpikeRecorderHID/USB_config" -g --define=__MSP430F5522__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_app/usbEventHandling.pp" --obj_directory="USB_app" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


