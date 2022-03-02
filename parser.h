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

#ifndef HG_PARSER_H
#define HG_PARSER_H

/* Opaque */

typedef struct HG_PARSER_S *HG_PARSER;
typedef void               *HG_VALUE;
typedef struct  HG_ERROR_S *HG_ERROR;

/* Parsing */

typedef struct HG_RESULT_S { HG_VALUE v; HG_ERROR e; } HG_RESULT;

int HG_parse(const char *s, HG_PARSER p, HG_RESULT *r);
int HG_fparse(const char *n, FILE *f, HG_PARSER p, HG_RESULT *r);

/* Parsers */

HG_PARSER HG_any();
HG_PARSER HG_alpha();
HG_PARSER HG_digit();
HG_PARSER HG_char(char c);
HG_PARSER HG_string(const char *s);

/* Errors */

void HG_eprint(HG_ERROR e);
void HG_efree(HG_ERROR e);

#endif
