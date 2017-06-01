#include "../source/hotsBlocs.hpp"

#include "catch.hpp"

struct TsEvent{
  int64_t p;
  std::vector<double> context;
};

TEST_CASE("Compute Hots from the given events", "[Hots]") {
  auto battacharya = [](std::vector<double>::iterator beg1,
                        std::vector<double>::iterator end1,
                        std::vector<double>::iterator beg2){
    double d = -log(std::inner_product(beg1,
                                       end1,
                                       beg2,
                                       0.,
                                       [](double a, double b){
                                         return a+b;
                                       },
                                       [](double a, double b){
                                         return sqrt(a*b);
                                       })
                    );
    return (std::isinf(d)) ? 1000 : (d < 1e-6) ? 0 : d;
  };

  bool go = false;
  auto tsEventFromTsEventIiwk = [&go](TsEvent ev, int64_t out_p){
    struct TsEvent tsev = ev;
    tsev.p = out_p;
    if(go){
      switch(ev.p){
      case 0:
        REQUIRE(out_p == 2);
        break;
      case 1:
        REQUIRE(out_p == 0);
        break;
      case 2:
        REQUIRE(out_p == 1);
        break;
      case 3:
        REQUIRE(out_p == 3);
        break;
      default:
        REQUIRE(0);
        break;
      }
    }
    return tsev;
  };

  auto tsEventFromTsEventStandard = [&go](TsEvent ev, int64_t out_p){
    struct TsEvent tsev = ev;
    tsev.p = out_p;
    if(go){
      switch(ev.p){
      case 0:
        REQUIRE(out_p == 0);
        break;
      case 1:
        REQUIRE(out_p == 1);
        break;
      case 2:
        REQUIRE(out_p == 2);
        break;
      case 3:
        REQUIRE(out_p == 3);
        break;
      default:
        REQUIRE(0);
        break;
      }
    }
    return tsev;
  };

  auto handlerHots = [](TsEvent ev){
  };

  auto iiwkLayer = tarsier::make_iiwkClustering<4,(5*2+1)*2,TsEvent,TsEvent>
    (2e-4, 2e-4,1,battacharya,tsEventFromTsEventIiwk,handlerHots);
  auto standardLayer = tarsier::make_standardClustering<4,(5*2+1)*2,TsEvent,TsEvent>
    (0.005, 20000.0,battacharya,tsEventFromTsEventStandard,handlerHots);

  srand(time(NULL));


  standardLayer(TsEvent{1,std::vector<double>{0.7, 0.8, 0.8, 0.9, 0.9, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.7, 0.7, 0.6, 0.6, 0.6, 0.5, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}});
  standardLayer(TsEvent{1,std::vector<double>{0.8, 0.9, 0.9, 0.9, 1, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.8, 0.8, 0.8, 0.7, 0.6, 0.5, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}});
  standardLayer(TsEvent{3,std::vector<double>{0.6, 0.6, 0.7, 0.7, 0.7, 0.7, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 1, 0.9, 0.8, 0.8, 0.7}});
  standardLayer(TsEvent{3,std::vector<double>{0.6, 0.7, 0.7, 0.7, 0.7, 0.8, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.9, 0.9, 0.9, 0.8, 0.8}});

  for(auto i = 0; i < 1000000; i++){
    if(i > 1000000-20){
      go = true;
    }

    switch(i%4){
    case 0:
      iiwkLayer(TsEvent{0,std::vector<double>{0.778801, 0.818731, 0.860708, 0.904837, 0.951229, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}});
      standardLayer(TsEvent{0,std::vector<double>{0.778801, 0.818731, 0.860708, 0.904837, 0.951229, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}});
      break;
    case 2:
      iiwkLayer(TsEvent{1,std::vector<double>{0.778801, 0.818731, 0.860708, 0.904837, 0.951229, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.740818, 0.704688, 0.670320, 0.637628, 0.606531, 0.576950, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}});
      standardLayer(TsEvent{1,std::vector<double>{0.778801, 0.818731, 0.860708, 0.904837, 0.951229, 1.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.740818, 0.704688, 0.670320, 0.637628, 0.606531, 0.576950, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000}});
      break;
    case 1:
      iiwkLayer(TsEvent{2,std::vector<double>{0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.951229, 0.904837, 0.860708, 0.818731, 0.778801}});
      standardLayer(TsEvent{2,std::vector<double>{0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.951229, 0.904837, 0.860708, 0.818731, 0.778801}});
      break;
    case 3:
      iiwkLayer(TsEvent{3,std::vector<double>{0.576950, 0.606531, 0.637628, 0.670320, 0.704688, 0.740818, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.951229, 0.904837, 0.860708, 0.818731, 0.778801}});
      standardLayer(TsEvent{3,std::vector<double>{0.576950, 0.606531, 0.637628, 0.670320, 0.704688, 0.740818, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 1.000000, 0.951229, 0.904837, 0.860708, 0.818731, 0.778801}});
      break;
    }
  }
}
