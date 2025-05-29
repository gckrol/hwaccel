module matmul #(
    parameter MAX_DIM = 16
)(
    input         clk,
    input         rst,
    input  [7:0]  in_data,
    input         in_valid,
    output reg    in_ready,

    output reg [31:0] out_data,
    output reg        out_valid,
    input             out_ready
);

    // FSM states
    parameter IDLE = 3'd0;
    parameter READ_DIM = 3'd1;
    parameter READ_VEC = 3'd2;
    parameter READ_MATRIX = 3'd3;

    reg [2:0] state;

    // Dimensions
    reg [7:0] vdim, hdim;
    reg [7:0] row_idx;
    reg [$clog2(MAX_DIM)-1:0] col_idx;

    // Storage
    reg [7:0] vector    [0:MAX_DIM-1];

    // Accumulator
    reg [31:0] acc;

    // Read counter
    reg [$clog2(MAX_DIM)-1:0] count;

    always @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
            out_valid <= 0;
            in_ready <= 1;
        end else begin
            case (state)
                IDLE: begin
                    out_valid <= 0;
                    if (in_valid) begin
                        vdim <= in_data;
                        state <= READ_DIM;
                    end
                end

                READ_DIM: begin
                    if (in_valid) begin
                        hdim <= in_data;
                        count <= 0;
                        state <= READ_VEC;
                    end
                end

                READ_VEC: begin
                    if (in_valid) begin
                        vector[count] <= in_data;
                        count <= count + 1;
                        if ({4'b0, count} + 1 == hdim) begin
                            count <= 0;
                            row_idx <= 0;
                            acc <= 0;
                            state <= READ_MATRIX;
                        end
                    end
                end

                READ_MATRIX: begin
                    if (in_valid && out_ready) begin
                        acc <= acc + in_data * vector[col_idx];
                        col_idx <= col_idx + 1;
                        if ({4'b0, col_idx} + 1 == hdim) begin
                            col_idx <= 0;
                            out_data <= acc + in_data * vector[col_idx];
                            out_valid <= 1;
                            acc <= 0;
                            row_idx <= row_idx + 1;
                            if (row_idx + 1 == vdim) begin
                                // We're done.
                                row_idx <= 0;
                                state <= IDLE;
                                in_ready <= 1;
                            end
                        end else begin
                            out_valid <= 0;
                        end
                    end
                end

                default: begin
                    state <= IDLE;
                end
            endcase
        end
    end
endmodule
