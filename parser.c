/**
 *
 * Copyright (C) 2022 Jared B. Resch
 *
 * This file is part of MERCURY.
 * 
 * MERCURY is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * MERCURY is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with MERCURY. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define HG_NL             '\n'
#define HG_STOF_MAX_LEN   10
#define HG_ANY_ERR_EOF    "expected character encountered end of input"
#define HG_CHAR_ERR       "expected '%c' encountered '%c'"
#define HG_CHAR_ERR_EOF   "expected '%c' encountered end of input"
#define HG_STRING_ERR     "expected \"%s\" encountered '%c'"
#define HG_STRING_ERR_EOF "expected \"%s\" encountered end of input"
#define HG_CHAR_C(p)      (p->d.c.c)
#define HG_STRING_S(p)    (p->d.s.s)
#define HG_STRING_C(p, i) (p->d.s.s[i])
#define HG_STRING_L(p)    (p->d.s.l)
#define HG_OR_X(p)        (p->d.or.x)
#define HG_OR_Y(p)        (p->d.or.y)
#define HG_AND_X(p)       (p->d.and.x)
#define HG_AND_Y(p)       (p->d.and.y)
#define HG_ANY_LEN(p)     (p->d.any.len)
#define HG_ANY_XYZ(p)     (p->d.any.xyz)
#define HG_ANY_X(p, i)    (p->d.any.xyz[i])
#define HG_MANY_LEN(p)    (p->d.many.len)
#define HG_MANY_XYZ(p)    (p->d.many.xyz)
#define HG_MANY_X(p, i)   (p->d.many.xyz[i])
#define HG_IN_J(i)        (i->s.j)
#define HG_IN_READ(i)     (i->s.j < i->l ? i->b[i->s.j] : EOF)
#define HG_IN_ADV(i)      ((HG_IN_READ(i) != HG_NL) ? i->s.j++ && i->s.c++ : \
                                i->s.j++ && i->s.r++ && (i->s.c = i->s.t = 1))
#define HG_IN_TERM(i, j)  (i->s.t += j)
#define HG_IN_ROW(i)      (i->s.r)
#define HG_IN_COL(i)      (i->s.c)
#define HG_IN_SAVE(i)     (i->p = i->s)
#define HG_IN_RES(i)      (i->s = i->p)
#define HG_IN_STATE(i)    (i->s)
#define HG_STATE_INIT(s)  (s.j = 0, s.r = s.c = s.t = 1)

enum HG_TYPES {
        HG_ANY,
        HG_ALPHA,
        HG_DIGIT,
        HG_CHAR,
        HG_STRING
};

struct HG_PARSER_S {
        int t, r;
        union {
                struct { char c; } c;
                struct { int l; char *s; } s;
        } d;
};

typedef struct HG_STATE_S { int j, r, c, t; } HG_STATE;

struct HG_ERROR_S { char *f, *m; HG_STATE s; };

typedef struct HG_INPUT_S {
        int l;
        char *b, *f;
        HG_STATE s, p;
} *HG_INPUT;

static int HG_pany(HG_PARSER p, HG_INPUT i, HG_RESULT *r);
static int HG_palpha(HG_PARSER p, HG_INPUT i, HG_RESULT *r);
static int HG_pdigit(HG_PARSER p, HG_INPUT i, HG_RESULT *r);
static int HG_pchar(HG_PARSER p, HG_INPUT i, HG_RESULT *r);
static int HG_pstring(HG_PARSER p, HG_INPUT i, HG_RESULT *r);

static char *HG_strcpy(const char *s, int *l)
{
        int m;
        char *c;
        m = strlen(s);
        c = malloc(m + 1);
        if (l) *l = m;
        strcpy(c, s);
        return c;
}

static char *HG_stof(const char *s)
{
        char *r;
        int i, l, d;
        l = strlen(s);
        d = l > HG_STOF_MAX_LEN;
        l = d ? HG_STOF_MAX_LEN : l;
        r = malloc(d ? l + 6 : l + 3);
        r[0] = '"';
        for (i = 0; i <= l; i++) r[i + 1] = s[i];
        if (d) {
                r[i++] = '.', r[i++] = '.', r[i++] = '.';
                r[i++] = '"', r[i] = '\0';
        } else {
                r[i++] = '"', r[i] = '\0';
        }
        return r;
}

static char *HG_ctoa(char c)
{
        char *s;
        s = malloc(2);
        s[0] = c, s[1] = '\0';
        return s;
}

static char *HG_fmtv(const char *f, va_list v)
{
        int l;
        char *s, b[4096];
        l = vsprintf(b, f, v);
        if (l < 0) return NULL;
        s = malloc(l + 1);
        strcpy(s, b);
        return s;
}

static HG_INPUT HG_isalloc(const char *s)
{
        HG_INPUT i;
        i = malloc(sizeof(*i));
        if (!i) return NULL;
        i->b = HG_strcpy(s, &i->l);
        if (!i->b) {
                free(i);
                return NULL;
        }
        i->f = HG_stof(s);
        if (!i->f) {
                free(i);
                free(i->b);
                return NULL;
        }
        HG_STATE_INIT(i->s);
        return i;
}

static HG_INPUT HG_ifalloc(const char *n, FILE *f)
{
        HG_INPUT i;
        i = malloc(sizeof(*i));
        if (!i) return NULL;
        i->f = HG_strcpy(n, NULL);
        /* TODO */
        return i;
}

static void HG_in_free(HG_INPUT i)
{
        free(i->f);
        free(i->b);
        free(i);
}

static HG_ERROR HG_ealloc(HG_INPUT i, const char *f, ...)
{
        HG_ERROR e;
        va_list v;
        e = malloc(sizeof(*e));
        e->f = HG_strcpy(i->f, NULL);
        if (!e->f) return NULL;
        va_start(v, f);
        e->m = HG_fmtv(f, v);
        va_end(v);
        if (!e->m) return NULL;
        e->s = HG_IN_STATE(i);
        return e;
}

void HG_eprint(HG_ERROR e)
{
        fprintf(stderr, "\033[1m%s%s%d:%d-%d:\033[0m %s\n",
                e->f ? "" : e->f, e->f ? "" : ":",
                e->s.r, e->s.c, e->s.t, e->m);
}

void HG_efree(HG_ERROR e)
{
        free(e->f);
        free(e->m);
        free(e);
}

static int HG_parze(HG_PARSER p, HG_INPUT i, HG_RESULT *r)
{
        switch (p->t) {
                case    HG_ANY: return    HG_pany(p, i, r);
                case  HG_ALPHA: return  HG_palpha(p, i, r);
                case  HG_DIGIT: return  HG_pdigit(p, i, r);
                case   HG_CHAR: return   HG_pchar(p, i, r);
                case HG_STRING: return HG_pstring(p, i, r);
        }
        return 0;
}

int HG_parse(const char *s, HG_PARSER p, HG_RESULT *r)
{
        int z;
        HG_INPUT i;
        i = HG_isalloc(s);
        z = HG_parze(p, i, r);
        HG_in_free(i);
        return z;
}

int HG_fparse(const char *n, FILE *f, HG_PARSER p, HG_RESULT *r)
{
        int z;
        HG_INPUT i;
        i = HG_ifalloc(n, f);
        z = HG_parze(p, i, r);
        HG_in_free(i);
        return z;
}

static HG_PARSER HG_palloc(int t)
{
        HG_PARSER p;
        p = malloc(sizeof(*p));
        if (!p) return NULL;
        p->t = t;
        p->r = 1;
        return p;
}

HG_PARSER HG_any()
{
        return HG_palloc(HG_ANY);
}

HG_PARSER HG_char(char c)
{
        HG_PARSER p;
        p = HG_palloc(HG_CHAR);
        if (!p) return NULL;
        HG_CHAR_C(p) = c;
        return p;
}

HG_PARSER HG_string(const char *s)
{
        HG_PARSER p;
        p = HG_palloc(HG_STRING);
        if (!p) return NULL;
        HG_STRING_L(p) = strlen(s);
        HG_STRING_S(p) = malloc(HG_STRING_L(p) + 1);
        strcpy(HG_STRING_S(p), s);
        return p;
}

int HG_pany(HG_PARSER p, HG_INPUT i, HG_RESULT *r)
{
        char c;
        c = HG_IN_READ(i);
        if (c != EOF) {
                r->v = HG_ctoa(c);
                HG_IN_ADV(i);
                HG_IN_TERM(i, 1);
                return 1;
        }
        r->e = HG_ealloc(i, HG_ANY_ERR_EOF);
        return 0;
}

int HG_palpha(HG_PARSER p, HG_INPUT i, HG_RESULT *r)
{
        return 1;
}

int HG_pdigit(HG_PARSER p, HG_INPUT i, HG_RESULT *r)
{
        return 1;
}

int HG_pchar(HG_PARSER p, HG_INPUT i, HG_RESULT *r)
{
        char c;
        c = HG_IN_READ(i);
        if (c != EOF && c == HG_CHAR_C(p)) {
                r->v = HG_ctoa(c);
                HG_IN_ADV(i);
                HG_IN_TERM(i, 1);
                return 1;
        } else if (c == EOF) {
                r->e = HG_ealloc(i, HG_CHAR_ERR_EOF, HG_CHAR_C(p));
        } else {
                HG_IN_TERM(i, 1);
                r->e = HG_ealloc(i, HG_CHAR_ERR, HG_CHAR_C(p), c);
        }
        return 0;
}

int HG_pstring(HG_PARSER p, HG_INPUT i, HG_RESULT *r)
{
        return 1;
}
