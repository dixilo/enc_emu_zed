#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xuartlite.h"
#include "sleep.h"

#define POLARITY_MASK 0x00000002
#define Z_PULSE_MASK  0x00000004
#define SPEED_MASK    0xFFFFFFF8
#define UART_DEV_ID   XPAR_AXI_UARTLITE_DEVICE_ID

#define UART_BIN_LEN  6

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

    if (ret != 1){
        print("INVALID INPUT.\n\r");
        return -1;
    }

    set_speed(speed);
    return 0;
}

int send_z_pulse(){
    int cur_status;
    cur_status = Xil_In32(XPAR_AXI_GPIO_BASEADDR);
    Xil_Out32(XPAR_AXI_GPIO_BASEADDR, cur_status | Z_PULSE_MASK);
    Xil_Out32(XPAR_AXI_GPIO_BASEADDR, cur_status & (~Z_PULSE_MASK));
    return 0;
}

/* scanf("%s", ...) のあとに残る改行を捨てる */
int discard_until_newline(void){
    int c;

    while (1){
        c = getchar();

        if (c == EOF){
            return -1;
        }

        if (c == '\n'){
            break;
        }
    }

    return 0;
}

/* stdin からちょうど len バイト読む */
int read_exact_stdin(u8 *buf, int len){
    int total = 0;

    while (total < len){
        int n = fread(buf + total, 1, len - total, stdin);
        if (n <= 0){
            return -1;
        }
        total += n;
    }

    return 0;
}

/*
 * PC -> Zybo:
 *   "5\n"
 *   55 00 00 00 00  00 (生バイナリ6バイト)
 *
 * Zybo -> FWD UART:
 *   上記5バイトをそのまま再送信
 */
int send_uart_dialog(XUartLite *inst){
    u8 uart_bin[UART_BIN_LEN];
    int i;

    print("PLEASE PUT 6 BYTES.\n\r");

    /* メニュー入力の "5\n" で残った改行を捨てる */
    if (discard_until_newline() != 0){
        print("FAILED TO DISCARD NEWLINE.\n\r");
        return -1;
    }

    /* 生バイナリ6バイトをそのまま読む */
    if (read_exact_stdin(uart_bin, UART_BIN_LEN) != 0){
        print("FAILED TO READ UART BINARY.\n\r");
        return -1;
    }

    xil_printf("RX BINARY:");
    for (i = 0; i < UART_BIN_LEN; i++){
        xil_printf(" %02X", uart_bin[i]);
    }
    xil_printf("\n\r");

    /* FWD側へ6バイトそのまま転送 */
    XUartLite_Send(inst, uart_bin, UART_BIN_LEN);
    while (XUartLite_IsSending(inst));

    print("UART FORWARDED.\n\r");
    return 0;
}

int main()
{
    long menu_num;
    char input_buf[256];
    char *end_ptr;
    int ret;
    XUartLite uart_lite;

    init_platform();
    print("###### ENCODER EMULATOR v0.1 ######\n\r");

    print("## GPIO RESET ##\n\r");
    Xil_Out32(XPAR_AXI_GPIO_BASEADDR, 0x00000000);
    print("## GPIO RESET SUCCESS ##\n\r");

    print("## UART INIT ##\n\r");

    ret = XUartLite_Initialize(&uart_lite, UART_DEV_ID);
    if (ret != XST_SUCCESS){
        print("## UART INIT FAIL.##\n\r");
        return XST_FAILURE;
    }

    ret = XUartLite_SelfTest(&uart_lite);
    if (ret != XST_SUCCESS){
        print("## UART SELF TEST FAIL.##\n\r");
        return XST_FAILURE;
    }

    print("## UARTRESET SUCCESS ##\n\r");

    while(1){
        print("###### MENU ######\n\r");
        print("1. POSITIVE INCREMENT\n\r");
        print("2. NEGATIVE INCREMENT\n\r");
        print("3. SET SPEED\n\r");
        print("4. SEND Z-PULSE\n\r");
        print("5. SEND UART\n\r");
        print("6. STOP\n\r");
        print("PLEASE SELECT MENU NUM: ");

        ret = scanf("%255s", input_buf);

        if (ret != 1){
            print("INVALID INPUT.\n\r");
            continue;
        }

        menu_num = strtol(input_buf, &end_ptr, 10);

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
            send_uart_dialog(&uart_lite);
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
