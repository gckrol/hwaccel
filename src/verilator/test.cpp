#include "Vmatmul_tb.h"
#include "verilated.h"
#include <cstdio>
#include <iostream>
#include <vector>

// Function to perform matrix-vector multiplication using the matmul hardware
std::vector<uint32_t> hw_matmul(const std::vector<std::vector<uint8_t>>& matrix, const std::vector<uint8_t>& vector) {
    // Create and initialize the hardware module
    Vmatmul_tb* dut = new Vmatmul_tb;
    
    // Reset the hardware
    dut->clk = 0; dut->rst = 1; dut->eval();
    dut->clk = 1; dut->eval();
    dut->clk = 0; dut->rst = 0; dut->eval();
    
    // Prepare dimensions
    uint8_t vdim = matrix.size();    // Number of rows
    uint8_t hdim = vector.size();    // Number of columns
    
    // Set up for input
    dut->in_valid = 0;
    dut->out_ready = 1;
    dut->vec_sram_we = 0;
    dut->eval();
    
    // Preload vector data into the SRAM
    for (int i = 0; i < hdim; i++) {
        dut->vec_sram_we = 1;
        dut->vec_sram_addr = i;
        dut->vec_sram_din = vector[i];
        dut->clk = 1; dut->eval();
        dut->clk = 0; dut->eval();
    }
    dut->vec_sram_we = 0;
    printf("Vector loaded into SRAM.\n");
    
    // Set dimensions directly in the registers
    dut->vdim = vdim;
    dut->hdim = hdim;
    
    // Continue with READ_MATRIX state
    printf("Matrix dimensions set: %d x %d\n", vdim, hdim);
    dut->in_valid = 1;
    
    // Send matrix rows and collect results
    std::vector<uint32_t> results;
    results.reserve(vdim);
    
    printf("Starting matrix-vector multiplication...\n");
    for (int row = 0; row < vdim; row++) {
        // Send each element in the row
        for (int col = 0; col < hdim; col++) {
            printf("Sending row %d, col %d: %u\n", row, col, matrix[row][col]);
            dut->in_data = matrix[row][col];
            dut->clk = 1; dut->eval();
            
            // Check if result is ready
            if (dut->out_valid) {
                uint32_t d = dut->out_data;
                results.push_back(d);
                printf("Row %d, Col %d: %u\n", row, col, d);
            }
            
            dut->clk = 0; dut->eval();
        }
    }
    printf("Matrix-vector multiplication completed.\n");
    
    // Make sure we've received all results
    while (results.size() < vdim) {
        // printf("Waiting for result %d of %d...\n", results.size(), vdim);
        dut->in_valid = 0;
        dut->clk = 1; dut->eval();
        
        if (dut->out_valid) {
            printf("Received result: %u\n", dut->out_data);
            results.push_back(dut->out_data);
        }
        
        dut->clk = 0; dut->eval();
    }
    printf("All results received.\n");
    
    // Clean up
    delete dut;
    return results;
}

// Software implementation of matrix-vector multiplication
std::vector<uint32_t> sw_matmul(const std::vector<std::vector<uint8_t>>& matrix, const std::vector<uint8_t>& vector) {
    std::vector<uint32_t> result;
    result.reserve(matrix.size());
    
    for (size_t i = 0; i < matrix.size(); i++) {
        uint32_t sum = 0;
        for (size_t j = 0; j < vector.size(); j++) {
            sum += static_cast<uint32_t>(matrix[i][j]) * static_cast<uint32_t>(vector[j]);
        }
        result.push_back(sum);
    }
    
    return result;
}

int main() {
    // Define a 3x4 matrix and a 4-element vector
    std::vector<std::vector<uint8_t>> matrix = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };
    
    std::vector<uint8_t> vector = {4, 3, 2, 1};
    
    // Perform matrix-vector multiplication using hardware
    std::cout << "Hardware implementation:" << std::endl;
    std::vector<uint32_t> hw_result = hw_matmul(matrix, vector);
    
    // Perform matrix-vector multiplication using software
    std::cout << "\nSoftware implementation:" << std::endl;
    std::vector<uint32_t> sw_result = sw_matmul(matrix, vector);
    
    // Print and compare results
    std::cout << "\nResults comparison:" << std::endl;
    std::cout << "Row\tHardware\tSoftware\tMatch" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    bool all_match = true;
    for (size_t i = 0; i < hw_result.size(); i++) {
        bool match = (hw_result[i] == sw_result[i]);
        std::cout << i << "\t" << hw_result[i] << "\t\t" << sw_result[i] << "\t\t" 
                  << (match ? "Yes" : "No") << std::endl;
        if (!match) all_match = false;
    }
    
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Overall match: " << (all_match ? "Yes" : "No") << std::endl;
    
    return 0;
}
