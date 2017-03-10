#include "neuralpso.h"

NeuralPso::NeuralPso(NeuralNetParameters params) :
  _neuralNet(new NeuralNet(params)) {

}

NeuralPso::~NeuralPso() {
  delete _neuralNet;
}

void NeuralPso::build(vector<vector<vector<byte> > > &images, vector<byte> &labels) {

}

void NeuralPso::fly() {

}

double NeuralPso::getCost() {

  return 0;
}
