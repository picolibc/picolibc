/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright © 2021 Nordic Semiconductor
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <getopt.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define assert_equal(a, b, msg)                                              \
    do {                                                                     \
        if ((a) != (b)) {                                                    \
            printf("%s:%d %s != %s: %s\n", __FILE__, __LINE__, #a, #b, msg); \
            err = 1;                                                         \
        }                                                                    \
    } while (0)

#define assert_not_equal(a, b, msg)                                          \
    do {                                                                     \
        if ((a) == (b)) {                                                    \
            printf("%s:%d %s == %s: %s\n", __FILE__, __LINE__, #a, #b, msg); \
            err = 1;                                                         \
        }                                                                    \
    } while (0)

enum getopt_long_idx {
    GETOPT_LONG_IDX_CMD_NAME,
    GETOPT_LONG_IDX_VERBOSE,
    GETOPT_LONG_IDX_OPT,
    GETOPT_LONG_IDX_OPTARG
};

static int
test_getopt_basic(void)
{
    int                       err = 0;
    static const char * const nargv[]
        = { "cmd_name", "-b", "-a", "-h", "-c", "-l", "-h", "-a", "-i", "-w", NULL };
    static const char *accepted_opt = "abchw";
    static const char *expected = "bahc?ha?w";
    size_t             argc = ARRAY_SIZE(nargv) - 1;
    size_t             cnt = 0;
    int                c;
    char             **argv;

    argv = (char **)nargv;
    optind = 0;

    do {
        c = getopt(argc, argv, accepted_opt);
        if (cnt >= strlen(expected)) {
            break;
        }

        assert_equal(c, expected[cnt++], "unexpected opt character");
    } while (c != -1);

    c = getopt(argc, argv, accepted_opt);
    assert_equal(c, -1, "unexpected opt character");
    return err;
}

enum getopt_idx { GETOPT_IDX_CMD_NAME, GETOPT_IDX_OPTION1, GETOPT_IDX_OPTION2, GETOPT_IDX_OPTARG };

static int
test_getopt(void)
{
    int                       err = 0;
    static const char        *test_opts = "ac:";
    static const char * const nargv[] = {
        [GETOPT_IDX_CMD_NAME] = "cmd_name",
        [GETOPT_IDX_OPTION1] = "-a",
        [GETOPT_IDX_OPTION2] = "-c",
        [GETOPT_IDX_OPTARG] = "foo",
        NULL,
    };
    int    argc = ARRAY_SIZE(nargv) - 1;
    char **argv;
    int    c;

    argv = (char **)nargv;

    /* Test uknown option */
    optind = 0;
    c = getopt(argc, argv, test_opts);
    assert_equal(c, 'a', "unexpected opt character");
    c = getopt(argc, argv, test_opts);
    assert_equal(c, 'c', "unexpected opt character");
    assert_equal(0, strcmp(argv[GETOPT_IDX_OPTARG], optarg), "unexpected optarg result");
    return err;
}

static int
test_getopt_long(void)
{
    int           err = 0;
    /* Below test is based on example
     * https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
     */
    int           verbose_flag = 0;
    /* getopt_long stores the option index here. */
    int           option_index = 0;
    char        **argv;
    int           c;
    struct option long_options[] = {
        /* These options set a flag. */
        { "verbose", no_argument,       &verbose_flag, 1   },
        { "brief",   no_argument,       &verbose_flag, 0   },
        /* These options don’t set a flag.
         * We distinguish them by their indices.
         */
        { "add",     no_argument,       0,             'a' },
        { "create",  required_argument, 0,             'c' },
        { "delete",  required_argument, 0,             'd' },
        { "long",    required_argument, 0,             'e' },
        { 0,         0,                 0,             0   },
    };
    static const char        *accepted_opt = "ac:d:e:";

    static const char * const argv1[] = {
        [GETOPT_LONG_IDX_CMD_NAME] = "cmd_name",
        [GETOPT_LONG_IDX_VERBOSE] = "--verbose",
        [GETOPT_LONG_IDX_OPT] = "--create",
        [GETOPT_LONG_IDX_OPTARG] = "some_file",
        NULL,
    };
    int                       argc1 = ARRAY_SIZE(argv1) - 1;

    static const char * const argv2[] = {
        [GETOPT_LONG_IDX_CMD_NAME] = "cmd_name",
        [GETOPT_LONG_IDX_VERBOSE] = "--brief",
        [GETOPT_LONG_IDX_OPT] = "-d",
        [GETOPT_LONG_IDX_OPTARG] = "other_file",
        NULL,
    };
    int                       argc2 = ARRAY_SIZE(argv2) - 1;

    static const char * const argv3[] = {
        [GETOPT_LONG_IDX_CMD_NAME] = "cmd_name",
        [GETOPT_LONG_IDX_VERBOSE] = "--brief",
        [GETOPT_LONG_IDX_OPT] = "-a",
        NULL,
    };
    int                       argc3 = ARRAY_SIZE(argv3) - 1;

    /* this test distinguish getopt_long and getopt_long_only functions */
    static const char * const argv4[] = {
        [GETOPT_LONG_IDX_CMD_NAME] = "cmd_name",
        [GETOPT_LONG_IDX_VERBOSE] = "--brief",
        /* below should not be interpreted as "--long/-e" option */
        [GETOPT_LONG_IDX_OPT] = "-l",
        [GETOPT_LONG_IDX_OPTARG] = "long_argument",
        NULL,
    };
    int argc4 = ARRAY_SIZE(argv4) - 1;

    /* Test scenario 1 */
    optind = 0;
    argv = (char **)argv1;
    c = getopt_long(argc1, argv, accepted_opt, long_options, &option_index);
    assert_equal(verbose_flag, 1, "verbose flag expected");
    c = getopt_long(argc1, argv, accepted_opt, long_options, &option_index);
    assert_equal('c', c, "unexpected option");
    assert_equal(0, strcmp(optarg, argv[GETOPT_LONG_IDX_OPTARG]), "unexpected optarg");
    c = getopt_long(argc1, argv, accepted_opt, long_options, &option_index);
    assert_equal(-1, c, "getopt_long shall return -1");

    /* Test scenario 2 */
    argv = (char **)argv2;
    optind = 0;
    c = getopt_long(argc2, argv, accepted_opt, long_options, &option_index);
    assert_equal(verbose_flag, 0, "verbose flag expected");
    c = getopt_long(argc2, argv, accepted_opt, long_options, &option_index);
    assert_equal('d', c, "unexpected option");
    assert_equal(0, strcmp(optarg, argv[GETOPT_LONG_IDX_OPTARG]), "unexpected optarg");
    c = getopt_long(argc2, argv, accepted_opt, long_options, &option_index);
    assert_equal(-1, c, "getopt_long shall return -1");

    /* Test scenario 3 */
    argv = (char **)argv3;
    optind = 0;
    c = getopt_long(argc3, argv, accepted_opt, long_options, &option_index);
    assert_equal(verbose_flag, 0, "verbose flag expected");
    c = getopt_long(argc3, argv, accepted_opt, long_options, &option_index);
    assert_equal('a', c, "unexpected option");
    c = getopt_long(argc3, argv, accepted_opt, long_options, &option_index);
    assert_equal(-1, c, "getopt_long shall return -1");

    /* Test scenario 4 */
    argv = (char **)argv4;
    optind = 0;
    c = getopt_long(argc4, argv, accepted_opt, long_options, &option_index);
    assert_equal(verbose_flag, 0, "verbose flag expected");
    c = getopt_long(argc4, argv, accepted_opt, long_options, &option_index);
    /* Function was called with option '-l'. It is expected it will be
     * NOT evaluated to '--long' which has flag 'e'.
     */
    assert_not_equal('e', c, "unexpected option match");
    c = getopt_long(argc4, argv, accepted_opt, long_options, &option_index);
    return err;
}

static int
test_getopt_long_only(void)
{
    int           err = 0;
    /* Below test is based on example
     * https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
     */
    int           verbose_flag = 0;
    /* getopt_long stores the option index here. */
    int           option_index = 0;
    char        **argv;
    int           c;
    struct option long_options[] = {
        /* These options set a flag. */
        { "verbose", no_argument,       &verbose_flag, 1   },
        { "brief",   no_argument,       &verbose_flag, 0   },
        /* These options don’t set a flag.
         * We distinguish them by their indices.
         */
        { "add",     no_argument,       0,             'a' },
        { "create",  required_argument, 0,             'c' },
        { "delete",  required_argument, 0,             'd' },
        { "long",    required_argument, 0,             'e' },
        { 0,         0,                 0,             0   },
    };
    static const char        *accepted_opt = "ac:d:e:";

    static const char * const argv1[] = {
        [GETOPT_LONG_IDX_CMD_NAME] = "cmd_name",
        [GETOPT_LONG_IDX_VERBOSE] = "--verbose",
        [GETOPT_LONG_IDX_OPT] = "--create",
        [GETOPT_LONG_IDX_OPTARG] = "some_file",
        NULL,
    };
    int                       argc1 = ARRAY_SIZE(argv1) - 1;

    static const char * const argv2[] = {
        [GETOPT_LONG_IDX_CMD_NAME] = "cmd_name",
        [GETOPT_LONG_IDX_VERBOSE] = "--brief",
        [GETOPT_LONG_IDX_OPT] = "-d",
        [GETOPT_LONG_IDX_OPTARG] = "other_file",
        NULL,
    };
    int                       argc2 = ARRAY_SIZE(argv2) - 1;

    static const char * const argv3[] = {
        [GETOPT_LONG_IDX_CMD_NAME] = "cmd_name",
        [GETOPT_LONG_IDX_VERBOSE] = "--brief",
        [GETOPT_LONG_IDX_OPT] = "-a",
        NULL,
    };
    int                       argc3 = ARRAY_SIZE(argv3) - 1;

    /* this test distinguish getopt_long and getopt_long_only functions */
    static const char * const argv4[] = {
        [GETOPT_LONG_IDX_CMD_NAME] = "cmd_name",
        [GETOPT_LONG_IDX_VERBOSE] = "--brief",
        /* below should be interpreted as "--long/-e" option */
        [GETOPT_LONG_IDX_OPT] = "-l",
        [GETOPT_LONG_IDX_OPTARG] = "long_argument",
        NULL,
    };
    int argc4 = ARRAY_SIZE(argv4) - 1;

    /* Test scenario 1 */
    argv = (char **)argv1;
    optind = 0;
    c = getopt_long_only(argc1, argv, accepted_opt, long_options, &option_index);
    assert_equal(verbose_flag, 1, "verbose flag expected");
    c = getopt_long_only(argc1, argv, accepted_opt, long_options, &option_index);
    assert_equal('c', c, "unexpected option");
    assert_equal(0, strcmp(optarg, argv[GETOPT_LONG_IDX_OPTARG]), "unexpected optarg");
    c = getopt_long_only(argc1, argv, accepted_opt, long_options, &option_index);
    assert_equal(-1, c, "getopt_long_only shall return -1");

    /* Test scenario 2 */
    argv = (char **)argv2;
    optind = 0;
    c = getopt_long_only(argc2, argv, accepted_opt, long_options, &option_index);
    assert_equal(verbose_flag, 0, "verbose flag expected");
    c = getopt_long_only(argc2, argv, accepted_opt, long_options, &option_index);
    assert_equal('d', c, "unexpected option");
    assert_equal(0, strcmp(optarg, argv[GETOPT_LONG_IDX_OPTARG]), "unexpected optarg");
    c = getopt_long_only(argc2, argv, accepted_opt, long_options, &option_index);
    assert_equal(-1, c, "getopt_long_only shall return -1");

    /* Test scenario 3 */
    argv = (char **)argv3;
    optind = 0;
    c = getopt_long_only(argc3, argv, accepted_opt, long_options, &option_index);
    assert_equal(verbose_flag, 0, "verbose flag expected");
    c = getopt_long_only(argc3, argv, accepted_opt, long_options, &option_index);
    assert_equal('a', c, "unexpected option");
    c = getopt_long_only(argc3, argv, accepted_opt, long_options, &option_index);
    assert_equal(-1, c, "getopt_long_only shall return -1");

    /* Test scenario 4 */
    argv = (char **)argv4;
    optind = 0;
    c = getopt_long_only(argc4, argv, accepted_opt, long_options, &option_index);
    assert_equal(verbose_flag, 0, "verbose flag expected");
    c = getopt_long_only(argc4, argv, accepted_opt, long_options, &option_index);

    /* Function was called with option '-l'. It is expected it will be
     * evaluated to '--long' which has flag 'e'.
     */
    assert_equal('e', c, "unexpected option");
    c = getopt_long_only(argc4, argv, accepted_opt, long_options, &option_index);
    return err;
}

int
main(void)
{
    int ret = 0;
    ret |= test_getopt_basic();
    ret |= test_getopt();
    ret |= test_getopt_long();
    ret |= test_getopt_long_only();
    return ret;
}
