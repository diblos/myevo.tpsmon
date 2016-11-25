################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../includes/ \
../libs/


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../main.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
main.o


OBJS_AS_ARGS +=  \
main.o


C_DEPS +=  \
main.d


C_DEPS_AS_ARGS +=  \
main.d


OUTPUT_FILE_PATH +=ContactlessCard.elf

OUTPUT_FILE_PATH_AS_ARGS +=ContactlessCard.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

# AVR32/GNU C Compiler



./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR32/GNU C Compiler
	$(QUOTE)C:\Program Files (x86)\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\bin\avr32-gcc.exe$(QUOTE)  -I"../includes" -I"../activities"  -O1 -ffunction-sections -masm-addr-pseudos -g3 -Wall -c -Werror-implicit-function-declaration -Wmissing-prototypes -Wpointer-arith -Wstrict-prototypes -mno-cond-exec-before-reload -mrelax -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"  -mpart=uc3a3256  -o"$@" "$<" 
	@echo Finished building: $<



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP)
	@echo Building target: $@
	@echo Invoking: AVR32/GNU C Linker
	$(QUOTE)C:\Program Files (x86)\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\bin\avr32-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -nostartfiles -Wl,-Map="ContactlessCard.map" -lh2framework -lh2core -l_iniparser -l_util -l_font  -L"../libs"  -Wl,--gc-sections --rodata-writable -Wl,--direct-data -T../link_uc3a3256sdram.lds -Wl,--relax -Wl,-e,_trampoline  -mpart=uc3a3256 
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\bin\avr32-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "ContactlessCard.elf" "ContactlessCard.hex"
	"C:\Program Files (x86)\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\bin\avr32-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "ContactlessCard.elf" "ContactlessCard.eep" || exit 0
	"C:\Program Files (x86)\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\bin\avr32-objdump.exe" -h -S "ContactlessCard.elf" > "ContactlessCard.lss"
	"C:\Program Files (x86)\Atmel\AVR Studio 5.1\extensions\Atmel\AVRGCC\3.3.1.27\AVRToolchain\bin\avr32-size.exe" "ContactlessCard.elf"
	




# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS)$(C_DEPS_AS_ARGS) $(EXECUTABLES) 
	rm -rf "ContactlessCard.hex" "ContactlessCard.lss" "ContactlessCard.eep" "ContactlessCard.map"
	