#include "../source/timeSurfaceGenerator.hpp"

#include "catch.hpp"

struct Event1d{ // minimum field require to build 1D TS
  int64_t t;
  int64_t x;
  int64_t p;
};

struct TsEvent1d{
  int64_t t;
  int64_t x;
  int64_t p;
  std::vector<double> context;
};

struct Event2d{ // minimum field require to build 2D TS
  int64_t t;
  int64_t x;
  int64_t y;
  int64_t p;
};

struct TsEvent2d{
  int64_t t;
  int64_t x;
  int64_t y;
  int64_t p;
  std::vector<double> context;
};

TEST_CASE("Compute 1D timeSurfaces from the given events", "[TimeSurface]") {
  auto tsEvent1dFromEvent = [](Event1d ev, std::vector<double> context){
    return TsEvent1d{ev.t, ev.x, ev.p, context};
  };
  auto kernel1d = [](Event1d evRef, Event1d evNeighbor){
    auto diff = static_cast<double>(evRef.t)-static_cast<double>(evNeighbor.t);
    return (diff < 3*100.) ? exp(-(diff)/100.) : 0;
  };
  bool go = false;
  auto handlerTimeSurface1d = [&go](TsEvent1d ev){
    if(go){
      std::vector<double> vec = {0, 0, 0, 0.4493, 0.4966, 0.5488, 0.6065, 0.6703, 0.7408, 0, 0, 0, 0, 0, 0, 0.4066, 1.0000, 0.9048, 0.8187, 0, 0, 0};
      auto cpt = 0;
      auto sum = 0.;
      for(auto&& it: ev.context){
        sum+=((it-vec[cpt])*(it-vec[cpt]));
        cpt++;
      }
      sum/=vec.size();
      REQUIRE(sum < 1e-8);
    }
  };

  auto tsEvent2dFromEvent = [](Event2d ev, std::vector<double> context){
    return TsEvent2d{ev.t, ev.x, ev.y, ev.p, context};
  };
  auto kernel2d = [](Event2d evRef, Event2d evNeighbor){
    auto diff = static_cast<double>(evRef.t)-static_cast<double>(evNeighbor.t);
    return (diff < 3*100.) ? exp(-(diff)/100.) : 0;
  };

  auto handlerTimeSurface2d = [&go](TsEvent2d ev){
    if(go){
      std::vector<double> vec = {0,0,0,0,0, 0.4966,0,0,0,0,
                                 0,0.6703,0.9048,0,0,
                                 0,0,0,0,0,
                                 0,0,0,0,0,
                                 0.4493,0,0,0,0,
                                 0,0,0,0,0,
                                 0,0.5488,1.0000,0,0.8187,
                                 0,0,0.6065,0,0,
                                 0,0,0,0,0};
      auto cpt = 0;
      auto sum = 0.;
      for(auto&& it: ev.context){
        sum+=((it-vec[cpt])*(it-vec[cpt]));
        cpt++;
      }
      sum/=vec.size();
      REQUIRE(sum < 1e-8);
    }
  };

  auto myTs1d = tarsier::make_timeSurfaceGenerator<200, 2, 5,-10000, Event1d, TsEvent1d>(kernel1d, tsEvent1dFromEvent, handlerTimeSurface1d);
  auto myTs2d = tarsier::make_timeSurfaceGenerator<200, 200, 2, 2,-10000, Event2d, TsEvent2d>(kernel2d, tsEvent2dFromEvent, handlerTimeSurface2d);

  myTs1d(Event1d{0, 10, 1});
  myTs1d(Event1d{10, 9, 1});
  myTs1d(Event1d{20, 8, 0});
  myTs1d(Event1d{30, 9, 0});
  myTs1d(Event1d{40, 10, 0});
  myTs1d(Event1d{50, 11, 0});
  myTs1d(Event1d{60, 12, 0});
  myTs1d(Event1d{70, 13, 0});
  myTs1d(Event1d{80, 12, 1});
  myTs1d(Event1d{90, 11, 1});
  go = true;
  myTs1d(Event1d{100, 10, 1});
  go = false;

  //----------

  myTs2d(Event2d{0,10,10,1});
  myTs2d(Event2d{10,11,10,0});
  myTs2d(Event2d{20,12,10,0});
  myTs2d(Event2d{30,12,11,1});
  myTs2d(Event2d{40,13,11,0});
  myTs2d(Event2d{50,14,12,1});
  myTs2d(Event2d{60,15,13,1});
  myTs2d(Event2d{70,14,12,0});
  myTs2d(Event2d{80,12,18,1});
  myTs2d(Event2d{90,14,15,1});
  myTs2d(Event2d{100,14,13,0});
  go = true;
  myTs2d(Event2d{110,14,13,1});
  go = false;
}
