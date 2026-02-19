#include "File.hpp"

#include <catch2/catch_all.hpp>

using namespace core;

TEST_CASE("File.Basic")
{
    File f("<test>", true);
    REQUIRE(f.emptyData());
    REQUIRE(f.isVirtual());

    f.append("line 1\n");
    f.append("\n"); // line 2
    f.append("line 3\n");

    REQUIRE(f.sizeAppendLocs() == 3);
    REQUIRE(f.getLastAppendData() == "line 3\n");
    REQUIRE(f.getAppendData(1) == "\n");

    std::ostringstream oss;
    utils::output(oss, &f, 0, 3, ""); // "line" (on line 1)
    REQUIRE(oss.str() == "In: <test>\n1 | line 1\n    ~~~~\n    ^\n");
    oss.str("");
    utils::output(oss, &f, 5, 5, "Warning: test warning"); // "1" (on line 1)
    REQUIRE(oss.str() ==
            "In: <test>\n1 | line 1\n         ~\n         ^\n    Warning: test warning\n");
    oss.str("");
    utils::output(oss, &f, 8, 11, "Error: test fail"); // "line" (on line 3)
    REQUIRE(oss.str() == "In: <test>\n3 | line 3\n    ~~~~\n    ^\n    Error: test fail\n");
}