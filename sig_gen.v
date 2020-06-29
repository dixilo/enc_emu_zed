`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: kuhep
// Engineer: jsuzuki
// 
// Create Date: 2020/06/26
// Design Name: sig_gen
// Module Name: sig_gen
// Project Name: el_encoder
// Target Devices: Zedbaord
// Tool Versions: Vivado2018.3
// Description: Generation of incremental signal of encoder
// 
// Dependencies: Standalone
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module sig_gen #(
    parameter Z_WIDTH   = 50 // 500 ns for 100MHz clock
    )(
    input        clk,
    input        rstn,
    input [31:0] conf,

    output       rot_a,
    output       rot_b,
    output       rot_z);

    /////////////////////////////////////// configuration
    //// rot_setting: conf[1:0]
    // 2'b00: stop
    // 2'b01: increment
    // 2'b11: decrement
    // 2'b10: NA
    wire [1:0] rot_setting;
    assign rot_setting = conf[1:0];

    //// z_pub: conf[2]
    wire z_pub;
    assign z_pub = conf[2];

    //// speed: conf [31:3]
    wire [28:0] speed;
    reg [28:0] speed_buf;
    assign speed = conf[28:0];

    /////////////////////////////////////// speed
    wire speed_changed;
    always @(posedge clk) begin
        speed_buf <= speed;
    end
    assign speed_changed = (speed_buf != speed);

    /////////////////////////////////////// rot_a and rot_b
    reg [28:0] rot_counter;
    reg [1:0] phase;
    wire rot_counter_max;
    assign rot_counter_max = (rot_counter == speed_buf);

    always @(posedge clk) begin
        if ( ~rstn | speed_changed ) begin
            rot_counter <= 28'b0;
        end else begin
            if (rot_counter_max) begin
                phase <= phase + rot_setting;
                rot_counter <= 28'b0;
            end else begin
                rot_counter <= rot_counter + 1;
            end
        end
    end

    assign rot_a = (phase == 2'b00) | (phase == 2'b01);
    assign rot_b = (phase == 2'b01) | (phase == 2'b10);

    /////////////////////////////////////// rot_z
    reg z_pub_buf;
    reg [$clog2(Z_WIDTH)-1:0] z_counter;
    wire z_pub_edge;
    wire z_counter_max;

    always @(posedge clk) begin
        z_pub_buf <= z_pub;
    end

    assign z_pub_edge = (z_pub) & (~z_pub_buf);
    assign z_counter_max = (z_counter == Z_WIDTH);

    always @(posedge clk) begin
        if (z_pub_edge | (z_counter != 0)) begin
            if (z_counter_max)
                z_counter <= 0;
            else
                z_counter <= z_counter + 1;
        end
    end
    
    assign rot_z = (z_counter != 0);

endmodule
