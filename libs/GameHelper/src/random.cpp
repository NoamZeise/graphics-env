#include <game/random.h>

#include <random>
#include <ctime>

namespace gamehelper {
  
class Random {
public:
    Random() {
	randomGen = std::mt19937(time(0));
	posReal = std::uniform_real_distribution<double>(0, 1);
	real = std::uniform_real_distribution<double>(-1, 1);
    }

    double Real() {
	return real(randomGen);
    }
    double PositiveReal() {
	return posReal(randomGen);
    }
private:
    std::mt19937 randomGen;
    std::uniform_real_distribution<double> posReal;
    std::uniform_real_distribution<double> real;
};

}

gamehelper::Random randGen;

namespace gamehelper {
namespace random {
  double real() {
      return randGen.Real();
  }
  
  double posReal() {
      return randGen.PositiveReal();
  }
}
}
