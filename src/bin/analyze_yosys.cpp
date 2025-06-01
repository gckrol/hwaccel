/*
This application will parse the synthesis data from Yosys and calculate performance metrics.
It will estimate the cost of a complete PCIE board design at different process nodes,
including ASIC, DRAM controller, packaging, PCB, and RAM chips.
*/

#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// PCIE board component parameters
const double RAM_CAPACITY_GB = 12.0;       // Total RAM capacity in GB
const double RAM_COST_PER_GB = 5.0;        // USD per GB of RAM
const double MEMORY_BANDWIDTH_GB_S = 80.0; // Memory bandwidth in GB/s
const double PCB_BASE_COST = 30.0;         // Base PCB cost in USD
const double PACKAGING_BASE_COST = 10.0;   // Base packaging cost in USD

// Function to calculate system costs for a given node size
double calculate_system_cost(double area, double freq_ghz, double macs_per_cycle, 
                           double node_size, double target_gmac, 
                           double cost_per_um2, bool print_details) {
    // Performance calculation
    double gmac = macs_per_cycle * freq_ghz;
    double cores_needed = target_gmac / gmac;
    
    // ASIC core cost
    double asic_cost = area * cost_per_um2 * cores_needed;
    
    // Packaging cost (relatively fixed, but varies slightly with node size)
    double packaging_scaling = pow(node_size / 45.0, 0.3);
    double packaging_cost = PACKAGING_BASE_COST * packaging_scaling;
    
    // PCB cost (relatively fixed)
    double pcb_cost = PCB_BASE_COST;
    
    // RAM costs
    double ram_cost = RAM_CAPACITY_GB * RAM_COST_PER_GB;
    
    // Total system cost
    double total_cost = asic_cost + packaging_cost + pcb_cost + ram_cost;
    
    if (print_details) {
        printf("-> %0.0fnm:\n", node_size);
        printf("   Area        : %.2f µm²\n", area);
        printf("   Freq        : %.2f GHz\n", freq_ghz);
        printf("   GMAC        : %.2f\n", gmac);
        printf("   Cores needed: %.2f\n", cores_needed);
        printf("   ASIC cost   : $%.3f\n", asic_cost);
        printf("   Packaging   : $%.3f\n", packaging_cost);
        printf("   PCB         : $%.3f\n", pcb_cost);
        printf("   RAM (12GB)  : $%.3f\n", ram_cost);
        printf("   Total cost  : $%.3f\n\n", total_cost);
    }
    
    return total_cost;
}

int main() {
    char line[1024];
    double gates = 0, area = 0, delay_ps = 0;

    while (fgets(line, sizeof(line), stdin)) {
        if (strstr(line, "ABC:") && strstr(line, "Gates") && strstr(line, "Area") && strstr(line, "Delay")) {
            sscanf(line,
                "ABC: WireLoad = \"none\" Gates = %lf %*[^A]Area = %lf %*[^D]Delay = %lf",
                &gates, &area, &delay_ps);
        }
    }

    if (gates == 0 || area == 0 || delay_ps == 0) {
        fprintf(stderr, "Error: Failed to parse synthesis data.\n");
        return 1;
    }

    // Common parameters
    double macs_per_cycle = 1.0;
    double target_gmac = 100.0;
    double memory_bandwidth_gb_s = 80.0; // 80GB/s as specified
    
    // Frequency calculation
    double freq_ghz_45nm = 1000.0 / delay_ps;
    
    // Node sizes
    double from_node_size = 45.0;
    double to_node_size = 16.0;
    
    // Cost per unit area at different nodes
    double cost_per_um2_45nm = 0.0000035;  // in USD
    double cost_per_um2_16nm = 0.000015;   // in USD
    
    // Dennard scaling approximations
    double scaling_factor_area = pow(to_node_size / from_node_size, 1.5); // NB: worse for SRAM
    double scaling_factor_freq = pow(from_node_size / to_node_size, 0.5);
    
    // Calculate scaled metrics for 16nm
    double area_16nm = area * scaling_factor_area;
    double freq_ghz_16nm = freq_ghz_45nm * scaling_factor_freq;
    
    // Print results
    printf("=== Performance Summary ===\n");
    printf("   Gates                : %.0f\n", gates);
    printf("   Memory Bandwidth     : %.2f GB/s\n", memory_bandwidth_gb_s);
    printf("   Target Performance   : %.2f GMAC\n\n", target_gmac);

    // Calculate and print system costs for both nodes
    double cost_45nm = calculate_system_cost(area, freq_ghz_45nm, macs_per_cycle, 
                                         from_node_size, target_gmac, 
                                         cost_per_um2_45nm, true);
    
    double cost_16nm = calculate_system_cost(area_16nm, freq_ghz_16nm, macs_per_cycle, 
                                         to_node_size, target_gmac, 
                                         cost_per_um2_16nm, true);
    
    printf("Cost comparison: 16nm is %.2f%% of 45nm cost\n", 
           (cost_16nm / cost_45nm) * 100.0);
    
    return 0;
}
