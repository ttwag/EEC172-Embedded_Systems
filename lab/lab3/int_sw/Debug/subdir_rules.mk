################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
gpio_if.obj: /Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/example/common/gpio_if.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=none -me -Ooff --include_path="/Users/taowang/Desktop/EEC172-Embedded_Systems/lab/lab3/int_sw" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/driverlib/" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/inc/" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/example/common/" --include_path="/Applications/ti/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs --define=cc3200 --define=NON_NETWORK -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=none -me -Ooff --include_path="/Users/taowang/Desktop/EEC172-Embedded_Systems/lab/lab3/int_sw" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/driverlib/" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/inc/" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/example/common/" --include_path="/Applications/ti/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs --define=cc3200 --define=NON_NETWORK -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_ccs.obj: /Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/example/common/startup_ccs.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"/Applications/ti/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=none -me -Ooff --include_path="/Users/taowang/Desktop/EEC172-Embedded_Systems/lab/lab3/int_sw" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/driverlib/" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/inc/" --include_path="/Applications/ti/lib/cc3200sdk_1.5.0/cc3200-sdk/example/common/" --include_path="/Applications/ti/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs --define=cc3200 --define=NON_NETWORK -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


