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
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

static void HG_any_tests()
{
        HG_RESULT r;
        HG_PARSER any;
        printf("-----------------------------------------------------------\n");
        printf("| HG_any_tests()\n");
        printf("-----------------------------------------------------------\n");
        printf("| any = HG_any();\n");
        any = HG_any();
        printf("| assert(any != NULL);\n");
        assert(any != NULL);
        printf("| PASSED\n");
        printf("-----------------------------------------------------------\n");
        printf("| assert(HG_parse(\"abc\", any, &r));\n");
        assert(HG_parse("abc", any, &r));
        printf("| r.v = \"%s\";\n", (char*)r.v);
        free(r.v);
        printf("| free(r.v);\n");
        printf("| PASSED\n");
        printf("-----------------------------------------------------------\n");
        printf("| assert(HG_parse(\"bc\", any, &r));\n");
        assert(HG_parse("bc", any, &r));
        printf("| r.v = \"%s\";\n", (char*)r.v);
        free(r.v);
        printf("| free(r.v);\n");
        printf("| PASSED\n");
        printf("-----------------------------------------------------------\n");
        printf("| assert(HG_parse(\"c\", any, &r));\n");
        assert(HG_parse("c", any, &r));
        printf("| r.v = \"%s\";\n", (char*)r.v);
        free(r.v);
        printf("| free(r.v);\n");
        printf("| PASSED\n");
        printf("-----------------------------------------------------------\n");
        printf("| assert(!HG_parse(\"\", any, &r));\n");
        assert(!HG_parse("", any, &r));
        printf("| HG_err_print(r.e);\n");
        fprintf(stderr, "| ");
        HG_eprint(r.e);
        printf("| HG_err_free(r.e)\n");
        HG_efree(r.e);
        printf("| PASSED\n");
        printf("-----------------------------------------------------------\n");
}

static void HG_char_tests()
{
        HG_RESULT r;
        HG_PARSER a, b;
        printf("| HG_char_tests()\n");
        printf("-----------------------------------------------------------\n");
        a = HG_char('a'), b = HG_char('b');
        printf("| a = HG_char('a'), b = HG_char('b');\n");
        printf("| assert(HG_parse(\"abc\", a, &r));\n");
        assert(HG_parse("abc", a, &r));
        printf("| free(r.v);\n");
        free(r.v);
        printf("| PASSED\n");
        printf("-----------------------------------------------------------\n");
        printf("| assert(!HG_parse(\"abc\", b, &r));\n");
        assert(!HG_parse("abc", b, &r));
        printf("| HG_err_print(r.e);\n");
        fprintf(stderr, "| ");
        HG_eprint(r.e);
        printf("| HG_err_free(r.e)\n");
        HG_efree(r.e);
        printf("| PASSED\n");
        printf("-----------------------------------------------------------\n");
        printf("| assert(!HG_parse(\"\", a, &r));\n");
        assert(!HG_parse("", a, &r));
        printf("| HG_err_print(r.e);\n");
        fprintf(stderr, "| ");
        HG_eprint(r.e);
        printf("| HG_err_free(r.e)\n");
        HG_efree(r.e);
        printf("| PASSED\n");
        printf("-----------------------------------------------------------\n");
}

int main(void)
{
        HG_any_tests();
        HG_char_tests();
        return EXIT_SUCCESS;
}
