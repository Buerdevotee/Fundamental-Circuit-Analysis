#include <stdio.h>
#include <string.h>
#include "ui.h"

void ui_print_menu(void) {
    printf("\n=== BASIC CIRCUIT ANALYSER ===\n");
    printf("1) Add resistor (node - ground)\n");
    printf("2) Add voltage source (node - ground)\n");
    printf("3) Add capacitor (node - ground)\n");
    printf("4) List components\n");
    printf("5) Solve nodal (DC)\n");
    printf("6) RC transient (analytic if single R/C)\n");
    printf("7) Save circuit\n");
    printf("8) Load circuit\n");
    printf("9) Exit\n");
}

int ui_get_line(char *buf, int bufsize) {
    if (!fgets(buf, bufsize, stdin)) return -1;
    buf[strcspn(buf, "\n")] = '\0';
    return 0;
}
