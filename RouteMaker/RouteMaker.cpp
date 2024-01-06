#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iomanip>
#include <unordered_map>
#include <algorithm>

struct node {
    std::vector<size_t> index_to;
    std::vector<float> cost_to;
    std::string name;

    void push(size_t index, float cost) {
        index_to.push_back(index);
        cost_to.push_back(cost);
    }

    void pop(size_t index) {
        for (size_t i = 0; i < index_to.size(); i++) {
            if (index_to[i] == index) {
                index_to.erase(index_to.begin() + i);
                cost_to.erase(cost_to.begin() + i);
                break;
            }
        }
    }
};

std::vector<std::string> splitStringFromFile(const std::string& filename, char divider) {
    std::ifstream fin;
    std::string str;
    std::vector<std::string> strs;
    fin.open(filename);

    if (!fin.is_open()) {
        std::cout << "No\n";
        return strs;
    }

    while (!fin.eof()) {
        std::getline(fin, str, divider);
        if(str.size() >= 4)
            strs.push_back(str);
    }
    fin.close();

    return strs;
}

std::vector<std::string> outputUniqueNames(const std::vector<std::string>& table) {
    std::vector<std::string> stops;
    bool uniq;

    for (auto& row : table) {
        uniq = true;
        for (auto& elem : stops) {
            if (!(uniq = (row != elem)))
                break;
        }
        if (uniq)
            stops.push_back(row);
    }

    return stops;
}

std::vector<size_t> countElements(const std::string& str, char el) {
    std::vector<size_t> founded;

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == el)
            founded.push_back(i);
    }
    return founded;
}

std::vector<std::string> extractColomnsFromRow(std::string row) {
    std::vector<std::string> colomns;
    
    auto foundedPos = countElements(row, '\t');
    size_t accumulatePos = 0;

    for (auto& founded : foundedPos) {
        
        colomns.push_back(row.substr(0, founded - accumulatePos));
        row.erase(row.cbegin(), row.cbegin() + (founded + 1 - accumulatePos));
        accumulatePos += founded + 1;
    }

    colomns.push_back(row);
    return colomns;
}

std::unordered_map<std::string, size_t> extractUniqueFromListOfRow(const std::vector<std::string>& table) {
    std::unordered_map<std::string, size_t> names;
    char uniq = 3;

    std::string sep1;
    std::string sep2;
    for(size_t i = 0; i < table.size(); i++) {
        sep1 = table[i].substr(0, table[i].find('\t'));
        sep2 = table[i];
        sep2.erase(sep2.cbegin(), sep2.cbegin() + sep1.size()+1);
        sep2 = sep2.substr(0,sep2.find('\t'));

        uniq = 3;
        for (auto& name : names) {
            uniq &= (char)(name.first != sep1) | (char)((name.first != sep2) << 1); // res = and(res, byte[ not(cond1), not(cond2) ])
            if (uniq == 0)
                break;
        }

        if (uniq & 1)
            names[sep1] = names.size();
        if (uniq & 2)
            names[sep2] = names.size();
    }

    return names;
}

std::vector<node> createNodes(const std::string& filename, const std::vector<std::string>& names) {
    std::vector<node> nodes;
    node freshNode;

    nodes.reserve(names.size());
    for (auto& name : names) {
        freshNode.name = name;

        nodes.push_back(freshNode);
    }

    return nodes;
}

std::vector<node> unwrapFileNode(const std::string& filename) {
    auto fileContent = splitStringFromFile(filename, '\n');
    const auto names = splitStringFromFile("Stops "+filename, '\n');

    auto nodes = createNodes(filename, names);

    std::vector<std::string> colomns;

    size_t nodeIndex1;
    size_t nodeIndex2;

    for (auto iter = fileContent.begin(); iter != fileContent.end(); iter++) {
        colomns = extractColomnsFromRow(*iter);

        nodeIndex1 = std::stoul(colomns[0]);
        nodeIndex2 = std::stoul(colomns[1]);

        nodes[nodeIndex1].push(nodeIndex2, std::stof(colomns[2]));
        nodes[nodeIndex2].push(nodeIndex1, std::stof(colomns[2]));
    }

    return nodes;
}

void updateMatrixSizeLev(std::vector<std::vector<size_t>>& matrix, size_t& width, size_t& height, const size_t origSize, const size_t checkSize) {
    size_t i = 0;
    if (width < origSize) { // width module
        size_t prevWidth = width;
        width = origSize;
        matrix.resize(width + 1);
        for (i = 0; i < width + 1; i++) { // fill by height
            if (matrix[i].size() <= height)
                matrix[i].resize(height + 1);
            matrix[i].front() = i;
        }
    }
    if (height < checkSize) { // height module
        height = checkSize;
        for (i = 0; i < width + 1; i++) // fill by width
            matrix[i].resize(height + 1);
        for (i = 0; i < height + 1; i++)
            matrix.front()[i] = i;
    }
}

// Wagner–Fischer algorithm
size_t countErrorsInStringLev(const std::string& orig, const std::string& check) {
    // all static to not spend time on pushing temporary variables in stack and then poping, dynamic programming anyway
    static std::vector<std::vector<size_t>> matrix;
    static size_t width = 0;
    static size_t height = 0;
    static size_t x;
    static size_t y;
    static bool subst;

    updateMatrixSizeLev(matrix, width, height, orig.size(), check.size());

    for (x = 0; x < orig.size(); x++) {
        for (y = 0; y < check.size(); y++) {
            subst = orig[x] != check[y];
            matrix[x + 1][y + 1] = std::min(matrix[x][y + 1] + 1, std::min(matrix[x + 1][y] + 1, matrix[x][y] + subst));
        }
    }
    return matrix[x][y];
}

void initPathfinding(size_t countNodes, std::vector<float>& dist, std::vector<size_t>& prev, std::vector<bool>& expl, size_t dst) {
    dist.resize(countNodes);
    prev.resize(countNodes);
    expl.resize(countNodes);
    for (size_t i = 0; i < countNodes; i++) {
        dist[i] = INFINITY;
        prev[i] = -1;
        expl[i] = false;
    }
    dist[dst] = 0;
}

// Djikstra algorithm
std::pair<std::vector<size_t>, float> findPathTwoPoint(const std::vector<node>& nodes, size_t src, size_t dst) {
    std::vector<float> dist;
    std::vector<size_t> prev;
    std::vector<bool> expl;

    initPathfinding(nodes.size(), dist, prev, expl, dst);

    float leastValue = INFINITY;
    size_t indexLeast = dst;

    while (!expl[src]) {
        leastValue = INFINITY;
        for (size_t i = 0; i < dist.size(); i++) {
            if (leastValue > dist[i] && !expl[i]) {
                leastValue = dist[i];
                indexLeast = i;
            }
        }
        expl[indexLeast] = true;
        for (size_t i = 0; i < nodes[indexLeast].index_to.size(); i++) {
            if (dist[indexLeast] + nodes[indexLeast].cost_to[i] < dist[nodes[indexLeast].index_to[i]]) {
                dist[nodes[indexLeast].index_to[i]] = dist[indexLeast] + nodes[indexLeast].cost_to[i];
                prev[nodes[indexLeast].index_to[i]] = indexLeast;
            }
        }
    }

    // recreate path
    std::vector<size_t> path;
    path.push_back(indexLeast);
    do {
        indexLeast = prev[indexLeast];
        path.push_back(indexLeast);
    } while (indexLeast != dst);

    return std::make_pair(path, leastValue);
}

size_t findApproximatelyTypedWord(const std::vector<node>& nodes, const std::string& input) {
    size_t index = -1;
    size_t count = -1;
    
    size_t currentCount;

    for (size_t i = 0; i < nodes.size(); i++) {
        currentCount = countErrorsInStringLev(nodes[i].name, input);
        if (count > currentCount) {
            index = i;
            count = currentCount;
        }
        if (count == 0)
            return i;
    }

    return index;
}

int main()
{
    std::string inpfrom;
    std::string inpto;

    std::getline(std::cin >> std::ws, inpfrom);
    std::getline(std::cin >> std::ws, inpto);

    auto stopList = unwrapFileNode("Vienna.txt");
    auto path = findPathTwoPoint(stopList, findApproximatelyTypedWord(stopList, inpfrom), findApproximatelyTypedWord(stopList, inpto));

    std::cout << path.second << '\n';

    for (auto& stop : path.first) {
        std::cout << std::quoted(stopList[stop].name) << "\n";
    }
}
