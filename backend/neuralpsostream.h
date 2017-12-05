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
    std::string stringifyParticle(const Particle<NeuralNet::State> & p);
    // Get string definition of edges
    std::string stringifyState(const NeuralNet::State & state, const std::string & token);
    std::string stringifyState(const NeuralNet::State & state);

    std::string stringifyPParams(const PsoParams & p);
    std::string stringifyNParams(const NeuralNet::NeuralNetParameters & p);

    // Create parameter file from string
//    TrainingParameters parametersFromString(const std::string & pString);
    // Create PsoParameter from substring
    PsoParams psoParametersFromString(const std::string & ps);
    // Create NNParameters from substring
    NeuralNet::NeuralNetParameters nParametersFromString(const std::string & ps);
    // Create a particle from string
    Particle<NeuralNet::State> particleFromString(const std::string & particleState);
    // Create all particles from sub string
    std::vector<Particle<NeuralNet::State>> readParticlesFromString(const std::string & partSubString);
    // Create edges from string
    NeuralNet::State edgesFromString(const std::string & stateString);

    // Get the token name between <> brackets and determine if open/closed, advance iterator
    std::string getTokenLabelFromString(const string &s, int &it, bool & isClose);
    // Move iterator to the next open or close token
    bool findNextToken(const std::string & fullstring, int & it);
    // Find the close token from the specific position
    bool findCloseToken(const std::string & fullstring, const std::string token, int & it);
    // Gets substring between open and close tokens
    std::string subStringByToken(const std::string & fullstring, const std::string token, int & it);
    std::string subStringByToken(const string &fullString , const string & token);

    // Clears spaces, tabs, and new lines from strings
    void cleanInputString(std::string & dirtyString);
    void cleanInputString(std::string & dirtyString, const std::string & badChars);

    template <class T>
    std::string stringifyParamsNugget(const std::string & token, const T & val) {

        // Write the nugget
        std::string ps;

        // Write the open token
        ps.append("\t");
        ps.append(openToken(token));
        ps.append("\n\t\t");

        // Write the value
        ps.append(stringPut(val));

        // Write the close token
        ps.append("\n\t");
        ps.append(closeToken(token));
        ps.append("\n");

        // Return our string
        return ps;
    }

    template <class T>
    std::string stringifyParamsVector(const std::string & token, const std::vector<T> & val) {

        //!TEST!//
        std::string ps;

        // Write the open token
        ps.append("\t");
        ps.append(openToken(token));
        ps.append("\n\t\t");

        // Write the list of vector values
        for (size_t i = 0; i < val.size(); i++) {
            // Use the placement in the vector as a token
            ps.append(stringifyParamsNugget<T>(stringPut(i), val[i]));
        }

        // Write the close token
        ps.append("\n\t");
        ps.append(closeToken(token));
        ps.append("\n");

        // Return our string
        return ps;
    }

    template <class T>
    bool valFromNuggetString(const std::string & cleanString, const std::string & token, int & it, T & val) {

        // Move the iterator to the next available token in the full string
        if (!findNextToken(cleanString, it)) {
            // If no token found, bounce
            return false;
        }

        // Get the value string matching the token
        std::string valString = subStringByToken(cleanString, token, it);
        // If we found the value
        if (!valString.empty()) {
            // Get the value from the string
            T val_ = numberFromString<T>(valString);
            val = val_;
        }
        return true;
    }

    bool stateFromNuggetString(const std::string & cleanString, const std::string & token, int & it, NeuralNet::State & val);

    std::string openToken(const std::string &);
    std::string closeToken(const std::string &);

    template <class T>
    bool vectorFromString(const std::string & s, const std::string & token, std::vector<T> & val) {
        std::vector<T> buffer;
        std::string vectorString = subStringByToken(s, token);
        int it = 0;
        int i = 0;
        while (it < static_cast<int>(vectorString.size() - (token.size() + 2))) {
            T newVal;
            if (!valFromNuggetString(vectorString, stringPut(i++), it, newVal)) {
                return false;
            } else {
                buffer.push_back(newVal);
            }
        }
        val = buffer;
        return true;
    }

    NeuralNet::State stateFromString(const std::string & stateString);

}

#endif // NEURALPSOSTREAM_H
