#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "circuit.h"
#include "ui.h"
#include "solver.h"
#include "io.h"
#include "util.h"

int main(void) {
    Circuit *ckt = circuit_create();
    char line[128];

    while (1) {
        ui_print_menu();
        printf("Command> ");
        if (ui_get_line(line, sizeof(line)) != 0) break;

        if (strcmp(line, "1") == 0) {
            printf("Enter resistance (ohm) and node (e.g. 1000 1): ");
            if (fgets(line, sizeof(line), stdin)) {
                double r; int n;
                if (sscanf(line, "%lf %d", &r, &n) == 2) {
                    circuit_add_component(ckt, COMP_RESISTOR, r, n, 0);
                    printf("Added resistor.\n");
                } else printf("Invalid input.\n");
            }
        } else if (strcmp(line, "2") == 0) {
            printf("Enter voltage (V) and node (e.g. 5 1): ");
            if (fgets(line, sizeof(line), stdin)) {
                double v; int n;
                if (sscanf(line, "%lf %d", &v, &n) == 2) {
                    circuit_add_component(ckt, COMP_VOLTAGE_SOURCE, v, n, 0);
                    printf("Added voltage source.\n");
                } else printf("Invalid input.\n");
            }
        } else if (strcmp(line, "3") == 0) {
            printf("Enter capacitance (F) and node (e.g. 1e-6 1): ");
            if (fgets(line, sizeof(line), stdin)) {
                double c; int n;
                if (sscanf(line, "%lf %d", &c, &n) == 2) {
                    circuit_add_component(ckt, COMP_CAPACITOR, c, n, 0);
                    printf("Added capacitor.\n");
                } else printf("Invalid input.\n");
            }
        } else if (strcmp(line, "4") == 0) {
            circuit_list(ckt);
        } else if (strcmp(line, "5") == 0) {
            double *volt = xmalloc(sizeof(double) * ckt->node_count);
            int ok = nodal_solve(ckt, volt);
            if (ok == 0) {
                for (int i = 0; i < ckt->node_count; ++i) printf("node %d: %.6g V\n", i, volt[i]);
            } else printf("Solve failed (code %d)\n", ok);
            free(volt);
        } else if (strcmp(line, "6") == 0) {
            printf("Enter dt and tmax (e.g. 1e-4 0.01): ");
            if (fgets(line, sizeof(line), stdin)) {
                double dt, tmax;
                if (sscanf(line, "%lf %lf", &dt, &tmax) == 2) {
                    double *times = NULL, *voltages = NULL; int n = 0;
                    int ok = rc_analytic_step(ckt, dt, tmax, &times, &voltages, &n);
                    if (ok == 0) {
                        printf("Enter CSV filename to export (e.g. out.csv): ");
                        if (fgets(line, sizeof(line), stdin)) {
                            line[strcspn(line, "\n")] = '\0';
                            io_export_csv(line, times, voltages, n, 1);
                            printf("Exported %d samples to %s\n", n, line);
                        }
                        free(times); free(voltages);
                    } else {
                        printf("RC analytic sim failed (code %d)\n", ok);
                    }
                } else printf("Invalid input.\n");
            }
        } else if (strcmp(line, "7") == 0) {
            printf("Enter filename to save: ");
            if (fgets(line, sizeof(line), stdin)) {
                line[strcspn(line, "\n")] = '\0';
                io_save(ckt, line);
                printf("Saved.\n");
            }
        } else if (strcmp(line, "8") == 0) {
            printf("Enter filename to load: ");
            if (fgets(line, sizeof(line), stdin)) {
                line[strcspn(line, "\n")] = '\0';
                io_load(ckt, line);
                printf("Loaded.\n");
            }
        } else if (strcmp(line, "9") == 0) {
            break;
        } else {
            printf("Unknown command.\n");
        }
    }

    circuit_free(ckt);
    return 0;
}
