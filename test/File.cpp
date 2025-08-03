#include "File.hpp"

#include <catch2/catch_all.hpp>

using namespace core;

TEST_CASE("Basic", "File")
{
	auto fileRes = fs::File::create("<test>", true);
	REQUIRE(fileRes.isOk());
	fs::File f = fileRes.value();
	REQUIRE(f.emptyData());
	REQUIRE(f.getPath() == "<test>");
	REQUIRE(f.isVirtual());

	f.append("line 1\n");
	f.append("\n"); // line 2
	f.append("line 3\n");

	REQUIRE(f.sizeAppendLocs() == 3);
	REQUIRE(f.getLastAppendData() == "line 3\n");
	REQUIRE(f.getAppendData(1) == "\n");

	std::ostringstream oss;
	f.warn(oss, fs::FileLoc(1, 5), "test warning");
	REQUIRE(oss.str() == "1 | line 1\n     ~~~~~\n     ^\n    Warning: test warning\n");
	oss.str("");
	f.warn(oss, fs::FileLoc(8, 10), "test warning 2");
	REQUIRE(oss.str() == "3 | line 3\n    ~~~\n    ^\n    Warning: test warning 2\n");
}