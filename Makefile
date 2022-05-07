# Run with:
# "C:\\ti\\ccs1010\\ccs\\utils\\bin\\gmake" -k -j 8 all -O 

SHELL = cmd.exe
CG_TOOL_ROOT := C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-msp430_20.2.1.LTS

CFLAGS = -vmsp -O4 --opt_for_speed=0 --use_hw_mpy=none --advice:power="all" --define=__MSP430G2553__ -g --c11 --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number -z -m"AddressScan.map" --heap_size=412 --stack_size=100 -i"C:/ti/ccs1010/ccs/ccs_base/msp430/include" -i"C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-msp430_20.2.1.LTS/lib" -i"C:/ti/ccs1010/ccs/tools/compiler/ti-cgt-msp430_20.2.1.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="AddressScan_linkInfo.xml" --use_hw_mpy=none --rom_model -o "AddressScan.out" $(ORDERED_OBJS)
main.o:
	@echo "Compiling main.c"
	"$(CG_TOOL_ROOT)/bin/cl430" 