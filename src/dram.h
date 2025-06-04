#ifndef DRAM_AXI4_H
#define DRAM_AXI4_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>

/**
 * C++ dram implementation, accessed over AXI4.
 * Only supports INCR burst type.
 */
class Dram {
public:
    // Internal storage
    uint8_t *data;
    size_t size;

    // General
    bool clk = 0;
    int cycle = 0; // Current cycle number

    // Input
    typedef struct Input {
        bool arvalid = false; // Read address valid
        size_t araddr = 0;  // Read address
        size_t arlen = 0;    // Burst length
        size_t arsize = 0;   // Size of each transfer (log2 of byte size)
        size_t arburst = 0;  // Burst type (0: FIXED, 1: INCR, 2: WRAP)

        bool rready = true; // Data read ready

        bool rst;
    } Input;

    Input in;

    // Output
    typedef struct Output {
        bool arready = true; // Read address ready

        bool rvalid = false; // Data valid
        uint8_t *rdata = nullptr; // Read data
        int rresp = 0; // Read response (0: OKAY, 1: EXOKAY, 2: SLVERR, 3: DECERR)
        bool rlast = false; // Last transfer in burst
    } Output;

    Output out;
    Output next;
    
    typedef struct ReadRequest {
        size_t address; // Address to read from
        size_t burst_length; // Number of transfers in the burst
        size_t bytes_per_transfer; // Size of each transfer in bytes
        size_t transfers_remaining; // Transfers remaining in the burst
        int completion_cycle; // Cycle when the burst will complete
    } ReadRequest;
    std::vector<ReadRequest> read_requests;

    Dram(size_t size) {
        this->size = size;
        data = new uint8_t[size];
        // Initialize memory to a known pattern.
        for (size_t i = 0; i < size; i++) {
            data[i] = 0xAB;
        }
    }

    ~Dram() {
        delete[] data;
    }

    void eval() {
        if (!clk) {
            // Make output visible.
            out = next;
            return;
        }
        cycle++;

        if (in.rst) {
            next = Output();
            return;
        }
        if (in.arvalid) {
            assert(in.arburst == 1); // Only INCR burst type is supported.
            
            // Put read request in the queue.
            ReadRequest req;
            req.address = in.araddr;
            req.burst_length = in.arlen + 1; // arlen is burst length - 1
            req.bytes_per_transfer = 1 << in.arsize; // 2^arsize
            req.transfers_remaining = req.burst_length;
            req.completion_cycle = cycle + 10 + rand()%3;
            read_requests.push_back(req);

            // Keep arready high - we always accept new read requests.
        }

        if (in.rready) {
            // Data read completed by the master.
            next.rvalid = false;
        }

        if (in.rready && !read_requests.empty()) {
            // Master is ready, and we have active read requests.
            ReadRequest &req = read_requests.front();
            if (cycle >= req.completion_cycle) {
                size_t address = req.address + (req.burst_length - req.transfers_remaining) * req.bytes_per_transfer;
                next.rdata = &data[address];
                next.rvalid = true;
                next.rresp = 0; // OKAY response

                req.transfers_remaining--;
                // Check if this is the last transfer
                if (!req.transfers_remaining) {
                    next.rlast = true;
                    read_requests.erase(read_requests.begin()); // Remove request
                } else {
                    next.rlast = false;
                }
            }
        }
    }
};

#endif // DRAM_AXI4_H
