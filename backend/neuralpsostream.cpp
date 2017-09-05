#include "neuralpsostream.h"

namespace NeuralPsoStream {

std::string stringifyParticle(const Particle<NeuralNet::EdgeType> & p) {
    std::string ps;

    // Particle Position X
    ps.append(stringifyEdges(p._x, "_x"));

    // Particle Velocity
    ps.append(stringifyEdges(p._v, "_v"));

    // Particle Personal Best X
    ps.append(stringifyEdges(p._x_pb, "_x_pb"));

    // Particle Local Best
    ps.append(stringifyEdges(p._x_lb, "_x_lb"));

    // Particle Personal Best Fitness
    ps.append(tokenizedValue(p._fit_pb, "_fit_pb"));

    // Particle Local Best Fitness
    ps.append(tokenizedValue(p._fit_lb, "_fit_lb"));

    int worstFlag = p._worstFlag ? 1 : 0;
    // Particle Worst Flag
    ps.append(tokenizedValue(worstFlag, "_worstFlag"));

    // Particle Points
    ps.append(tokenizedValue(p._points, "_points"));

    return ps;
}

template <class T>
std::string tokenizedValue(const T & val, const std::string & token) {
    std::string ps;

    ps.append("\t");
    ps.append(openToken(token));
    ps.append("\n");
    ps.append(stringPut(val));
    ps.append("\t");
    ps.append(closeToken(token));
    ps.append("\n");

    return ps;
}

std::string stringifyEdges(const NeuralNet::EdgeType & edges, const std::string & token) {
    std::string ps;

    ps.append("\t");
    ps.append(openToken(token));
    ps.append("\n");
    ps.append(stringifyEdges(edges));
    ps.append("\t");
    ps.append(closeToken(token));
    ps.append("\n");

    return ps;
}

std::string stringifyEdges(const NeuralNet::EdgeType & edges) {
    std::string particleString;

    for (size_t i = 0; i < edges.size(); i++) {
        particleString.append("\t\t{\n");
        for (size_t j = 0; j < edges[i].size(); j++) {
            particleString.append("\t\t\t{\n\t\t\t\t");
            for (size_t k = 0; k < edges[i][j].size(); k++) {
                particleString.append(stringPut(edges[i][j][k]));
                particleString.append(",");
                if (k == edges[i][j].size() -1) {
                    particleString.append("\n");
                }
            }
            particleString.append("\t\t\t}\n");
        }
        particleString.append("\t\t}\n");
    }

    return particleString;
}

std::string stringifyRecEdges(const NeuralNet::RecEdgeType & edges, const std::string & token) {
    std::string ps;

    ps.append("\t");
    ps.append(openToken(token));
    ps.append("\n");
    ps.append(stringifyRecEdges(edges));
    ps.append("\t");
    ps.append(closeToken(token));
    ps.append("\n");

    return ps;
}

std::string stringifyRecEdges(const NeuralNet::RecEdgeType & edges) {

    std::string ps;

    for (size_t i = 0; i < edges.size(); i++) {
        ps.append("\t\t{\n");
        for (size_t j = 0; j < edges[i].size(); j++) {
            ps.append(stringPut(edges[i][j]));
            ps.append(",");
            if (j == edges[i].size() -1) {
                ps.append("\n");
            }
        }
        ps.append("\t\t}\n");
    }

    return ps;
}

NeuralNet::EdgeType edgesFromString(const std::string & edgeString) {

    NeuralNet::EdgeType e;

    int it = 0;
    while ( it < (int) edgeString.size()) {
        if (edgeString[it]  == '{') {
            it++;
            while (edgeString[it] != '}') {
                std::vector<std::vector<real>> innerNet;
                if (edgeString[it] == '{') {
                    it++;
                    while (edgeString[it] != '}') {
                        std::vector<real> leftNode;
                        int jt = it+1;
                        while (edgeString[jt] != '}') {
                            if (edgeString[jt] == ',' || edgeString[jt] == '}') {
                                std::string valString = edgeString.substr(it,jt-it);
                                real val = numberFromString<real>(valString);
                                leftNode.push_back(val);
                                it = jt+1;
                            }
                            jt++;
                        }
                        innerNet.push_back(leftNode);
                        it = jt+1;
                        if (edgeString[it] == '{') {
                            it++;
                        }
                    }
                }
                e.push_back(innerNet);
                it++;
                if (edgeString[it] == '{') {
                    it++;
                } else if (edgeString[it] == 0) {
                    return e;
                }
            }
        }
    }

    return e;
}

NeuralNet::RecEdgeType recEdgesFromString(const string &edgeString) {
    NeuralNet::RecEdgeType e;

    int it = 0;

    while (it < edgeString.size()) {
        if (edgeString[it++] == '{') {
            std::vector<real> cache;
            while (edgeString[it] != '}') {
                int jt = it+1;
                while (edgeString[jt] != ',') {
                    jt++;
                }
                std::string val = edgeString.substr(it, jt-it);
                cache.push_back(numberFromString<real>(val));
                it = jt + 1;
            }
            e.push_back(cache);
            if (edgeString[it] == '}') {
                return e;
            }
        }
    }

    // Case is when edgeString is empty
    return e;
}

Particle<NeuralNet::EdgeType> particleFromString(const std::string & particleString) {
    typedef Particle<NeuralNet::EdgeType> P;
    P pParticle;
    int it = 0;

    // Particle Position X
    {
        NeuralNet::EdgeType xEdge;
        if (!edgeFromNuggetString(particleString, "_x", it, xEdge)) {
            return P();
        } else {
            pParticle._x = xEdge;
        }
    }

    // Particle Velocity
    {
        NeuralNet::EdgeType vEdge;
        if (!edgeFromNuggetString(particleString, "_v", it, vEdge)) {
            return P();
        } else {
            pParticle._v = vEdge;
        }
    }

    // Particle Personal Best X
    {
        NeuralNet::EdgeType xbEdge;
        if (!edgeFromNuggetString(particleString, "_x_pb", it, xbEdge)) {
            return P();
        } else {
            pParticle._x_pb = xbEdge;
        }
    }

    // Particle Local Best
    {
        NeuralNet::EdgeType xlEdge;
        if (!edgeFromNuggetString(particleString, "_x_lb", it, xlEdge)) {
            return P();
        } else {
            pParticle._x_lb = xlEdge;
        }
    }

    // Particle Personal Best Fitness
    real fpb = 0;
    if (!valFromNuggetString(particleString, "_fit_pb", it, fpb)) {
        return P();
    } else {
        pParticle._fit_pb = fpb;
    }

    // Particle Local Best Fitness
    real flb = 0;
    if (!valFromNuggetString(particleString, "_fit_lb", it, flb)) {
        return P();
    } else {
        pParticle._fit_lb = flb;
    }

    // Particle Worst Flag
    int worst = 0;
    if (!valFromNuggetString(particleString, "_worstFlag", it, worst)) {
        return P();
    } else {
        pParticle._worstFlag = worst == 1;
    }

    // Particle Points
    real points = 0;
    if (!valFromNuggetString(particleString, "_points", it, points)) {
        return P();
    } else {
        pParticle._points = points;
    }

    return pParticle;
}

bool edgeFromNuggetString(const std::string & cleanString, const std::string & token, int & it, NeuralNet::EdgeType & val) {
    //!TEST!//

    // Move the iterator to the next token in the full string
    if (!findNextToken(cleanString, it)) {
        // If no tokens found, bounce
        return false;
    }
    // Get the edge string
    std::string edgeString = subStringByToken(cleanString, token, it);
    if (!edgeString.empty()) {
        // If it's not a failure, get the edges
        NeuralNet::EdgeType edge = edgesFromString(edgeString);
        // Send the edges to the return value
        val = edge;
    }

    return true;
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

template <class T>
bool vectorFromNuggetString(const std::string & cleanString, const std::string & token, int & it, std::vector<T> & val) {
    //!TEST!//

    if (!findNextToken(cleanString, it)) {
        return false;
    }

    std::string valString = subStringByToken(cleanString, token, it);
    if (!valString.empty()) {

        std::vector<T> vec_;
        T val_;
        int i = 0;
        int it_ = 0;
        while (valFromNuggetString(valString, stringPut(i++), it_, val_)) {
            vec_.push_back(val_);
        }
        val = vec_;
    }
    return true;
}

/**
 * @brief getTokenLabelFromString
 * @details Returns the token at the starting position
 * @details "it" must be the starting "<" position of the token
 * @param s - string containing the token
 * @param it - Position of the "<" start of the token
 * @param isClose - True if the token is a </token> terminate token.
 * @return std::string - Returns the token. Empty "" string is fail
 */
std::string getTokenLabelFromString(const string &s, int &it, bool & isClose) {


    std::string token;                      // We're going to read the <token>
    if (it >= (int) s.size()) return token; // If 'it' is incorrect, return empty token
    int i = it;                             // Setup local 'i' iterator
    if (s[i] == '<') {                      // If we do have a token, search
        while (s[i] != '>') {               // While it's not closed
            i++;                            // Increment the iterator
        }

        if (i > it) {                       // If iterator has incremented
            token = s.substr(it+1, i-(it+1));     // Token is the substring between 'it' and 'i'
        } else {                            // else
            isClose = false;                // Default not a closing token
            return token;                   // Return empty token
        }

        if (token[0] == '/') {              // If '/' preceeds token
            isClose = true;                 // We're closing
            token.erase(0,1);               // Erase the '/'
        } else {                            // Else
            isClose = false;                // We're not closing
        }
        it = i;                             // Advance the iterator for the user
    }

    return token;                           // Return found or empty token
}

/**
 * @brief findNextToken
 * @details Finds the next token and returns true if it found one
 * @details "it" holds the position of the found token
 * @param fullstring - Full string to search through
 * @param it - Position of token
 * @return Boolean - True if a token was found, false if not
 */
bool findNextToken(const std::string & fullstring, int & it) {
    if (it >= (int) fullstring.size()) {
        return false;
    }

    while (it < (int) fullstring.size()) {
        if (fullstring[it] != '<') {
            it++;
        } else {
            return true;
        }
    }
    return false;
}

/**
 * @brief findCloseToken
 * @details Returns true if finds the matching close token
 * @details Iterator will be held in 'it'
 * @param fullstring - Full string to search
 * @param token - token to match
 * @param it - position found
 * @return Boolean - True if found, false if not
 */
bool findCloseToken(const string &fullstring, const string token, int &it) {
    if (fullstring.size() < token.size()) {
            return false;
    }

    if (it >= (int) fullstring.size()) {
        return false;
    }

    int i = it;
    while (i < (int) fullstring.size()) {
        if (!findNextToken(fullstring, i)) {
            return false;
        }
        bool isClosed = false;
        std::string c_token = getTokenLabelFromString(fullstring, i, isClosed);
        if (isClosed) {
            if (c_token == token) {
                it = i;
                return true;
            }
        }
    }
    return false;

}

/**
 * @brief subStringByToken
 * @details Returns the substring found between the open/closed tokens
 * @param fullstring - Fullstring to search
 * @param token - Token to match for open and close
 * @param it - Starting position of the first token, returns last position after close token
 * @return std::string - Substring between the tokens
 */
std::string subStringByToken(const string &fullstring, const string token, int &it) {
    std::string subString;

    if (token.size() > fullstring.size()) {
        return subString;
    }

    if (it >= (int) fullstring.size()) {
        return subString;
    }

    bool isClosed = false;
    if (!findNextToken(fullstring, it)) {
        return std::string();
    }
    std::string openToken = getTokenLabelFromString(fullstring, it, isClosed);
    if (isClosed || openToken != token) {
        return subString;
    }

    int startIt = it;
    int endIt = it;
    int matches = 0;
    bool foundClose = false;
    std::string closeToken;
    do {

        if (!findNextToken(fullstring, it)) {
            return subString;
        }
        endIt = it;
        closeToken = getTokenLabelFromString(fullstring, it, isClosed);

        if (closeToken.size() == 0) {
            return subString;
        }

        if (closeToken == token) {
            if (!isClosed) {
                matches++;
                continue;
            } else {
                if (matches-- == 0) {
                    foundClose = true;
                    break;
                }
            }
        }
    } while (it < (int) fullstring.size());

    if (foundClose) {
        subString = fullstring.substr(startIt+1, endIt-(startIt+1));
        //it = endIt;
    }

    return subString;
}

void cleanInputString(std::string & dirtyString) {

    std::string badChars;
    badChars.append(" \t\n");

    int it = 0;
    while (it < (int) dirtyString.size()) {
        bool modified = false;
        for (int i = 0; i < (int) badChars.size(); i++) {
            if (badChars[i] == dirtyString[it]) {
                dirtyString.erase(it,1);
                modified = true;
                break;
            }
        }
        if (!modified) {
            it++;
        }
    }
}

std::vector<Particle<NeuralNet::EdgeType>> readParticlesFromString(const std::string & partSubString) {
    std::vector<Particle<NeuralNet::EdgeType>> particles;
    int it = 0;
    int i = 0;
    do {
        if (!findNextToken(partSubString, it)) {
            break;
        }
        std::string numString = stringPut(i++);
        std::string pString = subStringByToken(partSubString, numString, it);
        if (!pString.empty()) {
            Particle<NeuralNet::EdgeType> p = particleFromString(pString);
            particles.push_back(p);
        }

    }while (it < (int)partSubString.size());

    return particles;
}

//std::string stringifyParameters(const TrainingParameters & params) {
//    std::string ps;

//    ps.append(stringifyPsoParams(params.pp));
//    ps.append(stringifyNNParams(params.np));
////    ps.append(stringifyFParams(params.fp));

//    return ps;
//}

std::string stringifyPParams(const PsoParams & p) {
    //!TEST!//

    std::string ps;
    std::string token("PsoParams");

    ps.append(openToken(token));
    ps.append("\n");

    // Particles
    ps.append(stringifyParamsNugget<uint32_t>("particles", p.particles));

    // Neighbors
    ps.append(stringifyParamsNugget<uint32_t>("neighbors", p.neighbors));

    // Iterations
    ps.append(stringifyParamsNugget<uint32_t>("iterations", p.iterations));

    // delta
    ps.append(stringifyParamsNugget<real>("delta", p.delta));

    // vDelta
    ps.append(stringifyParamsNugget<real>("vDelta", p.vDelta));

    // vLimit
    ps.append(stringifyParamsNugget<real>("vLimit", p.vLimit));

    // Window
    ps.append(stringifyParamsNugget<uint32_t>("window", p.window));

    // termIterationFlag
    int termIterationFlag = p.termIterationFlag ? 1 : 0;
    ps.append(stringifyParamsNugget<int>("termIterationFlag", termIterationFlag));

    // TermDeltaFlag
    int termDeltaFlag = p.termDeltaFlag ? 1 : 0;
    ps.append(stringifyParamsNugget<int>("termDeltaFlag", termDeltaFlag));

    // BackPropagation
    int backPropagation = p.backPropagation ? 1 : 0;
    ps.append(stringifyParamsNugget<int>("backPropagation", backPropagation));

    // IterationsPerLevel
    ps.append(stringifyParamsNugget<int>("iterationsPerLevel", p.iterationsPerLevel));

    // StartPoints
    ps.append(stringifyParamsNugget<int>("startPoints", p.startPoints));

    // pbPoints
    ps.append(stringifyParamsNugget<int>("pbPoints", p.pbPoints));

    // lbPoints
    ps.append(stringifyParamsNugget<int>("lbPoints", p.lbPoints));

    // gbPoints
    ps.append(stringifyParamsNugget<int>("gbPoints", p.gbPoints));

    // weakPoints
    ps.append(stringifyParamsNugget<int>("weakPoints", p.weakPoints));

    // decayPoints
    ps.append(stringifyParamsNugget<int>("decayPoints", p.decayPoints));

    ps.append(closeToken(token));
    ps.append("\n");

    return ps;
}

std::string stringifyNParams(const NeuralNet::NeuralNetParameters & p) {
    //!TEST!//

    std::string ps;
    std::string token("NeuralNetParameters");

    ps.append(openToken(token));
    ps.append("\n");

    // inputs
    ps.append(stringifyParamsNugget<int>("inputs", p.inputs));

    // innerNets
    ps.append(stringifyParamsNugget<int>("innerNets", p.innerNets));

    // innerNetNodes
    ps.append(stringifyParamsVector<int>("innerNetNodes", p.innerNetNodes));

    // outputs
    ps.append(stringifyParamsNugget<int>("outputs", p.outputs));

    // testIterations
    ps.append(stringifyParamsNugget<int>("testIterations", p.testIterations));

    ps.append(closeToken(token));
    ps.append("\n");

    return ps;
}

std::string stringifyFParams(const FitnessParameters & p) {
    std::string ps;
    std::string token("FitnessParameters");

    ps.append(openToken(token));
    ps.append("\n");

    // Put stuff here later

    ps.append(closeToken(token));
    ps.append("\n");

    return ps;
}

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

//TrainingParameters parametersFromString(const std::string & pString) {
//    //!TEST!//
//}

PsoParams psoParametersFromString(const std::string & ps) {
    //!TEST!//
    PsoParams p;
    PsoParams emptyP;
    int it = 0;

    // Particles
    uint32_t particles = 0;
    if (!valFromNuggetString(ps, "particles", it, particles)) {
        return emptyP;
    } else {
        p.particles = particles;
    }

    // Neighbors
    uint32_t neighbors=0;
    if (!valFromNuggetString(ps, "neighbors", it, neighbors)) {
        return emptyP;
    } else {
        p.neighbors = neighbors;
    }

    // Iterations
    uint32_t iterations = 0;
    if (!valFromNuggetString(ps, "iterations", it, iterations)) {
        return emptyP;
    } else {
        p.iterations = iterations;
    }

    // Delta
    real delta = 0;
    if (!valFromNuggetString(ps, "delta", it, delta)) {
        return emptyP;
    } else {
        p.delta = delta;
    }

    // vDelta
    real vDelta =0;
    if (!valFromNuggetString(ps, "vDelta", it, vDelta)) {
        return emptyP;
    } else {
        p.vDelta = vDelta;
    }

    // vLimit
    real vLimit = 0;
    if (!valFromNuggetString(ps, "vLimit", it, vLimit)) {
        return emptyP;
    } else {
        p.vLimit = vLimit;
    }

    // Window
    uint32_t window = 0;
    if (!valFromNuggetString(ps, "window", it, window)) {
        return emptyP;
    } else {
        p.window = window;
    }

    // TermIterationFlag (bool)
    int termIterationFlag = 0;
    if (!valFromNuggetString(ps, "termIterationFlag", it, termIterationFlag)) {
        return emptyP;
    } else {
        p.termIterationFlag = termIterationFlag == 1;
    }

    // TermDeltaFlag (bool)
    int termDeltaFlag = 0;
    if (!valFromNuggetString(ps, "termDeltaFlag", it, termDeltaFlag)) {
        return emptyP;
    } else {
        p.termDeltaFlag = termDeltaFlag == 1;
    }

    // BackPropagation (bool)
    int backPropagation = 0;
    if (!valFromNuggetString(ps, "backPropagation", it, backPropagation)) {
        return emptyP;
    } else {
        p.backPropagation = backPropagation == 1;
    }

    // IterationsPerLevel
    int iterationsPerLevel = 0;
    if (!valFromNuggetString(ps, "iterationsPerLevel", it, iterationsPerLevel)) {
        return emptyP;
    } else {
        p.iterationsPerLevel = iterationsPerLevel;
    }

    // StartPoints
    int startPoints = 0;
    if (!valFromNuggetString(ps, "startPoints", it, startPoints)) {
        return emptyP;
    } else {
        p.startPoints = startPoints;
    }

    // pbPoints
    int pbPoints = 0;
    if (!valFromNuggetString(ps, "pbPoints", it, pbPoints)) {
        return emptyP;
    } else {
        p.pbPoints = pbPoints;
    }

    // lbPoints
    int lbPoints = 0;
    if (!valFromNuggetString(ps, "lbPoints", it, lbPoints)) {
        return emptyP;
    } else {
        p.lbPoints = lbPoints;
    }

    // gbPoints
    int gbPoints = 0;
    if (!valFromNuggetString(ps, "gbPoints", it, gbPoints)) {
        return emptyP;
    } else {
        p.gbPoints = gbPoints;
    }

    // WeakPoints
    int weakPoints = 0;
    if (!valFromNuggetString(ps, "weakPoints", it, weakPoints)) {
        return emptyP;
    } else {
        p.weakPoints = weakPoints;
    }

    // DecayPoints
    int decayPoints = 0;
    if (!valFromNuggetString(ps, "decayPoints", it, decayPoints)) {
        return emptyP;
    } else {
        p.decayPoints = decayPoints;
    }

    return p;
}

NeuralNet::NeuralNetParameters nParametersFromString(const std::string & ps) {
    //!TEST!//
    NeuralNet::NeuralNetParameters p;
    NeuralNet::NeuralNetParameters emptyP;

    int it = 0;

    // Inputs
    int inputs = 0;
    if (!valFromNuggetString(ps, "inputs", it, inputs)) {
        return emptyP;
    } else {
        p.inputs = inputs;
    }

    // InnerNets
    int innerNets = 0;
    if (!valFromNuggetString(ps, "innerNets", it, innerNets)) {
        return emptyP;
    } else {
        p.innerNets = innerNets;
    }

    {
        // InnerNetNodes
        std::vector<int> innerNetNodes;
        if (!vectorFromNuggetString(ps, "innerNetNodes", it, innerNetNodes)) {
            return emptyP;
        } else {
            p.innerNetNodes = innerNetNodes;
        }
    }

    // Outputs
    int outputs = 0;
    if (!valFromNuggetString(ps, "outputs", it, outputs)) {
        return emptyP;
    } else {
        p.outputs = outputs;
    }

    // testIterations
    int testIterations = 0;
    if (!valFromNuggetString(ps, "testIterations", it, testIterations)) {
        return emptyP;
    } else {
        p.testIterations = testIterations;
    }

    return p;
}

std::string openToken(const std::string & token) {
    std::string ps;
    ps.append("<");
    ps.append(token);
    ps.append(">");
    return ps;
}

std::string closeToken(const std::string & token) {
    std::string ps;
    ps.append("</");
    ps.append(token);
    ps.append(">");
    return ps;
}

}
