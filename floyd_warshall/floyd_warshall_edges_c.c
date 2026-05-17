#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define INF (INT64_MAX / 4)

typedef struct {
    int from;
    int to;
    int64_t weight;
} Edge;

int64_t **createDistanceMatrix(int vertex_count) {
    int64_t **dist = (int64_t **)malloc((size_t)vertex_count * sizeof(int64_t *));
    if (dist == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память для матрицы расстояний.\n");
        return NULL;
    }

    for (int i = 0; i < vertex_count; i++) {
        dist[i] = (int64_t *)malloc((size_t)vertex_count * sizeof(int64_t));
        if (dist[i] == NULL) {
            fprintf(stderr, "Ошибка: не удалось выделить память для строки матрицы.\n");
            for (int j = 0; j < i; j++) {
                free(dist[j]);
            }
            free(dist);
            return NULL;
        }
    }

    for (int i = 0; i < vertex_count; i++) {
        for (int j = 0; j < vertex_count; j++) {
            dist[i][j] = (i == j) ? 0 : INF;
        }
    }

    return dist;
}

void freeDistanceMatrix(int64_t **dist, int vertex_count) {
    if (dist == NULL) {
        return;
    }

    for (int i = 0; i < vertex_count; i++) {
        free(dist[i]);
    }
    free(dist);
}

int isValidVertex(int vertex, int vertex_count) {
    return vertex >= 1 && vertex <= vertex_count;
}

int readGraph(Edge **edges, int *vertex_count, int *edge_count) {
    if (edges == NULL || vertex_count == NULL || edge_count == NULL) {
        fprintf(stderr, "Ошибка: передан некорректный аргумент.\n");
        return 1;
    }

    printf("Введите количество вершин: ");
    if (scanf("%d", vertex_count) != 1 || *vertex_count <= 0) {
        fprintf(stderr, "Ошибка: количество вершин должно быть положительным числом.\n");
        return 1;
    }

    printf("Введите количество ребер: ");
    if (scanf("%d", edge_count) != 1 || *edge_count < 0) {
        fprintf(stderr, "Ошибка: количество ребер не может быть отрицательным.\n");
        return 1;
    }

    if (*edge_count == 0) {
        *edges = NULL;
        return 0;
    }

    *edges = (Edge *)malloc((size_t)(*edge_count) * sizeof(Edge));
    if (*edges == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память для списка ребер.\n");
        return 1;
    }

    printf("Введите ребра в формате: <вершина_1> <вершина_2> <вес>\n");
    for (int i = 0; i < *edge_count; i++) {
        int from;
        int to;
        int64_t weight;

        printf("Ребро %d: ", i + 1);
        if (scanf("%d %d %" SCNd64, &from, &to, &weight) != 3) {
            fprintf(stderr, "Ошибка: не удалось прочитать ребро.\n");
            free(*edges);
            *edges = NULL;
            return 1;
        }

        if (!isValidVertex(from, *vertex_count) || !isValidVertex(to, *vertex_count)) {
            fprintf(stderr, "Ошибка: номера вершин должны быть в диапазоне от 1 до %d.\n", *vertex_count);
            free(*edges);
            *edges = NULL;
            return 1;
        }

        (*edges)[i].from = from - 1;
        (*edges)[i].to = to - 1;
        (*edges)[i].weight = weight;
    }

    return 0;
}

void printEdgeList(const Edge *edges, int edge_count) {
    if (edge_count == 0) {
        printf("Список ребер пуст.\n");
        return;
    }

    printf("Список ребер неориентированного графа:\n");
    for (int i = 0; i < edge_count; i++) {
        printf("%d) %d -- %d, вес = %" PRId64 "\n",
               i + 1,
               edges[i].from + 1,
               edges[i].to + 1,
               edges[i].weight);
    }
}

int64_t **buildDistanceMatrix(const Edge *edges, int vertex_count, int edge_count) {
    int64_t **dist = createDistanceMatrix(vertex_count);
    if (dist == NULL) {
        return NULL;
    }

    for (int i = 0; i < edge_count; i++) {
        int from = edges[i].from;
        int to = edges[i].to;
        int64_t weight = edges[i].weight;

        if (weight < dist[from][to]) {
            dist[from][to] = weight;
            dist[to][from] = weight;
        }
    }

    return dist;
}

void floydWarshall(int64_t **dist, int vertex_count) {
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

int hasNegativeCycle(int64_t **dist, int vertex_count) {
    for (int i = 0; i < vertex_count; i++) {
        if (dist[i][i] < 0) {
            return 1;
        }
    }

    return 0;
}

void printDistanceMatrix(int64_t **dist, int vertex_count) {
    printf("Матрица кратчайших расстояний:\n");
    printf("      ");
    for (int i = 0; i < vertex_count; i++) {
        printf("%8d", i + 1);
    }
    printf("\n");

    for (int i = 0; i < vertex_count; i++) {
        printf("%5d ", i + 1);
        for (int j = 0; j < vertex_count; j++) {
            if (dist[i][j] == INF) {
                printf("%8s", "INF");
            } else {
                printf("%8" PRId64, dist[i][j]);
            }
        }
        printf("\n");
    }
}

int main(void) {
    Edge *edges = NULL;
    int64_t **dist = NULL;
    int vertex_count = 0;
    int edge_count = 0;

    printf("Алгоритм Флойда-Уоршелла для неориентированного графа\n");
    printf("Граф хранится как список ребер. Вершины нумеруются с 1.\n\n");

    if (readGraph(&edges, &vertex_count, &edge_count) != 0) {
        return 1;
    }

    printf("\n");
    printEdgeList(edges, edge_count);

    dist = buildDistanceMatrix(edges, vertex_count, edge_count);
    if (dist == NULL) {
        free(edges);
        return 1;
    }

    floydWarshall(dist, vertex_count);

    if (hasNegativeCycle(dist, vertex_count)) {
        printf("\nВ графе найден отрицательный цикл. Кратчайшие расстояния некорректны.\n");
    } else {
        printf("\n");
        printDistanceMatrix(dist, vertex_count);
    }

    freeDistanceMatrix(dist, vertex_count);
    free(edges);

    return 0;
}
