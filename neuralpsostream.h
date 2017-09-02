#ifndef NEURALPSOSTREAM_H
#define NEURALPSOSTREAM_H

#include <string>
#include <memory>
#include <vector>

#include "NeuralNet/NeuralNet.h"
#include "PSO/pso.h"
#include "utils/util.h"

class NeuralPso;

namespace NeuralPsoStream {

    std::string stringifyParticle(const Particle<NeuralNet::EdgeType> & p);
    std::string stringifyEdges(const NeuralNet::EdgeType & edges);
    std::unique_ptr<Particle<NeuralNet::EdgeType>>
          particleFromString(const std::string & particleState);
    NeuralNet::EdgeType edgesFromString(const std::string & edgeString);
    std::string getTokenLabelFromString(const string &s, int &it, bool & isClose);
    bool findNextToken(const std::string & fullstring, int & it);
    bool findCloseToken(const std::string & fullstring, const std::string token, int & it);
    std::string subStringByToken(const std::string & fullstring, const std::string token, int & it);
    void cleanInputString(std::string & dirtyString);

}

#endif // NEURALPSOSTREAM_H
