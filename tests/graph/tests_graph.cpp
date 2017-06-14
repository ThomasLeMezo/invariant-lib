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
  SECTION("Test Graph 2D")
  {
    Graph g1 = graphTest_2D();
    Graph g2 = graphTest_2D();
    REQUIRE(g1.is_equal(g1));
    REQUIRE(g1.is_equal(g2));
  }

  SECTION("Test Graph 5D")
  {
      Graph g1 = graphTest_5D();
      Graph g2 = graphTest_5D();
      REQUIRE(g1.is_equal(g1));
      REQUIRE(g1.is_equal(g2));
  }
}

TEST_CASE("Serialization", "[graph]"){
    SECTION("Test Graph 2D")
    {
      Graph g1 = graphTest_2D();
      string file_name = "test";
      ofstream binFileWrite(file_name.c_str(), ios::out | ios::binary);
      g1.serialize(binFileWrite);
      binFileWrite.close();

      ifstream binFileRead(file_name.c_str(), ios::in | ios::binary);
      Graph g2;
      g2.deserialize(binFileRead);
      binFileRead.close();

      REQUIRE(g1.is_equal(g2));
    }
}

TEST_CASE("Bisection", "[graph]"){
    SECTION("Test number of paves while bisect Graph 2D norm"){
        Graph g1 = graphTest_2D_Norm();
        for(int i=0; i<10; i++){
            g1.bisect();
            REQUIRE(g1.size()==pow(2, i+1));
        }
    }

    SECTION("Test number of paves while bisect Graph 5D norm"){
        Graph g1 = graphTest_5D_Norm();
        for(int i=0; i<10; i++){
            g1.bisect();
            REQUIRE(g1.size()==pow(2, i+1));
        }
    }
}
