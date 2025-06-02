module matmul #(
    parameter MAX_DIM = 16,
    parameter SRAM_ADDR_WIDTH = 10
)(
    input         clk,
    input         rst,
    input  [7:0]  in_data,
    input         in_valid,
    output reg    in_ready,
    
    // New dimension inputs
    input  [7:0]  vdim,
    input  [7:0]  hdim,

    output reg [31:0] out_data,
    output reg        out_valid,
    input             out_ready,
    
    // SRAM interface for vector data
    output reg                    vec_sram_we,
    output reg [SRAM_ADDR_WIDTH-1:0] vec_sram_addr,
    input      [7:0]              vec_sram_dout
);

    // Stage 0: Initiate SRAM fetch.
    reg stage0_valid;
    reg [7:0] stage0_row_idx;
    reg [$clog2(MAX_DIM)-1:0] stage0_col_idx;
    reg [7:0] stage0_matrix_element;
    reg stage0_row_done;

    // Stage 1 waits for SRAM.
    reg stage1_valid;
    reg stage1_ready;
    assign stage1_ready = !stage1_valid || stage2_ready;
    reg stage1_row_done;
    reg [7:0] stage1_matrix_element;

    // Stage 2 does the multiplication and accumulation and outputs the result.
    reg stage2_valid;
    reg stage2_ready;
    assign stage2_ready = !stage2_valid || out_ready;
    reg [31:0] acc; // Accumulator for the current row.

    // Output:
    assign out_valid = stage2_valid;

    // Stage 0: Process input matrix elements
    always @(posedge clk) begin
        if (rst) begin
            in_ready <= 1;
            vec_sram_we <= 0;
            stage0_valid <= 0;
            stage0_col_idx <= 0;
            stage0_row_idx <= 0;
            stage0_row_done <= 0;
        end else begin
            if (in_valid && stage1_ready) begin
                // Pre-fetch the vector element for the next cycle
                // Also pipeline the matrix element, as it arrives in this cycle.
                $display("Fetching vector element at col_idx=%d", stage0_col_idx);
                vec_sram_we <= 0;
                vec_sram_addr <= {6'b0, stage0_col_idx};
                stage0_col_idx <= stage0_col_idx + 1;
                stage0_matrix_element <= in_data;
                stage0_valid <= 1;

                if ({4'b0, stage0_col_idx} + 1 == hdim) begin
                    stage0_row_done <= 1;
                    stage0_col_idx <= 0;
                    stage0_row_idx <= stage0_row_idx + 1;
                    if (stage0_row_idx + 1 == vdim) begin
                        // We're done with all matrix rows
                        in_ready <= 0;  // Stop accepting new input until reset
                    end
                end else begin
                    stage0_row_done <= 0;
                end                        
            end else if (stage1_ready) begin
                stage0_valid <= 0;
            end
        end
    end

    // Stage 1: wait for SRAM data.
    always @(posedge clk) begin
        if (rst) begin
            stage1_valid <= 0;
            stage1_row_done <= 0;
            stage1_matrix_element <= 0;
        end else if (stage0_valid && stage2_ready) begin
            stage1_row_done <= stage0_row_done;
            stage1_matrix_element <= stage0_matrix_element;
            stage1_valid <= 1;
        end else if (stage2_ready) begin
            stage1_valid <= 0;
        end
    end

    // Stage 2: perform multiplication and accumulation.
    always @(posedge clk) begin
        if (rst) begin
            stage2_valid <= 0;
            acc <= 0;
        end else if (stage1_valid && out_ready) begin
            $display("Stage 2: Multiplying matrix element %d with vector element %d", stage1_matrix_element, vec_sram_dout);
            acc <= acc + stage1_matrix_element * vec_sram_dout;
            if (stage1_row_done) begin
                $display("Stage 2: Row done, outputting accumulated value %d", acc);
                out_data <= acc + stage1_matrix_element * vec_sram_dout;
                stage2_valid <= 1;
                acc <= 0;
            end else begin
                stage2_valid <= 0;
            end
        end else if (out_ready) begin
            stage2_valid <= 0;
        end
    end

endmodule
