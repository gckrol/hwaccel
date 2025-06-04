#include "dram.h"
#include <iostream>
#include <cassert>
#include <cstring>

// Function prototype
void clock_cycle(Dram &dram);

// Function to advance clock by one cycle
void clock_cycle(Dram &dram) {
    // Rising edge
    dram.clk = 1;
    dram.eval();
    
    // Falling edge
    dram.clk = 0;
    dram.eval();
}

int main() {
    srand(42); // Deterministic random seed for testing

    // Create a 1KB DRAM
    const size_t MEM_SIZE = 1024;
    Dram dram(MEM_SIZE);
    
    // Initialize memory with some test data
    for (size_t i = 0; i < MEM_SIZE; i++) {
        dram.data[i] = i & 0xFF;
    }
    
    std::cout << "DRAM Test Started" << std::endl;
    
    // Reset
    dram.in.rst = 1;
    clock_cycle(dram);
    dram.in.rst = 0;
    
    // Test 1: Single read
    std::cout << "Test 1: Single Read" << std::endl;
    dram.in.arvalid = 1;
    dram.in.araddr = 0x10;
    dram.in.arlen = 0;  // Single transfer
    dram.in.arsize = 0; // 1 byte
    dram.in.arburst = 1; // INCR burst type
    
    clock_cycle(dram);
    dram.in.arvalid = 0;
    
    bool read_completed = false;
    // Wait for the read to complete
    for (int i = 0; i < 15; i++) {
        clock_cycle(dram);
        printf("Cycle %d: arready=%d, rvalid=%d, rdata=0x%02X, rlast=%d\n", 
               i, dram.out.arready, dram.out.rvalid, 
               dram.out.rdata ? *dram.out.rdata : 0, dram.out.rlast);
        if (dram.out.rvalid) {
            read_completed = true;
            std::cout << "Read data at address 0x10: 0x" << std::hex << (int)(*dram.out.rdata) << std::dec << std::endl;
            assert(*dram.out.rdata == 0x10);
            assert(dram.out.rlast == true);
            break;
        }
    }
    assert(read_completed);
    
    // Test 2: Burst read
    std::cout << "Test 2: Burst Read" << std::endl;
    dram.in.arvalid = 1;
    dram.in.araddr = 0x20;
    dram.in.arlen = 3;   // 4 transfers
    dram.in.arsize = 0;  // 1 byte
    dram.in.arburst = 1; // INCR burst type
    
    clock_cycle(dram);
    dram.in.arvalid = 0;
    
    // Wait for the burst read to complete
    int transfers_received = 0;
    for (int i = 0; i < 20; i++) {
        clock_cycle(dram);
        if (dram.out.rvalid) {
            std::cout << "Burst read data " << transfers_received << ": 0x" 
                      << std::hex << (int)(*dram.out.rdata) << std::dec << std::endl;
            assert(*dram.out.rdata == (0x20 + transfers_received));
            
            transfers_received++;
            if (dram.out.rlast) {
                std::cout << "Burst read complete." << std::endl;
                break;
            }
        }
    }
    assert(transfers_received == 4);
    
    // Test 3: Burst read with 4-byte size
    std::cout << "Test 3: Burst Read with 4-byte size" << std::endl;
    dram.in.arvalid = 1;
    dram.in.araddr = 0x30;
    dram.in.arlen = 3;   // 4 transfers
    dram.in.arsize = 2;  // 4 bytes
    dram.in.arburst = 1; // INCR burst type
    
    clock_cycle(dram);
    dram.in.arvalid = 0;
    
    // Wait for the burst read to complete
    transfers_received = 0;
    for (int i = 0; i < 20; i++) {
        clock_cycle(dram);
        if (dram.out.rvalid) {
            uint32_t data = 0;
            // Read up to 4 bytes from rdata
            for (int j = 0; j < 4; j++) {
                if (dram.out.rdata) {
                    data |= (uint32_t)(*(dram.out.rdata + j)) << (j * 8);
                }
            }
            
            std::cout << "4-byte burst read data " << transfers_received << ": 0x" 
                      << std::hex << data << std::dec << std::endl;
            
            // For 4-byte transfers, each address increment is by 4
            uint32_t expected = 0;
            for (int j = 0; j < 4; j++) {
                expected |= (uint32_t)(*(dram.data + 0x30 + (transfers_received * 4) + j)) << (j * 8);
            }
            assert(data == expected);
            
            transfers_received++;
            if (dram.out.rlast) {
                std::cout << "4-byte burst read complete." << std::endl;
                break;
            }
        }
    }
    assert(transfers_received == 4);
    
    // Test 4: Larger burst read with bigger arsize and length
    std::cout << "Test 4: Larger Burst Read with 2-byte size and 8 transfers" << std::endl;
    dram.in.arvalid = 1;
    dram.in.araddr = 0x40;
    dram.in.arlen = 7;   // 8 transfers
    dram.in.arsize = 1;  // 2 bytes
    dram.in.arburst = 1; // INCR burst type
    
    clock_cycle(dram);
    dram.in.arvalid = 0;
    
    // Wait for the larger burst read to complete
    transfers_received = 0;
    for (int i = 0; i < 30; i++) {
        clock_cycle(dram);
        if (dram.out.rvalid) {
            uint16_t data = 0;
            // Read 2 bytes from rdata
            if (dram.out.rdata) {
                data = *dram.out.rdata;
                if (dram.out.rdata + 1) {  // Check if second byte is available
                    data |= (uint16_t)(*(dram.out.rdata + 1)) << 8;
                }
            }
            
            std::cout << "Larger burst read data " << transfers_received << ": 0x" 
                      << std::hex << data << std::dec << std::endl;
            
            // For 2-byte transfers, each address increment is by 2
            uint16_t expected = 0;
            expected = *(dram.data + 0x40 + (transfers_received * 2)) | 
                      (*(dram.data + 0x40 + (transfers_received * 2) + 1) << 8);
            assert(data == expected);
            
            transfers_received++;
            if (dram.out.rlast) {
                std::cout << "Larger burst read complete." << std::endl;
                break;
            }
        }
    }
    assert(transfers_received == 8);
    
    // Test 5: Extra large burst read with 4-byte size and 16 transfers
    std::cout << "Test 5: Extra Large Burst Read with 4-byte size and 16 transfers" << std::endl;
    dram.in.arvalid = 1;
    dram.in.araddr = 0x80;
    dram.in.arlen = 15;   // 16 transfers
    dram.in.arsize = 2;   // 4 bytes
    dram.in.arburst = 1;  // INCR burst type
    
    clock_cycle(dram);
    dram.in.arvalid = 0;
    
    // Wait for the extra large burst read to complete
    transfers_received = 0;
    for (int i = 0; i < 50; i++) {
        clock_cycle(dram);
        if (dram.out.rvalid) {
            uint32_t data = 0;
            // Read up to 4 bytes from rdata
            for (int j = 0; j < 4; j++) {
                if (dram.out.rdata && (dram.out.rdata + j)) {
                    data |= (uint32_t)(*(dram.out.rdata + j)) << (j * 8);
                }
            }
            
            std::cout << "Extra large burst read data " << transfers_received << ": 0x" 
                      << std::hex << data << std::dec << std::endl;
            
            // For 4-byte transfers, each address increment is by 4
            uint32_t expected = 0;
            for (int j = 0; j < 4; j++) {
                expected |= (uint32_t)(*(dram.data + 0x80 + (transfers_received * 4) + j)) << (j * 8);
            }
            assert(data == expected);
            
            transfers_received++;
            if (dram.out.rlast) {
                std::cout << "Extra large burst read complete." << std::endl;
                break;
            }
        }
    }
    assert(transfers_received == 16);
    
    std::cout << "DRAM Test Completed Successfully" << std::endl;
    return 0;
}