#include "neuralpsostream.h"

namespace NeuralPsoStream {

std::string stringifyParticle(const Particle<NeuralNet::EdgeType> & p) {
    std::string particleString;

    // Particle Position X
    particleString.append("\t<_x>\n");
    particleString.append(stringifyEdges(p._x));
    particleString.append("\t</_x>\n");

    // Particle Velocity
    particleString.append("\t<_v>\n");
    particleString.append(stringifyEdges(p._v));
    particleString.append("\t</_v>\n");

    // Particle Personal Best X
    particleString.append("\t<_x_pb>\n");
    particleString.append(stringifyEdges(p._x_pb));
    particleString.append("\t</_x_pb>\n");

    // Particle Local Best
    particleString.append("\t<_x_lb>\n");
    particleString.append(stringifyEdges(p._x_lb));
    particleString.append("\t</_x_lb>\n");

    // Particle Personal Best Fitness
    particleString.append("\t<_fit_pb>\n\t\t");
    particleString.append(stringPut(p._fit_pb));
    particleString.append("\n\t</_fit_pb>\n");

    // Particle Local Best Fitness
    particleString.append("\t<_fit_lb>\n\t\t");
    particleString.append(stringPut(p._fit_lb));
    particleString.append("\n\t</_fit_lb>\n");

    int worstFlag = p._worstFlag ? 1 : 0;
    // Particle Worst Flag
    particleString.append("\t<_worstFlag>\n\t\t");
    particleString.append(stringPut(worstFlag));
    particleString.append("\n\t</_worstFlag>\n");

    // Particle Points
    particleString.append("\t<_points>\n\t\t");
    particleString.append(stringPut(p._points));
    particleString.append("\n\t</_points>\n");

    return particleString;
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
                if (k == edges[i][k].size() -1) {
                    particleString.append("\n");
                }
            }
            particleString.append("\t\t\t}\n");
        }
        particleString.append("\t\t}\n");
    }

    return particleString;
}

NeuralNet::EdgeType edgesFromString(const std::string & edgeString) {
    //!TEST!//

    NeuralNet::EdgeType e;

    int it = 0;
    while ( it < (int) edgeString.size()) {
        if (edgeString[it]  == '{') {
            it++;
            while (edgeString[it] != '}') {
                std::vector<std::vector<double>> innerNet;
                if (edgeString[it] == '{') {
                    it++;
                    while (edgeString[it] != '}') {
                        std::vector<double> leftNode;
                        int jt = it+1;
                        while (edgeString[jt] != '}') {
                            if (edgeString[jt] == ',' || edgeString[jt] == '}') {
                                std::string valString = edgeString.substr(it,jt-it);
                                double val = numberFromString<double>(valString);
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

Particle<NeuralNet::EdgeType> particleFromString(const std::string & particleString) {
    typedef Particle<NeuralNet::EdgeType> P;
    P pParticle;
    int it = 0;

    // Particle Position X
    if (!findNextToken(particleString, it)) {
        return P();
    }
    std::string xString = subStringByToken(particleString, "_x", it);
    if (!xString.empty()) {
        NeuralNet::EdgeType xEdge = edgesFromString(xString);
        pParticle._x = xEdge;
    }


    // Particle Velocity
    if (!findNextToken(particleString, it)) {
        return P();
    }
    std::string vString = subStringByToken(particleString, "_v", it);
    if (!vString.empty()) {
        NeuralNet::EdgeType vEdge = edgesFromString(vString);
        pParticle._v = vEdge;
    }

    // Particle Personal Best X
    if (!findNextToken(particleString, it)) {
        return P();
    }
    std::string xbString = subStringByToken(particleString, "_x_pb", it);
    if (!xbString.empty()) {
        NeuralNet::EdgeType xbEdge = edgesFromString(xbString);
        pParticle._x_pb = xbEdge;
    }

    // Particle Local Best
    if (!findNextToken(particleString, it)) {
        return P();
    }
    std::string xlString = subStringByToken(particleString, "_x_lb", it);
    if (!xlString.empty()) {
        NeuralNet::EdgeType xlEdge = edgesFromString(xlString);
        pParticle._x_lb = xlEdge;
    }

    // Particle Personal Best Fitness
    if (!findNextToken(particleString, it)) {
        return P();
    }
    char c = particleString[it];
    std::string fpbString = subStringByToken(particleString, "_fit_pb", it);
    if (!fpbString.empty()) {
        double fpb = numberFromString<double>(fpbString);
        pParticle._fit_pb = fpb;
    }

    // Particle Local Best Fitness
    if (!findNextToken(particleString, it)) {
        return P();
    }
    std::string flbString = subStringByToken(particleString, "_fit_lb", it);
    if (!flbString.empty()) {
        double flb = numberFromString<double>(flbString);
        pParticle._fit_lb = flb;
    }

    // Particle Worst Flag
    if (!findNextToken(particleString, it)) {
        return P();
    }
    std::string worstString = subStringByToken(particleString, "_worstFlag", it);
    if (!worstString.empty()) {
        int worst = numberFromString<int>(worstString);
        pParticle._worstFlag = worst == 1;
    }

    // Particle Points
    if (!findNextToken(particleString, it)) {
        return P();
    }
    std::string pointsString = subStringByToken(particleString, "_points", it);
    if (!pointsString.empty()) {
        double points = numberFromString<double>(fpbString);
        pParticle._points = points;
    }

    return pParticle;
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
        return false;
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

    }while (it < partSubString.size());

    return particles;
}

}
