#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "circuit.h"
#include "util.h"

#define INITIAL_CAPACITY 8

Circuit *circuit_create(void) {
    Circuit *c = xmalloc(sizeof(Circuit));
    c->components = xmalloc(sizeof(Component) * INITIAL_CAPACITY);
    c->comp_count = 0;
    c->comp_capacity = INITIAL_CAPACITY;
    c->node_count = 1; /* node 0 = ground */
    return c;
}

void circuit_free(Circuit *c) {
    if (!c) return;
    free(c->components);
    free(c);
}

/* ensure capacity; return 0 ok, -1 fail */
static int ensure_capacity(Circuit *c) {
    if (c->comp_count < c->comp_capacity) return 0;
    int newcap = c->comp_capacity * 2;
    Component *tmp = realloc(c->components, sizeof(Component) * newcap);
    if (!tmp) return -1;
    c->components = tmp;
    c->comp_capacity = newcap;
    return 0;
}

int circuit_add_component(Circuit *c, ComponentType type, double value, int npos, int nneg) {
    if (!c) return -1;
    if (npos < 0 || nneg < 0) return -1;
    if (ensure_capacity(c) != 0) return -1;
    Component comp;
    comp.id = (c->comp_count == 0) ? 1 : (c->components[c->comp_count - 1].id + 1);
    comp.type = type;
    comp.value = value;
    comp.node_pos = npos;
    comp.node_neg = nneg;
    c->components[c->comp_count++] = comp;
    int maxnode = (npos > nneg) ? npos : nneg;
    if (maxnode + 1 > c->node_count) c->node_count = maxnode + 1;
    return comp.id;
}

void circuit_list(const Circuit *c) {
    if (!c) return;
    printf("Components (count=%d):\n", c->comp_count);
    for (int i = 0; i < c->comp_count; ++i) {
        const Component *p = &c->components[i];
        const char *tstr = (p->type == COMP_RESISTOR) ? "R" :
                           (p->type == COMP_VOLTAGE_SOURCE) ? "V" : "C";
        printf("  id=%d %s value=%.6g nodes=%d-%d\n", p->id, tstr, p->value, p->node_pos, p->node_neg);
    }
}
