#include "../thirdparty/munit.h"

extern MunitTest GrayscaleTests[] ;

static MunitSuite test_suites[] = {
    { (char*) "GrayscaleTests", GrayscaleTests, NULL, 1, MUNIT_SUITE_OPTION_NONE },
    { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
};

int main(int argc, char* argv[]) {
    return munit_suite_main(&test_suites[0], NULL, argc, argv);
}
