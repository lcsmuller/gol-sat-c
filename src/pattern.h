#ifndef GOLSAT_PATTERN_H
#define GOLSAT_PATTERN_H

#include <stdio.h>
#include <stdlib.h>

enum golsat_cellstate {
    GOLSAT_CELLSTATE_ALIVE,
    GOLSAT_CELLSTATE_DEAD,
    GOLSAT_CELLSTATE_UNKNOWN
};

struct golsat_pattern {
    int width;
    int height;
    enum golsat_cellstate *cells;
};

struct golsat_pattern *golsat_pattern_create(FILE *file);
void golsat_pattern_cleanup(struct golsat_pattern *pattern);
enum golsat_cellstate golsat_pattern_get_cell(
    const struct golsat_pattern *pattern, int x, int y);
int golsat_pattern_is_empty(const struct golsat_pattern *pattern);

#endif /* !GOLSAT_PATTERN_H */
