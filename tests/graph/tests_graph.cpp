#define CATCH_CONFIG_MAIN
#include "../catch/catch.hpp"
#include "../catch/catch_interval.hpp"

#include "tests_graph.hpp"
#include "graph.h"
#include <iostream>

using namespace Catch;
using namespace Detail;

using namespace invariant;
using namespace std;

TEST_CASE("Equality", "[graph]")
{
  SECTION("Test Graph1")
  {
    Graph g1 = graphTest1();
    Graph g2 = graphTest1();
    REQUIRE((g1 == g1));
    REQUIRE((g1 == g2));
  }

  SECTION("Test Graph2")
  {
      Graph g1 = graphTest2();
      Graph g2 = graphTest2();
      REQUIRE(g1 == g1);
      REQUIRE(g1 == g2);
  }
}

TEST_CASE("Serialization", "[graph]"){
    SECTION("Test Graph1")
    {
      Graph g1 = graphTest1();
      string file_name = "test";
      ofstream binFileWrite(file_name.c_str(), ios::out | ios::binary);
      g1.serialize(binFileWrite);
      binFileWrite.close();

      ifstream binFileRead(file_name.c_str(), ios::in | ios::binary);
      Graph g2;
      g2.deserialize(binFileRead);
      binFileRead.close();

      REQUIRE(g1 == g2);
    }
}
