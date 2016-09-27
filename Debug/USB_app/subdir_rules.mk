################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
USB_app/usbConstructs.obj: ../USB_app/usbConstructs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/bin/cl430" -vmspx --abi=eabi -O3 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/include" --include_path="C:/Projects/CodeComposerFirmwares/SpikeRecorderHID/driverlib/MSP430F5xx_6xx" --include_path="C:/Projects/CodeComposerFirmwares/SpikeRecorderHID/driverlib/MSP430F5xx_6xx/deprecated" --include_path="C:/Projects/CodeComposerFirmwares/SpikeRecorderHID" --include_path="C:/Projects/CodeComposerFirmwares/SpikeRecorderHID/USB_config" -g --define=__MSP430F5522__ --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_app/usbConstructs.pp" --obj_directory="USB_app" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

USB_app/usbEventHandling.obj: ../USB_app/usbEventHandling.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/bin/cl430" -vmspx --abi=eabi -O3 --use_hw_mpy=F5 --include_path="C:/ti/ccsv6/ccs_base/msp430/include" --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/include" --include_path="C:/Projects/CodeComposerFirmwares/SpikeRecorderHID/driverlib/MSP430F5xx_6xx" --include_path="C:/Projects/CodeComposerFirmwares/SpikeRecorderHID/driverlib/MSP430F5xx_6xx/deprecated" --include_path="C:/Projects/CodeComposerFirmwares/SpikeRecorderHID" --include_path="C:/Projects/CodeComposerFirmwares/SpikeRecorderHID/USB_config" -g --define=__MSP430F5522__ --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="USB_app/usbEventHandling.pp" --obj_directory="USB_app" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


