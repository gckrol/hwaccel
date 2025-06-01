#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    // Performance @ 45nm
    double macs_per_cycle = 1.0;
    double freq_ghz = 1000.0 / delay_ps;
    double gmac_45 = macs_per_cycle * freq_ghz;

    // Dynamic scaling
    double from_node_size = 45.0; // Define the original node size (e.g., 45nm)
    double to_node_size = 16.0;  // Define the target node size (e.g., 16nm)

    // Dennard scaling approximations
    double scaling_factor_area = pow(to_node_size / from_node_size, 1.5); // NB: worse for SRAM.
    double scaling_factor_freq = pow(from_node_size / to_node_size, 0.5);

    double area_scaled = area * scaling_factor_area;
    double freq_scaled = freq_ghz * scaling_factor_freq;
    double gmac_scaled = macs_per_cycle * freq_scaled;

    // Cost estimation
    double cost_per_um2_45nm = 0.0000035;  // in USD
    double cost_per_um2_16nm = 0.000015;    

    double cost_from = area * cost_per_um2_45nm;
    double cost_to = area_scaled * cost_per_um2_16nm;

    // Print results
    printf("=== Performance Summary ===\n");
    printf("   Gates       : %.0f\n", gates);
    printf("\n");

    printf("-> %0.0fnm:\n", from_node_size);
    printf("   Area        : %.2f µm²\n", area);
    printf("   Freq        : %.2f GHz\n", freq_ghz);
    printf("   GMAC        : %.2f\n", gmac_45);
    printf("   Cost        : $%.3f\n\n", cost_from);

    printf("-> %0.0fnm (scaled):\n", to_node_size);
    printf("   Area        : %.2f µm²\n", area_scaled);
    printf("   Freq        : %.2f GHz\n", freq_scaled);
    printf("   GMAC        : %.2f\n", gmac_scaled);
    printf("   Cost        : $%.3f\n", cost_to);
    printf("\n");

    return 0;
}
