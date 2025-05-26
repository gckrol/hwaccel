// rtl/matmul.v
module matmul (
    input clk,
    input rst,
    input start,
    input [7:0] a, b,
    output reg [15:0] y,
    output reg done
);
    always @(posedge clk) begin
        if (rst) begin
            y <= 0;
            done <= 0;
        end else if (start) begin
            y <= a * b;  // simple multiply for now
            done <= 1;
        end else begin
            done <= 0;
        end
    end
endmodule
