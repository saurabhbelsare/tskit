#include "testlib.h"
#include "tsk_convert.h"

#include <unistd.h>
#include <stdlib.h>

static void
test_single_tree_newick(void)
{
    int ret;
    tsk_treeseq_t ts;
    tsk_tree_t t;
    size_t buffer_size = 1024;
    char newick[buffer_size];

    tsk_treeseq_from_text(&ts, 1, single_tree_ex_nodes, single_tree_ex_edges,
            NULL, NULL, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(tsk_treeseq_get_num_samples(&ts), 4);
    CU_ASSERT_EQUAL(tsk_treeseq_get_num_trees(&ts), 1);

    ret = tsk_tree_alloc(&t, &ts, 0);
    CU_ASSERT_EQUAL_FATAL(ret, 0)
    ret = tsk_tree_first(&t);
    CU_ASSERT_EQUAL_FATAL(ret, 1)

    ret = tsk_convert_newick(&t, -1, 1, 0, buffer_size, newick);
    CU_ASSERT_EQUAL_FATAL(ret, TSK_ERR_NODE_OUT_OF_BOUNDS);
    ret = tsk_convert_newick(&t, 7, 1, 0, buffer_size, newick);
    CU_ASSERT_EQUAL_FATAL(ret, TSK_ERR_NODE_OUT_OF_BOUNDS);

    ret = tsk_convert_newick(&t, 0, 0, 0, buffer_size, newick);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    /* Seems odd, but this is what a single node newick tree looks like.
     * Newick parsers seems to accept it in any case */
    CU_ASSERT_STRING_EQUAL(newick, "1;");

    ret = tsk_convert_newick(&t, 4, 0, 0, buffer_size, newick);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    CU_ASSERT_STRING_EQUAL(newick, "(1:1,2:1);");

    ret = tsk_convert_newick(&t, 6, 0, 0, buffer_size, newick);
    CU_ASSERT_EQUAL_FATAL(ret, 0);
    CU_ASSERT_STRING_EQUAL(newick, "((1:1,2:1):2,(3:2,4:2):1);");

    tsk_tree_free(&t);
    tsk_treeseq_free(&ts);
}

static void
verify_vcf_converter(tsk_treeseq_t *ts, unsigned int ploidy)
{
    int ret;
    char *str = NULL;
    tsk_vcf_converter_t vc;
    unsigned int num_variants;

    ret = tsk_vcf_converter_alloc(&vc, ts, ploidy, "chr1234");
    CU_ASSERT_FATAL(ret ==  0);
    tsk_vcf_converter_print_state(&vc, _devnull);
    ret = tsk_vcf_converter_get_header(&vc, &str);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_NSTRING_EQUAL("##", str, 2);
    num_variants = 0;
    while ((ret = tsk_vcf_converter_next(&vc, &str)) == 1) {
        CU_ASSERT_NSTRING_EQUAL("chr1234\t", str, 2);
        num_variants++;
    }
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_EQUAL(num_variants, tsk_treeseq_get_num_sites(ts));
    tsk_vcf_converter_free(&vc);
}

static void
test_single_tree_vcf(void)
{
    int ret;
    unsigned int ploidy;
    tsk_vcf_converter_t vc;
    tsk_treeseq_t ts;

    tsk_treeseq_from_text(&ts, 1, single_tree_ex_nodes, single_tree_ex_edges,
            NULL, single_tree_ex_sites, single_tree_ex_mutations, NULL, NULL);

    ret = tsk_vcf_converter_alloc(&vc, &ts, 0, "1");
    CU_ASSERT_EQUAL(ret, TSK_ERR_BAD_PARAM_VALUE);
    ret = tsk_vcf_converter_alloc(&vc, &ts, 3, "1");
    CU_ASSERT_EQUAL(ret, TSK_ERR_BAD_PARAM_VALUE);
    ret = tsk_vcf_converter_alloc(&vc, &ts, 11, "1");
    CU_ASSERT_EQUAL(ret, TSK_ERR_BAD_PARAM_VALUE);

    for (ploidy = 1; ploidy < 3; ploidy++) {
        verify_vcf_converter(&ts, ploidy);
    }

    tsk_treeseq_free(&ts);
}

static void
test_single_tree_vcf_no_mutations(void)
{
    int ret;
    char *str = NULL;
    tsk_vcf_converter_t vc;
    tsk_treeseq_t ts;

    tsk_treeseq_from_text(&ts, 1, single_tree_ex_nodes, single_tree_ex_edges,
            NULL, NULL, NULL, NULL, NULL);
    CU_ASSERT_EQUAL_FATAL(tsk_treeseq_get_num_sites(&ts), 0);

    ret = tsk_vcf_converter_alloc(&vc, &ts, 1, "1");
    CU_ASSERT_FATAL(ret ==  0);
    tsk_vcf_converter_print_state(&vc, _devnull);
    ret = tsk_vcf_converter_get_header(&vc, &str);
    CU_ASSERT_EQUAL(ret, 0);
    CU_ASSERT_NSTRING_EQUAL("##", str, 2);
    ret = tsk_vcf_converter_next(&vc, &str);
    CU_ASSERT_EQUAL(ret, 0);
    tsk_vcf_converter_free(&vc);

    tsk_treeseq_free(&ts);
}

int
main(int argc, char **argv)
{
    CU_TestInfo tests[] = {
        {"test_single_tree_newick", test_single_tree_newick},
        {"test_single_tree_vcf", test_single_tree_vcf},
        {"test_single_tree_vcf_no_mutations", test_single_tree_vcf_no_mutations},
        {NULL, NULL},
    };
    return test_main(tests, argc, argv);
}
