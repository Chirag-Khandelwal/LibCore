#include "Args.hpp"

#include <catch2/catch_all.hpp>

using namespace core;

TEST_CASE("Help", "Args")
{
	Array<StringRef, 2> args = {"mainProgram", "--help"};
	args::ArgParser parser(args);
	auto res = parser.parse();

	Array<StringRef, 2> args2 = {"mainProgram", "-h"};
	args::ArgParser parser2(args2);
	auto res2 = parser2.parse();

	REQUIRE(parser.getArgDefs().size() == 1);
	REQUIRE(parser.getArg("help") != nullptr);
	REQUIRE(parser.getArg("help")->getOpts().size() == 2);
	REQUIRE(res.getMsg() == "");
	REQUIRE(res.getCode());
	REQUIRE(parser.getPassthrough().empty());
	REQUIRE(parser.has("help"));

	REQUIRE(parser2.getArgDefs().size() == 1);
	REQUIRE(parser2.getArg("help") != nullptr);
	REQUIRE(parser2.getArg("help")->getOpts().size() == 2);
	REQUIRE(res2.getMsg() == "");
	REQUIRE(res2.getCode());
	REQUIRE(parser2.getPassthrough().empty());
	REQUIRE(parser2.has("help"));
}

TEST_CASE("Version", "Args")
{
	Array<StringRef, 2> args = {"mainProgram", "--version"};
	args::ArgParser parser(args);
	parser.addArg("version").addOpts("--version", "-v").setHelp("Show program version");
	auto res = parser.parse();

	REQUIRE(parser.getArgDefs().size() == 2);
	REQUIRE(parser.getArg("version") != nullptr);
	REQUIRE(parser.getArg("version")->getOpts().size() == 2);
	REQUIRE(res.getMsg() == "");
	REQUIRE(res.getCode());
	REQUIRE(parser.getPassthrough().empty());
	REQUIRE(parser.has("version"));
}

TEST_CASE("GivenArgs8", "Args")
{
	Array<StringRef, 8> args = {"mainProgram", "--a1", "a",	   "--b1",
				    "prog",	   "--",   "--o1", "value"};
	args::ArgParser parser(args);
	parser.addArg("a1").addOpts("--a1").setValReqd(true).setHelp("option 1");
	parser.addArg("b1").addOpts("--b1").setHelp("option 1");
	parser.addArg("src").setHelp("src positional arg");
	auto res		 = parser.parse();
	Span<StringRef> passThru = parser.getPassthrough();

	REQUIRE(parser.getArgDefs().size() == 4); // 3 added + 1 help
	REQUIRE(parser.getArg("a1") != nullptr);
	REQUIRE(parser.getArg("a1")->getOpts().size() == 1);
	REQUIRE(res.getMsg() == "");
	REQUIRE(res.getCode());
	REQUIRE(parser.getValue("a1") == "a");
	REQUIRE(parser.has("b1"));
	REQUIRE(parser.getValue("src") == "prog");
	REQUIRE(passThru.size() == 2);
	REQUIRE(passThru[0] == "--o1");
	REQUIRE(passThru[1] == "value");
}