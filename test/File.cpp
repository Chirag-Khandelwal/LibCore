#include "File.hpp"

#include <catch2/catch_all.hpp>

using namespace core;

TEST_CASE("Basic", "File")
{
	fs::File f("<test>", true);
	REQUIRE(f.emptyData());
	REQUIRE(f.isVirtual());

	f.append("line 1\n");
	f.append("\n"); // line 2
	f.append("line 3\n");

	REQUIRE(f.sizeAppendLocs() == 3);
	REQUIRE(f.getLastAppendData() == "line 3\n");
	REQUIRE(f.getAppendData(1) == "\n");

	std::ostringstream oss;
	utils::warn(&f, oss, 1, 5, "test warning");
	REQUIRE(oss.str() ==
		"In file: <test>\n1 | line 1\n     ~~~~~\n     ^\n    Warning: test warning\n");
	oss.str("");
	utils::fail(&f, oss, 8, 10, "test warning 2");
	REQUIRE(oss.str() ==
		"In file: <test>\n3 | line 3\n    ~~~\n    ^\n    Error: test warning 2\n");
}