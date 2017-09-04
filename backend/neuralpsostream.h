#ifndef NEURALPSOSTREAM_H
#define NEURALPSOSTREAM_H

#include <string>
#include <memory>
#include <vector>

#include "NeuralNet/NeuralNet.h"
#include "PSO/pso.h"
#include "utils/util.h"
#include "custommath.h"

class NeuralPso;
//struct TrainingParameters;
struct PsoParams;
struct NeuralNetParameters;
struct FitnessParameters;

/**
 *  @namespace NeuralPsoStream
 *  @details Provides saving and loading of the PSO State
 */
namespace NeuralPsoStream {

    template <class T>
    std::string tokenizedValue(const T & val, const std::string & token);

    // Get string definition of parameters
//    std::string stringifyParameters(const TrainingParameters &params);
    // Get string definition of particle
    std::string stringifyParticle(const Particle<NeuralNet::EdgeType> & p);
    // Get string definition of edges
    std::string stringifyEdges(const NeuralNet::EdgeType & edges, const std::string & token);
    std::string stringifyEdges(const NeuralNet::EdgeType & edges);
    std::string stringifyRecEdges(const NeuralNet::RecEdgeType & edges, const std::string & token);
    std::string stringifyRecEdges(const NeuralNet::RecEdgeType & edges);

    std::string stringifyPParams(const PsoParams & p);
    std::string stringifyNParams(const NeuralNet::NeuralNetParameters & p);

    // Create parameter file from string
//    TrainingParameters parametersFromString(const std::string & pString);
    // Create PsoParameter from substring
    PsoParams psoParametersFromString(const std::string & ps);
    // Create NNParameters from substring
    NeuralNet::NeuralNetParameters nParametersFromString(const std::string & ps);
    // Create a particle from string
    Particle<NeuralNet::EdgeType> particleFromString(const std::string & particleState);
    // Create all particles from sub string
    std::vector<Particle<NeuralNet::EdgeType>> readParticlesFromString(const std::string & partSubString);
    // Create edges from string
    NeuralNet::EdgeType edgesFromString(const std::string & edgeString);

    NeuralNet::RecEdgeType recEdgesFromString(const std::string & edgeString);

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

    template <class T>
    std::string stringifyParamsNugget(const std::string & token, const T & val);

    template <class T>
    std::string stringifyParamsVector(const std::string & token, const std::vector<T> & val);

    template <class T>
    bool valFromNuggetString(const std::string & cleanString, const std::string & token, int & it, T & val);

    bool edgeFromNuggetString(const std::string & cleanString, const std::string & token, int & it, NeuralNet::EdgeType & val);

    std::string openToken(const std::string &);
    std::string closeToken(const std::string &);

}

#endif // NEURALPSOSTREAM_H
