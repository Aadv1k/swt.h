#include "../thirdparty/munit.h"

extern MunitTest GrayscaleTests[];
extern MunitTest CCATests[];

static MunitSuite test_suites[] = {
    //{ (char*) "Grayscale_Tests", GrayscaleTests, NULL, 1, MUNIT_SUITE_OPTION_NONE },
    { (char*) "CCA_Tests", CCATests, NULL, 1, MUNIT_SUITE_OPTION_NONE },
    { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
};

static MunitSuite main_suite = {
    (char*) "",
    NULL,
    test_suites, 
    1,
    MUNIT_SUITE_OPTION_NONE 
};

int main(int argc, char* argv[]) {
    return munit_suite_main(&main_suite, NULL, argc, argv);
}
