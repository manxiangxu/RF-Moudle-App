################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv5/tools/compiler/msp430_4.1.5

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./5xx_HAL/hal_pmm.obj" \
"./5xx_HAL/hal_flash.obj" \
"./5xx_HAL/hal_UCS.obj" \
"./5xx_HAL/hal_PMAP.obj" \
"./5xx_HAL/RF1A.obj" \
"./AT25DF521/at25df521.obj" \
"./uart.obj" \
"./timer.obj" \
"./setup.obj" \
"./main.obj" \
-l"libc.a" \
"../lnk_cc430f5137.cmd" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include AT25DF521/subdir_vars.mk
-include 5xx_HAL/subdir_vars.mk
-include subdir_rules.mk
-include AT25DF521/subdir_rules.mk
-include 5xx_HAL/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: CC430APP.out

# Tool invocations
CC430APP.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: MSP430 Linker'
	"C:/ti/ccsv5/tools/compiler/msp430_4.1.5/bin/cl430" -vmspx --abi=eabi -Ooff -g --define=__CC430F5137__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU18 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal -z --stack_size=80 -m"CC430APP.map" --heap_size=80 --use_hw_mpy=F5 -i"C:/ti/ccsv5/ccs_base/msp430/include" -i"C:/ti/ccsv5/tools/compiler/msp430_4.1.5/lib" -i"C:/ti/ccsv5/tools/compiler/msp430_4.1.5/include" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="CC430APP_linkInfo.xml" --rom_model -o "CC430APP.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(MSP430_EXECUTABLE_OUTPUTS__QUOTED) "CC430APP.out"
	-$(RM) "main.pp" "setup.pp" "timer.pp" "uart.pp" "AT25DF521\at25df521.pp" "5xx_HAL\RF1A.pp" "5xx_HAL\hal_PMAP.pp" "5xx_HAL\hal_UCS.pp" "5xx_HAL\hal_flash.pp" "5xx_HAL\hal_pmm.pp" 
	-$(RM) "main.obj" "setup.obj" "timer.obj" "uart.obj" "AT25DF521\at25df521.obj" "5xx_HAL\RF1A.obj" "5xx_HAL\hal_PMAP.obj" "5xx_HAL\hal_UCS.obj" "5xx_HAL\hal_flash.obj" "5xx_HAL\hal_pmm.obj" 
	-@echo 'Finished clean'
	-@echo ' '

post-build:
	-@echo 'Create flash image: TI-TXT'
	-"C:/ti/ccsv5/tools/compiler/msp430_4.1.5/bin/hex430" --ti_txt "CC430APP.out" -o "CC430APP.txt" -order MS -romwidth 16
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
