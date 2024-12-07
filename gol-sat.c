#include <stdio.h>
#include <stdlib.h>

#include "commandline.h"
#include "field.h"
#include "formula.h"
#include "pattern.h"

int
main(int argc, char **argv)
{
    int exit_status = EXIT_FAILURE;

    struct golsat_field_init init = { 0 };
    struct golsat_field **fields;
    CMergeSat *s;
    FILE *f;

    struct golsat_options options = { 0 };

    if (!golsat_commandline_parse(argc, argv, &options)) {
        return EXIT_FAILURE;
    }

    printf("-- Reading pattern from file: %s\n", options.pattern);
    if (!(f = fopen(options.pattern, "r"))) {
        printf("-- Error: Cannot open %s\n", options.pattern);
        return EXIT_FAILURE;
    }

    struct golsat_pattern *pat = golsat_pattern_create(f);
    if (!pat) {
        fprintf(stderr, "-- Error: Pattern creation failed.\n");
        goto _cleanup_file;
    }

    printf("-- Building formula for %d evolution steps...\n",
           options.evolutions);
    fields = (struct golsat_field **)malloc(sizeof(struct golsat_field *)
                                            * (options.evolutions + 1));
    s = cmergesat_init();
    if (!fields) {
        fprintf(stderr, "-- Error: Memory allocation for fields failed.\n");
        goto _cleanup_pat;
    }

    for (int g = 0; g <= options.evolutions; ++g) {
        int width = pat->width;
        int height = pat->height;

        // Adjust field size based on evolution and growth options
        if (options.grow) {
            const int growth =
                (options.backwards) ? (options.evolutions - g) : g;
            width += 2 * growth;
            height += 2 * growth;
        }

        // Create field for the current generation
        if (!(fields[g] = golsat_field_create(s, width, height, &init))) {
            fprintf(stderr,
                    "-- Error: Field creation failed for generation %d.\n", g);
            goto _cleanup_sat;
        }

        // Add transitions between generations
        if (g > 0) {
            golsat_formula_transition(s, fields[g - 1], fields[g]);
        }
    }

    if (options.backwards) {
        printf("-- Setting pattern constraint on last generation...\n");
        golsat_formula_constraint(s, fields[options.evolutions], pat);
    }
    else {
        printf("-- Setting pattern constraint on first generation...\n");
        golsat_formula_constraint(s, fields[0], pat);
    }

    printf("-- Solving formula...\n");
    switch (cmergesat_solve(s)) {
    case 10:
        printf("\n");
        for (int g = 0; g <= options.evolutions; ++g) {
            if (options.backwards) {
                if (g == 0)
                    printf("-- Initial generation:\n");
                else if (g == options.evolutions)
                    printf("-- Evolves to final generation (from pattern):\n");
                else
                    printf("-- Evolves to:\n");
            }
            else {
                if (g == 0)
                    printf("-- Initial generation (from pattern):\n");
                else if (g == options.evolutions)
                    printf("-- Evolves to final generation:\n");
                else
                    printf("-- Evolves to:\n");
            }
            golsat_field_print(s, fields[g], stdout);
            printf("\n");
        }
        exit_status = EXIT_SUCCESS;
        break;
    case 0:
        fprintf(stderr, "-- Formula is not solvable.\n");
        break;
    case 20:
    default:
        fprintf(stderr,
                "-- Formula is not solvable. The selected pattern is probably "
                "too restrictive!\n");
        break;
    }
    printf("-- Formula statistics:\n");
    cmergesat_print_statistics(s);

_cleanup_sat:
    for (int g = 0; g <= options.evolutions; ++g)
        golsat_field_cleanup(fields[g]);
    free(fields);
    cmergesat_release(s);
_cleanup_pat:
    golsat_pattern_cleanup(pat);
_cleanup_file:
    fclose(f);

    return exit_status;
}
