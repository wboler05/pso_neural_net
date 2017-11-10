#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <limits>
#include <ctime>
#include <cmath>
using namespace std;

std::string fileName="C:\\Users\\wboler\\pso_neural_net\\Outage Data\\Final Sets\\ECE570_Final_Dataset.csv";
int buckets = 5;
double minValue = std::numeric_limits<double>::max();
double maxValue = 0.0;

struct Particle {
    std::vector<double> x;
    std::vector<double> v;
    std::vector<double> pb_x;
    std::vector<double> lb_x;
    double fit = -std::numeric_limits<double>::max();;
    double pb_fit = -std::numeric_limits<double>::max();
    double lb_fit = -std::numeric_limits<double>::max();
};

class Pso {
public:
    Pso(const std::vector<double> inputs);

    void process();

    void initialize();

    void fly();
    void evaluate();
    void getFitness();

    void printGb();
    void testGb();

private:
    std::vector<Particle> _particles;
    std::vector<double> _inputs;
    Particle _gb;

    int _population = 50;
    int _neighbors = 10;
    size_t minEpochs = 1;
    size_t maxEpochs = 500;

    double dt = 0.5;

    std::vector<double> sort(const std::vector<double> &);
};

Pso::Pso(const std::vector<double> inputs) : _inputs(inputs){
    initialize();
}

void Pso::process() {
    size_t epochs = 0;
    bool terminate = false;

    do {
        epochs++;

        std::cout << "Epoch: " << epochs << std::endl;

        fly();
        getFitness();
        evaluate();

        printGb();

        if (epochs > minEpochs) {
            if (epochs >= maxEpochs) {
                terminate = true;
            }
        }
    } while (!terminate);
}

void Pso::initialize() {
    _particles.resize(_population);
    for (size_t i = 0; i < _particles.size(); i++) {
        Particle & p = _particles[i];

        p.x.resize(buckets-2);
        p.v.resize(buckets-2, 0);
        p.pb_x.resize(buckets-2);
        p.lb_x.resize(buckets-2);
        for (size_t j = 0; j < buckets-2; j++) {
            p.x[j] = (double)(rand() % ((int) maxValue - (int)minValue)) + minValue;
            //std::cout << "Particle, range " << i << ", " << j << ": " << p.x[j] << std::endl;
            p.pb_x[j] = 0;
            p.lb_x[j] = 0;
        }
    }
    _gb.pb_x.resize(buckets-2);
}

void Pso::fly() {
    const double C1=2.54, C2=2.54;

    for (size_t i = 0; i < _particles.size(); i++) {
        for (size_t j = 0; j < _particles[i].x.size(); j++) {
            double c1 = C1 * (double)(rand() % 10000) / 10000.0;
            double c2 = C2 * (double)(rand() % 10000) / 10000.0;

//            std::cout << "Particle, range " << i << ", " << j << ": " \
//                << "Vel: " << _particles[i].v[j];

            _particles[i].v[j] = _particles[i].v[j] * 0.75 +
                c1 * (_particles[i].pb_x[j] - _particles[i].x[j]) +
                c2 * (_particles[i].lb_x[j] - _particles[i].x[j]);

//            std::cout << " NewV: " << _particles[i].v[j] <<
//                " X: " << _particles[i].x[j];

            _particles[i].x[j] += _particles[i].v[j];

            _particles[i].x[j] = max(_particles[i].x[j], minValue);
            _particles[i].x[j] = min(_particles[i].x[j], maxValue);

//            std::cout << " NewX: " << _particles[i].x[j] << std::endl;
        }
    }
}

void Pso::testGb() {
    std::vector<double> histo(buckets, 0);
    std::vector<double> ranges = sort(_gb.pb_x);

    for (size_t i = 0; i < _inputs.size(); i++) {
      for (size_t h = 0; h < histo.size(); h++) {
        if (h == 0) {
          if (_inputs[i] < 1) {
            histo[h]++;
            break;
          }
        } else if (h == histo.size()-1) {
          if (_inputs[i] >= ranges[h-2]) {
            histo[h]++;
            break;
          }
        } else {
          if (_inputs[i] < ranges[h-1]) {
            histo[h]++;
            break;
          }
        }
      }
    }

    std::cout << "Gb Histo: " << std::endl;
    for (size_t i = 0; i < histo.size(); i++) {
        std::cout << " - " << i << ": " << histo[i] << std::endl;
    }

}

void Pso::getFitness() {
    for (size_t i = 0; i < _particles.size(); i++) {
        std::vector<double> histo(buckets, 0);
        std::vector<double> ranges = sort(_particles[i].x);

        bool failCheck = false;

        for (size_t j = 0; j < _inputs.size(); j++) {
//            std::cout << "Input: " << _inputs[j] << std::endl;

//            std::cout << "Ranges: " << std::endl;
//            for (size_t k = 0 ; k < ranges.size(); k++) {
//              std::cout << " - " << k << ": " << ranges[k] << std::endl;
//            }

            for (size_t h = 0; h < histo.size(); h++) {
              if (h == 0) {
                if (_inputs[j] < 1) {
                  histo[h]++;
                  break;
                }
              } else if (h == histo.size()-1) {
                if (_inputs[j] < ranges[h-2] || ranges[h-2] == 0) {
                  histo[0]++;
                  failCheck = true;
                  break;
                } else {
                  histo[h]++;
                }
              } else {
                if (_inputs[j] < ranges[h-1]) {
                  histo[h]++;
                  break;
                }
              }
            }

//            std::cout << "Histo: " << std::endl;
//            for (size_t k = 0; k < histo.size(); k++) {
//              std::cout << " - " << k << ": " << histo[k] << std::endl;
//            }
//            std::cout << std::endl;
            /*
            if (_inputs[j] == 0) {
                histo[0]++;
            } else if (_inputs[j] < ranges[0]) {
                histo[1]++;
            } else if (_inputs[j] < ranges[1]) {
                histo[2]++;
            } else if (_inputs[j] < ranges[2]) {
                histo[3]++;
            } else {
                histo[4]++;
            }*/
        }

//        std::cout << "Histo: " << std::endl;
//        for (size_t j = 0; j < histo.size(); j++) {
//            std::cout << " - " << j << ": " << histo[j] << std::endl;
//        }

        double fit = 0;

        for (size_t j = 0; j < histo.size(); j++) {
            for (size_t k = 0; k < histo.size(); k++) {
                fit += abs(histo[j] - histo[k]);
            }
        }

 /*       double mean = 0;
        for (size_t j = 0; j < histo.size(); j++) {
            mean += histo[i];
        }
        mean /= (double) histo.size();

        for (size_t j = 0; j < histo.size(); j++) {
            double what = histo[j] - mean;
            fit += pow(what, 2);
        }
        fit = sqrt(fit);*/
        _particles[i].fit = -fit;

        for (size_t j = 0; j < histo.size(); j++) {
            if (histo[j] >= maxValue-1.0 ||
                histo[j] == minValue ||
                failCheck)
            {
                _particles[i].fit *= 100;
                break;
            }
        }

        for (size_t j = 1; j < ranges.size(); j++) {
          if (ranges[j-1] == ranges[j]) {
            _particles[i].fit *= 100;
            break;
          }
        }

    }
}

void Pso::evaluate() {
    for (size_t i = 0; i < _particles.size(); i++) {
        if (_particles[i].fit > _particles[i].pb_fit) {
            _particles[i].pb_fit = _particles[i].fit;
            _particles[i].pb_x = _particles[i].x;
            for (size_t j = 0; j < _particles[i].x.size(); j++) {
                _particles[i].pb_x[j] = _particles[i].x[j];
            }
        }
    }

    for (size_t i = 0; i < _particles.size(); i++) {
        for (size_t j = 0; j < _neighbors; j++) {
            size_t n = ( i + j ) % _particles.size();
            if (_particles[n].pb_fit > _particles[i].lb_fit) {
                _particles[i].lb_fit = _particles[n].pb_fit;
                for (size_t k = 0; k < _particles[i].x.size(); k++) {
                    _particles[i].lb_x[k] = _particles[n].pb_x[k];
                }
            }
        }
    }

    for (size_t i = 0; i < _particles.size(); i++) {
        if (_particles[i].pb_fit > _gb.pb_fit) {
            _gb.pb_fit = _particles[i].pb_fit;
            for (size_t j = 0; j < _particles[i].pb_x.size(); j++) {
                _gb.pb_x[j] = _particles[i].pb_x[j];
            }
        }
    }

}

void Pso::printGb() {
    std::cout << "GB: " << std::endl;
    for (size_t i = 0; i < _gb.pb_x.size(); i++) {
        std::cout << " - " << i << ": " << _gb.pb_x[i] << std::endl;
    }
}

std::vector<double> Pso::sort(const std::vector<double> & v) {
    std::vector<double> sortedV(v.size());
//    std::cout << "Unsorted: " << std::endl;
    for (size_t i = 0; i < v.size(); i++) {
//        std::cout << " - " << i << ": " << v[i] << std::endl;
        sortedV[i] = v[i];
    }
    for (size_t i = 0; i < sortedV.size(); i++) {
        size_t swapIt = i;
        for (size_t j = i+1; j < sortedV.size(); j++) {
            if (sortedV[j] < sortedV[swapIt]) {
                swapIt = j;
            }
        }
        std::swap(sortedV[i], sortedV[swapIt]);
    }

//    std::cout << "Sorted: " << std::endl;
//    for (size_t i = 0; i < sortedV.size(); i++) {
//      std::cout << " - " << i << ": " << sortedV[i] << std::endl;
//    }

    return sortedV;
}

std::vector<double> loadInputs();
void maxMin(const std::vector<double> & inputs);
void parseArgs(int argc, char ** argv);

int main (int argc, char ** argv) {
    srand(time(NULL));

    parseArgs(argc, argv);

    std::vector<double> inputValues = loadInputs();
    maxMin(inputValues);

    Pso pso(inputValues);

    pso.process();

    std::cout << "We done. " << std::endl;
    pso.printGb();

    std::cout << "Min: " << minValue << std::endl;
    std::cout << "Max: " << maxValue << std::endl;

    pso.testGb();

}

void parseArgs(int argc, char ** argv) {
  if (argc > 1) {
    for (size_t i = 1; i < argc; i++) {
      std::string argString(argv[i]);
      if (argString == "-f") {
        if (i + 1 >= argc) {
          std::cout << "Error: improper file path format: -f <path>" << std::endl;
          exit(1);
        } else {
          fileName = argv[++i];
        }
      } else if (argString == "-c") {
        if (i + 1 >= argc) {
          std::cout << "Error: need a count for buckets: -c <N>" << std::endl;
          exit(1);
        } else {
          int newBucket = 0;
          stringstream ss;
          ss << argv[++i];
          ss >> newBucket;
          if (newBucket > 1) {
            buckets = newBucket;
          }
        }
      }
    }

  }
}

std::vector<double> loadInputs () {
    std::ifstream file(fileName, ifstream::in);
    std::string inputLine;

    std::vector<double> inputValues;
    int offsetSkips = 2;
    int lines = 0;
    int valueIndex = 30;

    while (file.good()) {
        lines++;
        getline(file, inputLine, '\n' );
//        std::cout << "Line: " << inputLine << std::endl;
        if (lines <= offsetSkips) {
            continue;
        }
        int count = 0;
        for (size_t i = 0; i < inputLine.size(); i++) {
            if (inputLine[i] == ',') {
                count++;
                if (count == valueIndex-1) {
                    for (size_t j = i+1; j < inputLine.size(); j++) {
                        if (inputLine[j] == ',' || inputLine[j] == '\n' || j == inputLine.size() -1) {
                            string newValue = inputLine.substr(i+1, j-i);
//                            std::cout << "New Value: " << newValue << std::endl;
                            double newValue_d;
                            stringstream ss;
                            ss << newValue;
                            ss >> newValue_d;
                            inputValues.push_back(newValue_d);
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    std::cout << "Values: " << inputValues.size() << std::endl;

    return inputValues;
}

void maxMin(const std::vector<double> & inputs) {
    for (size_t i = 0; i < inputs.size(); i++) {
        minValue = std::min(inputs[i], minValue);
        maxValue = std::max(inputs[i], maxValue);
    }
}
