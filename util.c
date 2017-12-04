/*
EAGLE: explicit alternative genome likelihood evaluator
Given the sequencing data and candidate variant, explicitly test 
the alternative hypothesis against the reference hypothesis

Copyright 2016 Tony Kuo
This program is distributed under the terms of the GNU General Public License
*/

#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "util.h"

#define M_1_LOG10E (1.0/M_LOG10E)
#define LG3 (log(3.0))

char *strdup1(const char *src) {
    size_t n = strlen(src) + 1;
    char *des = malloc(n * sizeof *des);
    des[n] = '\0';
    return des ? memcpy(des, src, n) : NULL;
}

void str_resize(char **str, size_t size) {
    char *p = realloc(*str, size * sizeof *str);
    if (p == NULL) { exit_err("failed to realloc in str_resize\n"); }
    else { *str = p; }
}

int has_numbers(const char *str) {
    while (*str != '\0') {
        if (isdigit(*str++) == 1) return 1;
    }
    return 0;
}

int parse_int(const char *str) {
    errno = 0;
    char *end;
    int num = strtol(str, &end, 0);
    if (end != str && *end != '\0') { exit_err("failed to convert '%s' to int with leftover string '%s'\n", str, end); }
    return num;
}

float parse_float(const char *str) {
    errno = 0;
    char *end;
    double num = strtof(str, &end);
    if (end != str && *end != '\0') { exit_err("failed to convert '%s' to float with leftover string '%s'\n", str, end); }
    return num;
}

double sum(const double *a, int size) {
    double s = 0;
    while (--size >= 0) s += a[size];
    return s;
}

double *reverse(double *a, int size) {
    int i = 0;
    double *b = malloc(size * sizeof *b);
    while (--size >= 0) b[i++] = a[size];
    return b;
}

double log_add_exp(double a, double b) {
    double max_exp = a > b ? a : b;
    return log(exp(a - max_exp) + exp(b - max_exp)) + max_exp;
}

double log_sum_exp(const double *a, size_t size) {
    int i;
    double max_exp = a[0]; 
    for (i = 1; i < size; ++i) { 
        if (a[i] > max_exp) max_exp = a[i]; 
    }
    double s = 0;
    for (i = 0; i < size; ++i) s += exp(a[i] - max_exp);
    return log(s) + max_exp;
}

void init_seqnt_map(int *seqnt_map) {
    /* Mapping table, symmetrical according to complement */
    memset(seqnt_map, 0, sizeof(int) * 26);

    seqnt_map['A'-'A'] = 0;
    seqnt_map['C'-'A'] = 1;

    /* Ambiguous codes */
    seqnt_map['H'-'A'] = 2; // A, C, T
    seqnt_map['B'-'A'] = 3; // C, G, T
    seqnt_map['R'-'A'] = 4; // A, G
    seqnt_map['K'-'A'] = 5; // G, T
    seqnt_map['S'-'A'] = 6; // G, C
    seqnt_map['W'-'A'] = 7; // A, T

    seqnt_map['N'-'A'] = 8;
    seqnt_map['X'-'A'] = 8;

    // W also in 9, S also in 10
    seqnt_map['M'-'A'] = 11; // A, C
    seqnt_map['Y'-'A'] = 12; // C, T
    seqnt_map['V'-'A'] = 13; // A, C, G
    seqnt_map['D'-'A'] = 14; // A, G, T

    seqnt_map['G'-'A'] = 15;
    seqnt_map['T'-'A'] = 16;
    seqnt_map['U'-'A'] = 16;
}

void init_q2p_table(double *p_match, double *p_mismatch, size_t size) {
    /* FastQ quality score to ln probability lookup table */
    int i;
    double a;
    for (i = 0; i < size; ++i) { 
        if (i == 0) a = -0.01;
        else a = (double)i / -10 * M_1_LOG10E; //convert to ln
        p_match[i] = log(1 - exp(a)); // log(1-err)
        p_mismatch[i] = a - LG3; // log(err/3)
     }
}

void init_dp_q2p_table(double *p_match, double *p_mismatch, size_t size, int match, int mismatch) {
    /* FastQ quality score to ln probability lookup table modified by match and mismatch costs for dp*/
    int i;
    double a;
    for (i = 0; i < size; ++i) { 
        if (i == 0) a = -0.01;
        else a = (double)i / -10 * M_1_LOG10E; //convert to ln
        p_match[i] = log_add_exp(log(1 - exp(a)) + match, a - LG3 - mismatch);
        p_mismatch[i] = log_add_exp(a - LG3 + match, log(1 - exp(a)) - mismatch);
     }
}

