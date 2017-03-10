#include "neuralpso.h"

template <class T>
NeuralPso<T>::NeuralPso(NeuralNetParams params) :
  _neuralNet(new NeuralNet(params)) {

}

template <class T>
NeuralPso<T>::~NeuralPso() {
  delete _neuralNet;
}

template <class T>
void NeuralPso<T>::build() {

}

template <class T>
void NeuralPso<T>::fly() {

}

template <class T>
double NeuralPso<T>::getCost() {

  return 0;
}
