#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

const int64_t INF = std::numeric_limits<int64_t>::max() / 4;

struct Edge {
    int from;
    int to;
    int64_t weight;
};

bool isValidVertex(int vertex, int vertex_count) {
    return vertex >= 1 && vertex <= vertex_count;
}

int readGraph(std::vector<Edge>& edges, int& vertex_count) {
    int edge_count;

    std::cout << "Введите количество вершин: ";
    if (!(std::cin >> vertex_count) || vertex_count <= 0) {
        std::cerr << "Ошибка: количество вершин должно быть положительным числом.\n";
        return 1;
    }

    std::cout << "Введите количество ребер: ";
    if (!(std::cin >> edge_count) || edge_count < 0) {
        std::cerr << "Ошибка: количество ребер не может быть отрицательным.\n";
        return 1;
    }

    edges.clear();
    edges.reserve(edge_count);

    std::cout << "Введите ребра в формате: <вершина_1> <вершина_2> <вес>\n";
    for (int i = 0; i < edge_count; i++) {
        int from;
        int to;
        int64_t weight;

        std::cout << "Ребро " << i + 1 << ": ";
        if (!(std::cin >> from >> to >> weight)) {
            std::cerr << "Ошибка: не удалось прочитать ребро.\n";
            edges.clear();
            return 1;
        }

        if (!isValidVertex(from, vertex_count) || !isValidVertex(to, vertex_count)) {
            std::cerr << "Ошибка: номера вершин должны быть в диапазоне от 1 до " << vertex_count << ".\n";
            edges.clear();
            return 1;
        }

        edges.push_back({from - 1, to - 1, weight});
    }

    return 0;
}

void printEdgeList(const std::vector<Edge>& edges) {
    if (edges.empty()) {
        std::cout << "Список ребер пуст.\n";
        return;
    }

    std::cout << "Список ребер неориентированного графа:\n";
    for (std::size_t i = 0; i < edges.size(); i++) {
        std::cout << i + 1 << ") " << edges[i].from + 1 << " -- " << edges[i].to + 1
                  << ", вес = " << edges[i].weight << "\n";
    }
}

std::vector<std::vector<int64_t>> buildDistanceMatrix(const std::vector<Edge>& edges, int vertex_count) {
    std::vector<std::vector<int64_t>> dist(vertex_count, std::vector<int64_t>(vertex_count, INF));

    for (int i = 0; i < vertex_count; i++) {
        dist[i][i] = 0;
    }

    for (const Edge& edge : edges) {
        if (edge.weight < dist[edge.from][edge.to]) {
            dist[edge.from][edge.to] = edge.weight;
            dist[edge.to][edge.from] = edge.weight;
        }
    }

    return dist;
}

void floydWarshall(std::vector<std::vector<int64_t>>& dist) {
    int vertex_count = static_cast<int>(dist.size());

    for (int k = 0; k < vertex_count; k++) {
        for (int i = 0; i < vertex_count; i++) {
            if (dist[i][k] == INF) {
                continue;
            }

            for (int j = 0; j < vertex_count; j++) {
                if (dist[k][j] == INF) {
                    continue;
                }

                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }
}

bool hasNegativeCycle(const std::vector<std::vector<int64_t>>& dist) {
    for (std::size_t i = 0; i < dist.size(); i++) {
        if (dist[i][i] < 0) {
            return true;
        }
    }

    return false;
}

void printDistanceMatrix(const std::vector<std::vector<int64_t>>& dist) {
    std::cout << "Матрица кратчайших расстояний:\n";
    std::cout << "      ";
    for (std::size_t i = 0; i < dist.size(); i++) {
        std::cout.width(8);
        std::cout << i + 1;
    }
    std::cout << "\n";

    for (std::size_t i = 0; i < dist.size(); i++) {
        std::cout.width(5);
        std::cout << i + 1 << " ";

        for (std::size_t j = 0; j < dist[i].size(); j++) {
            std::cout.width(8);
            if (dist[i][j] == INF) {
                std::cout << "INF";
            } else {
                std::cout << dist[i][j];
            }
        }
        std::cout << "\n";
    }
}

int main() {
    std::vector<Edge> edges;
    int vertex_count = 0;

    std::cout << "Алгоритм Флойда-Уоршелла для неориентированного графа\n";
    std::cout << "Граф хранится как список ребер. Вершины нумеруются с 1.\n\n";

    if (readGraph(edges, vertex_count) != 0) {
        return 1;
    }

    std::cout << "\n";
    printEdgeList(edges);

    std::vector<std::vector<int64_t>> dist = buildDistanceMatrix(edges, vertex_count);

    floydWarshall(dist);

    if (hasNegativeCycle(dist)) {
        std::cout << "\nВ графе найден отрицательный цикл. Кратчайшие расстояния некорректны.\n";
    } else {
        std::cout << "\n";
        printDistanceMatrix(dist);
    }

    return 0;
}
