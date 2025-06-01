// filepath: /home/gkrol/projects/ai/hwaccel/rtl/matmul_tb.v
`include "sram.v"
`include "matmul.v"

module matmul_tb #(
    parameter MAX_DIM = 16,
    parameter SRAM_ADDR_WIDTH = 10,
    parameter DATA_WIDTH = 8,
    parameter SRAM_DEPTH = 1024
)(
    input         clk,
    input         rst,
    input  [7:0]  in_data,
    input         in_valid,
    output        in_ready,

    output [31:0] out_data,
    output        out_valid,
    input         out_ready,
    
    // SRAM interface for external control
    input                       vec_sram_we,
    input  [SRAM_ADDR_WIDTH-1:0] vec_sram_addr,
    input  [7:0]                vec_sram_din
);

    // Internal signals for connection between matmul and SRAM
    wire                        mm_vec_sram_we;
    wire [SRAM_ADDR_WIDTH-1:0]  mm_vec_sram_addr;
    /* verilator lint_off UNDRIVEN */
    wire [7:0]                  mm_vec_sram_din;
    wire [7:0]                  vec_sram_dout;

    // Connect matmul to vector SRAM
    matmul #(
        .MAX_DIM(MAX_DIM),
        .SRAM_ADDR_WIDTH(SRAM_ADDR_WIDTH)
    ) dut (
        .clk(clk),
        .rst(rst),
        .in_data(in_data),
        .in_valid(in_valid),
        .in_ready(in_ready),
        .out_data(out_data),
        .out_valid(out_valid),
        .out_ready(out_ready),
        .vec_sram_we(mm_vec_sram_we),
        .vec_sram_addr(mm_vec_sram_addr),
        .vec_sram_dout(vec_sram_dout)
    );
    
    // Instantiate vector SRAM
    sram #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(SRAM_ADDR_WIDTH),
        .DEPTH(SRAM_DEPTH)
    ) vec_sram (
        .clk(clk),
        .we(vec_sram_we | mm_vec_sram_we),
        .addr(vec_sram_we ? vec_sram_addr : mm_vec_sram_addr),
        .din(vec_sram_we ? vec_sram_din : mm_vec_sram_din),
        .dout(vec_sram_dout)
    );

endmodule
