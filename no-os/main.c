#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xuartlite.h"
#include "sleep.h"

#define POLARITY_MASK 0x00000002
#define Z_PULSE_MASK 0x00000004
#define SPEED_MASK 0xFFFFFFF8
#define UART_DEV_ID XPAR_AXI_UARTLITE_DEVICE_ID


int set_pol(int pol){
	int cur_status;
	cur_status = Xil_In32(XPAR_AXI_GPIO_BASEADDR);
	Xil_Out32(XPAR_AXI_GPIO_BASEADDR, (cur_status & (~POLARITY_MASK)) | pol);
	return 0;
}

int set_neg_inc(){
	set_pol(3);
	return 0;
}

int set_pos_inc(){
	set_pol(1);
	return 0;
}

int stop(){
	set_pol(0);
	return 0;
}

int set_speed(int speed){
	int cur_status;
	cur_status = Xil_In32(XPAR_AXI_GPIO_BASEADDR);
	Xil_Out32(XPAR_AXI_GPIO_BASEADDR, (cur_status & (~SPEED_MASK)) | (speed << 3));

	return 0;
}

int set_speed_dialog(){
	int speed;
	int ret;

	print("PLEASE PUT SPEED: ");
	ret = scanf("%d", &speed);

	if (ret == EOF){
		print("INVALID INPUT.\n\r");
		return -1;
	}

	set_speed(speed);
	return 0;
}

int send_z_pulse(){
	int cur_status;
	cur_status = Xil_In32(XPAR_AXI_GPIO_BASEADDR);
	Xil_Out32(XPAR_AXI_GPIO_BASEADDR, cur_status  | Z_PULSE_MASK);
	Xil_Out32(XPAR_AXI_GPIO_BASEADDR, cur_status & (~Z_PULSE_MASK));

	return 0;
}


int send_uart_dialog(XUartLite *inst){
	u8 uart_str[16];
	int ret;

	print("PLEASE PUT UART STRING: ");
	ret = scanf("%s", uart_str);
	if (ret == EOF){
		print("INVALID INPUT.\n\r");
		return -1;
	}

	XUartLite_Send(inst, uart_str, sizeof(uart_str));

	return 0;
}

int main()
{
	int menu_num;
	int ret;
	XUartLite* uart_lite;

	// Platform initialization
    init_platform();
    print("###### ENCODER EMULATOR v0.1 ######\n\r");

    // GPIO initialization
    print("## GPIO RESET ##\n\r");
    Xil_Out32(XPAR_AXI_GPIO_BASEADDR, 0x00000000);
    print("## GPIO RESET SUCCESS ##\n\r");

    // UART initialization
    print("## UART INIT ##\n\r");

    ret = XUartLite_Initialize(uart_lite, UART_DEV_ID);
	if (ret != XST_SUCCESS){
		print("## UART INIT FAIL.##\n\r");
		return XST_FAILURE;
	}

	ret = XUartLite_SelfTest(uart_lite);
	if (ret != XST_SUCCESS){
		print("## UART SELF TEST FAIL.##\n\r");
		return XST_FAILURE;
	}

	print("## UARTRESET SUCCESS ##\n\r");

    // Main menu
    while(1){
    	print("###### MENU ######\n\r");
    	print("1. POSITIVE INCREMENT\n\r");
    	print("2. NEGATIVE INCREMENT\n\r");
    	print("3. SET SPEED\n\r");
    	print("4. SEND Z-PULSE\n\r");
    	print("5. SEND UART\n\r");
    	print("6. STOP\n\r");
    	print("PLEASE SELECT MENU NUM: ");

    	ret = scanf("%d", &menu_num);

    	if ( ret == EOF ){
    		print("INVALID INPUT.\n\r");
    		continue;
    	}

    	switch(menu_num){
    	case 1:
    		set_pos_inc();
    		break;
    	case 2:
    		set_neg_inc();
    		break;
    	case 3:
    		set_speed_dialog();
    		break;
    	case 4:
    		send_z_pulse();
    		break;
    	case 5:
    		send_uart_dialog(uart_lite);
    		break;
    	case 6:
    		stop();
    		break;
    	default:
    		print("INVALID INPUT.\n\r");
    		break;
    	}
    	print("DONE.\n\r");
    }


    cleanup_platform();
    return 0;
}
