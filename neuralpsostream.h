#ifndef NEURALPSOSTREAM_H
#define NEURALPSOSTREAM_H

#include <string>
#include <memory>
#include <vector>

#include "NeuralNet/NeuralNet.h"
#include "PSO/pso.h"
#include "utils/util.h"

class NeuralPso;

/**
 *  @namespace NeuralPsoStream
 *  @details Provides saving and loading of the PSO State
 */
namespace NeuralPsoStream {

    // Get string definition of particle
    std::string stringifyParticle(const Particle<NeuralNet::EdgeType> & p);
    // Get string definition of edges
    std::string stringifyEdges(const NeuralNet::EdgeType & edges);

    // Create a particle from string
    Particle<NeuralNet::EdgeType> particleFromString(const std::string & particleState);
    // Create all particles from sub string
    std::vector<Particle<NeuralNet::EdgeType>> readParticlesFromString(const std::string & partSubString);
    // Create edges from string
    NeuralNet::EdgeType edgesFromString(const std::string & edgeString);

    // Get the token name between <> brackets and determine if open/closed, advance iterator
    std::string getTokenLabelFromString(const string &s, int &it, bool & isClose);
    // Move iterator to the next open or close token
    bool findNextToken(const std::string & fullstring, int & it);
    // Find the close token from the specific position
    bool findCloseToken(const std::string & fullstring, const std::string token, int & it);
    // Gets substring between open and close tokens
    std::string subStringByToken(const std::string & fullstring, const std::string token, int & it);

    // Clears spaces, tabs, and new lines from strings
    void cleanInputString(std::string & dirtyString);

}

#endif // NEURALPSOSTREAM_H
