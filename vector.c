/*
EAGLE: explicit alternative genome likelihood evaluator
Given the sequencing data and candidate variant, explicitly test 
the alternative hypothesis against the reference hypothesis

Copyright 2016 Tony Kuo
This program is distributed under the terms of the GNU General Public License
*/

#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include "util.h"
#include "vector.h"

void vector_init(vector_t *a, size_t initial_size, enum type var_type) {
    a->len = 0;
    a->type = var_type;
    a->size = initial_size;
    a->data = malloc(initial_size * sizeof (void *));
}

vector_t *vector_create(size_t initial_size, enum type var_type) {
    vector_t *a = malloc(sizeof (vector_t));
    vector_init(a, initial_size, var_type);
    return a;
}

void vector_free(vector_t *a) {
    if (a != NULL) {
        a->len = 0;
        free(a->data); a->data = NULL;
        free(a); a = NULL;
    }
}

void vector_destroy(vector_t *a) {
    if (a != NULL) {
        size_t i;
        enum type var_type = a->type;
        for (i = 0; i < a->len; i++) {
            switch (var_type) {
                case STATS_T:
                    stats_destroy((stats_t *)a->data[i]);
                    break;
                case VARIANT_T:
                    variant_destroy((variant_t *)a->data[i]);
                    break;
                case READ_T:
                    read_destroy((read_t *)a->data[i]);
                    break;
                case FASTA_T:
                    fasta_destroy((fasta_t *)a->data[i]);
                    break;
                default:
                    break;
            }
            free(a->data[i]); a->data[i] = NULL;
        }
        a->len = a->size = 0;
        free(a->data); a->data = NULL;
    }
}

void vector_add(vector_t *a, void *entry) {
    if (a->len >= a->size) {
        a->size *= 2;
        void **p = realloc(a->data, a->size * sizeof (void *));
        if (p == NULL) { exit_err("failed to realloc in vector_add\n"); }
        else { a->data = p; }
    }
    a->data[a->len++] = entry;
}

void vector_del(vector_t *a, size_t i) {
    a->data[i] = NULL;
    if (i == --a->len) return;
    memmove(&(a->data[i]), &(a->data[i + 1]), (a->len - i) * sizeof (void *));
    a->data[a->len] = NULL;
}

void *vector_pop(vector_t *a) {
    if (a->len <= 0) return NULL;
    void *entry = a->data[--a->len];
    a->data[a->len] = NULL;
    return entry;
}

vector_t *vector_dup_shallow(vector_t *a) {
    vector_t *v = vector_create(a->size, a->type);
    v->len = a->len;
    memcpy(&(v->data[0]), &(a->data[0]), a->len * sizeof (void *));
    return v;
}

void vector_int_init(vector_int_t *a, size_t initial_size) {
    a->len = 0;
    a->size = initial_size;
    a->data = malloc(initial_size * sizeof (int));
}

vector_int_t *vector_int_create(size_t initial_size) {
    vector_int_t *a = malloc(sizeof (vector_int_t));
    vector_int_init(a, initial_size);
    return a;
}

void vector_int_free(vector_int_t *a) {
    if (a != NULL) {
        a->len = 0;
        free(a->data); a->data = NULL;
        free(a); a = NULL;
    }
}

void vector_int_add(vector_int_t *a, int entry) {
    if (a->len >= a->size) {
        a->size *= 2;
        int *p = realloc(a->data, a->size * sizeof (int));
        if (p == NULL) { exit_err("failed to realloc in vector_add\n"); }
        else { a->data = p; }
    }
    a->data[a->len++] = entry;
}

void vector_int_del(vector_int_t *a, size_t i) {
    a->data[i] = 0;
    if (i == --a->len) return;
    memmove(&(a->data[i]), &(a->data[i + 1]), (a->len - i) * sizeof (int));
    a->data[a->len] = 0;
}

vector_int_t *vector_int_dup(vector_int_t *a) {
    vector_int_t *v = vector_int_create(a->size);
    v->len = a->len;
    memcpy(&(v->data[0]), &(a->data[0]), a->len * sizeof (int));
    return v;
}

void vector_double_init(vector_double_t *a, size_t initial_size) {
    a->len = 0;
    a->size = initial_size;
    a->data = malloc(initial_size * sizeof (double));
}

vector_double_t *vector_double_create(size_t initial_size) {
    vector_double_t *a = malloc(sizeof (vector_double_t));
    vector_double_init(a, initial_size);
    return a;
}

void vector_double_free(vector_double_t *a) {
    if (a != NULL) {
        a->len = 0;
        free(a->data); a->data = NULL;
        free(a); a = NULL;
    }
}

void vector_double_add(vector_double_t *a, double entry) {
    if (a->len >= a->size) {
        a->size *= 2;
        double *p = realloc(a->data, a->size * sizeof (double));
        if (p == NULL) { exit_err("failed to realloc in vector_add\n"); }
        else { a->data = p; }
    }
    a->data[a->len++] = entry;
}

void vector_double_del(vector_double_t *a, size_t i) {
    a->data[i] = 0;
    if (i == --a->len) return;
    memmove(&(a->data[i]), &(a->data[i + 1]), (a->len - i) * sizeof (double));
    a->data[a->len] = 0;
}

vector_double_t *vector_double_dup(vector_double_t *a) {
    vector_double_t *v = vector_double_create(a->size);
    v->len = a->len;
    memcpy(&(v->data[0]), &(a->data[0]), a->len * sizeof (double));
    return v;
}

variant_t *variant_create(char *chr, int pos, char *ref, char *alt) {
    variant_t *v = malloc(sizeof (variant_t));
    v->chr = strdup(chr);
    v->pos = pos;
    v->ref = strdup(ref);
    v->alt = strdup(alt);
    return v;
}

void variant_destroy(variant_t *v) {
    if (v) {
        v->pos = 0;
        free(v->chr); v->chr = NULL;      
        free(v->ref); v->ref = NULL;
        free(v->alt); v->alt = NULL;
    }
}

read_t *read_create(char *name, int tid, char *chr, int pos) {
    read_t *r = malloc(sizeof (read_t));
    r->name = strdup(name);
    r->tid = tid;
    r->chr = strdup(chr);
    r->pos = pos;
    r->end = pos;
    r->prgu = -DBL_MAX;
    r->prgv = -DBL_MAX;
    r->pout = -DBL_MAX;
    r->index = 0;
    r->var_list = vector_create(1, VOID_T);

    r->length = r->n_cigar = r->inferred_length = r->multimapNH = r->n_splice = 0;
    r->qseq = NULL;
    r->qual = NULL;
    r->flag = NULL;
    r->cigar_opchr = NULL;
    r->cigar_oplen = NULL;
    r->splice_pos = NULL;
    r->splice_offset = NULL;
    r->multimapXA = NULL;
    r->is_dup = 0;
    r->is_reverse = 0;
    r->is_secondary = 0;
    r->is_read2 = 0;
    return r;
}

void read_destroy(read_t *r) {
    if (r != NULL) {
        r->tid = r->pos = r->end = r->length = r->n_cigar = r->inferred_length = r->multimapNH = r->n_splice = 0;
        r->is_dup = r->is_reverse = r->is_secondary = r->is_read2 = 0;
        r->prgu = r->prgv = r->pout = 0;
        r->index = 0;
        free(r->name); r->name = NULL;
        free(r->chr); r->chr = NULL;
        free(r->qseq); r->qseq = NULL;
        free(r->qual); r->qual = NULL;
        free(r->flag); r->flag = NULL;
        free(r->cigar_opchr); r->cigar_opchr = NULL;
        free(r->cigar_oplen); r->cigar_oplen = NULL;
        free(r->splice_pos); r->splice_pos = NULL;
        free(r->splice_offset); r->splice_offset = NULL;
        free(r->multimapXA); r->multimapXA = NULL;
        vector_destroy(r->var_list); free(r->var_list); r->var_list = NULL;
    }
}

fasta_t *fasta_create(char *name) {
    fasta_t *f = malloc(sizeof (fasta_t));
    f->name = strdup(name);
    return f;
}

void fasta_destroy(fasta_t *f) {
    if (f != NULL) {
        f->seq_length = 0;
        free(f->seq); f->seq = NULL;      
        free(f->name); f->name = NULL;
    }
}

region_t *region_create(char *chr, int pos1, int pos2) {
    region_t *g = malloc(sizeof (region_t));
    g->chr = strdup(chr);
    g->pos1 = pos1;
    g->pos2 = pos2;
    return g;
}

void region_destroy(region_t *g) {
    if (g != NULL) {
        g->pos1 = g->pos2 = 0;
        free(g->chr); g->chr = NULL;      
    }
}

stats_t *stats_create(vector_int_t *combo, int nreads) {
    stats_t *s = malloc(sizeof (stats_t));
    s->combo = combo;
    s->read_prgv = vector_double_create(nreads);
    s->ref = 0;
    s->het = 0;
    s->alt = 0;
    s->mut = 0;
    s->ref_count = 0;
    s->alt_count = 0;
    s->seen = 0;
    return s;
}

void stats_destroy(stats_t *s) {
    if (s != NULL) {
        s->ref = s->het = s->alt = s->mut = s->ref_count = s->alt_count = 0;
        vector_int_free(s->combo);
        vector_double_free(s->read_prgv);
    }
}

int nat_sort_cmp(const void *a, const void *b, enum type var_type) {
    char *str1, *str2;
    switch (var_type) {
        case VARIANT_T: {
            variant_t *c1 = *(variant_t **)a;
            variant_t *c2 = *(variant_t **)b;
            if (strcasecmp(c1->chr, c2->chr) == 0) return (c1->pos > c2->pos) - (c1->pos < c2->pos);
            str1 = strdup(c1->chr);
            str2 = strdup(c2->chr);
            c1 = NULL;
            c2 = NULL;
            break;
        }
        case REGION_T: {
            region_t *c1 = *(region_t **)a;
            region_t *c2 = *(region_t **)b;
            if (strcasecmp(c1->chr, c2->chr) == 0) return ((c1->pos1 > c2->pos1) - (c1->pos1 < c2->pos1)) + ((c1->pos2 > c2->pos2) - (c1->pos2 < c2->pos2));
            str1 = strdup(c1->chr);
            str2 = strdup(c2->chr);
            c1 = NULL;
            c2 = NULL;
            break;
        }
        default:
            str1 = strdup(*(char **)a);
            str2 = strdup(*(char **)b);
            break;
    }
    char *s1 = str1;
    char *s2 = str2;
    int cmp = 0;
    while (cmp == 0 && *s1 != '\0' && *s2 != '\0') {
        if (isspace(*s1) && isspace(*s2)) { // ignore whitespace
            s1 += 1;
            s2 += 1;
        }
        else if ((isalpha(*s1) && isalpha(*s2)) || (ispunct(*s1) && ispunct(*s2))) { // compare alphabet and punctuation
            *s1 = tolower(*s1);
            *s2 = tolower(*s2);
            cmp = (*s1 > *s2) - (*s1 < *s2);
            s1 += 1;
            s2 += 1;
        }
        else { // compare digits
            int i1, i2, n1, n2, t1, t2;
            t1 = sscanf(s1, "%d%n", &i1, &n1);
            if (t1 == 0) t1 = sscanf(s1, "%*[^0123456789]%d%n", &i1, &n1);
            t2 = sscanf(s2, "%d%n", &i2, &n2);
            if (t2 == 0) t2 = sscanf(s2, "%*[^0123456789]%d%n", &i2, &n2);

            if (t1 < 1 || t2 < 1) { // one string has no digits
                cmp = strcmp(s1, s2);
            }
            else {
                cmp = (i1 > i2) - (i1 < i2);
                if (cmp == 0) { // first set of digits are equal, check further
                    s1 += n1;
                    s2 += n2;
                }
            }
        }
    }
    free(str1); str1 = NULL;
    free(str2); str2 = NULL;
    return cmp;
}

int nat_sort_vector(const void *a, const void *b) {
    return nat_sort_cmp(a, b, VOID_T);
}

int nat_sort_variant(const void *a, const void *b) {
    return nat_sort_cmp(a, b, VARIANT_T);
}

int nat_sort_region(const void *a, const void *b) {
    return nat_sort_cmp(a, b, REGION_T);
}

read_t *read_fetch(bam_hdr_t *bam_header, bam1_t *aln, int pao, int isc, int nodup, int splice, int phred64, int const_qual) {
    int i, j;
    if (aln->core.tid < 0) return NULL; // not mapped
    read_t *read = read_create((char *)aln->data, aln->core.tid, bam_header->target_name[aln->core.tid], aln->core.pos);

    char *flag = bam_flag2str(aln->core.flag);
    if (flag != NULL) read->flag = strdup(flag);
    else read->flag = NULL;
    free(flag); flag = NULL;

    int n;
    char *s, token[strlen(read->flag) + 1];
    for (s = read->flag; sscanf(s, "%[^,]%n", token, &n) == 1; s += n + 1) {
        if (strcmp("DUP", token) == 0) read->is_dup = 1;
        else if (strcmp("REVERSE", token) == 0) read->is_reverse = 1;
        else if (strcmp("SECONDARY", token) == 0 || strcmp("SUPPLEMENTARY", token) == 0) read->is_secondary = 1;
        else if (strcmp("READ2", token) == 0) read->is_read2 = 1;
        if (*(s + n) != ',') break;
    }
    if ((nodup && read->is_dup) || (pao && read->is_secondary)) {
        read_destroy(read);
        return NULL;
    }

    int start_align = 0;
    int s_offset = 0; // offset for softclip at start
    int e_offset = 0; // offset for softclip at end

    u_int32_t *cigar = bam_get_cigar(aln);
    read->n_cigar = aln->core.n_cigar;
    read->cigar_oplen = malloc(read->n_cigar * sizeof (read->cigar_oplen));
    read->cigar_opchr = malloc((read->n_cigar + 1) * sizeof (read->cigar_opchr));
    read->splice_pos = malloc(read->n_cigar * sizeof (read->splice_pos));
    read->splice_offset = malloc(read->n_cigar * sizeof (read->splice_offset));

    j = 0;
    int splice_pos = 0; // track splice position in reads
    for (i = 0; i < read->n_cigar; i++) {
        read->cigar_oplen[i] = bam_cigar_oplen(cigar[i]);
        read->cigar_opchr[i] = bam_cigar_opchr(cigar[i]);
        read->splice_pos[i] = 0;
        read->splice_offset[i] = 0;

        if (read->cigar_opchr[i] == 'M' || read->cigar_opchr[i] == '=' || read->cigar_opchr[i] == 'X') start_align = 1;
        else if (start_align == 0 && read->cigar_opchr[i] == 'S') s_offset = read->cigar_oplen[i];
        else if (start_align == 1 && read->cigar_opchr[i] == 'S') e_offset = read->cigar_oplen[i];

        if (splice && read->cigar_opchr[i] == 'N') {
            read->splice_pos[j] = (isc) ? splice_pos - s_offset : splice_pos;
            read->splice_offset[j] = read->cigar_oplen[i];
            j++;
        }
        else if (splice && read->cigar_opchr[i] != 'D') {
            splice_pos += read->cigar_oplen[i];
        }

        if (read->cigar_opchr[i] != 'I') read->end += read->cigar_oplen[i];
    }
    read->cigar_opchr[read->n_cigar] = '\0';
    read->inferred_length = bam_cigar2qlen(read->n_cigar, cigar);
    read->n_splice = j;

    if (!isc) {
        read->pos -= s_offset; // compensate for soft clip in mapped position
        s_offset = 0;
        e_offset = 0;
    }
    else {
        read->end -= e_offset; // compensate for soft clip in mapped position
    }
    read->length = aln->core.l_qseq - (s_offset + e_offset);
    read->qseq = malloc((read->length + 1) * sizeof (read->qseq));
    read->qual = malloc(read->length  * sizeof (read->qual));
    uint8_t *qual = bam_get_qual(aln);
    for (i = 0; i < read->length; i++) {
        read->qseq[i] = toupper(seq_nt16_str[bam_seqi(bam_get_seq(aln), i + s_offset)]); // get nucleotide id and convert into IUPAC id.
        if (const_qual > 0) read->qual[i] = const_qual;
        else read->qual[i] = (phred64) ? qual[i] - 31 : qual[i]; // account for phred64
    }
    read->qseq[read->length] = '\0';

    read->multimapXA = NULL;
    if (bam_aux_get(aln, "XA")) read->multimapXA = strdup(bam_aux2Z(bam_aux_get(aln, "XA")));

    read->multimapNH = 1;
    if (bam_aux_get(aln, "NH")) read->multimapNH = bam_aux2i(bam_aux_get(aln, "NH"));
    return read;
}
