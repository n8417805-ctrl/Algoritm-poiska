#define _CRT_SECURE_NO_WARNINGS
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define WALL '#'
#define PATH ' '
#define START 'O'
#define END 'X'
#define SOLUTION '*'

typedef struct {
    int row;
    int col;
} Point;

typedef struct {
    Point* data;
    int front;
    int rear;
    int capacity;
} Queue;

typedef struct {
    Point* data;
    int top;
    int capacity;
} Stack;

Queue* createQueue(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->data = (Point*)malloc(capacity * sizeof(Point));
    q->front = 0;
    q->rear = 0;
    q->capacity = capacity;
    return q;
}

void freeQueue(Queue* q) {
    free(q->data);
    free(q);
}

bool isEmptyQueue(Queue* q) {
    return q->front == q->rear;
}

void enqueue(Queue* q, Point p) {
    if (q->rear >= q->capacity) {
        q->capacity *= 2;
        q->data = (Point*)realloc(q->data, q->capacity * sizeof(Point));
    }
    q->data[q->rear] = p;
    q->rear++;
}

Point dequeue(Queue* q) {
    Point p = q->data[q->front];
    q->front++;
    if (q->front == q->rear) {
        q->front = q->rear = 0;
    }
    return p;
}

Stack* createStack(int capacity) {
    Stack* s = (Stack*)malloc(sizeof(Stack));
    s->data = (Point*)malloc(capacity * sizeof(Point));
    s->top = -1;
    s->capacity = capacity;
    return s;
}

void freeStack(Stack* s) {
    free(s->data);
    free(s);
}

bool isEmptyStack(Stack* s) {
    return s->top == -1;
}

void push(Stack* s, Point p) {
    if (s->top >= s->capacity - 1) {
        s->capacity *= 2;
        s->data = (Point*)realloc(s->data, s->capacity * sizeof(Point));
    }
    s->top++;
    s->data[s->top] = p;
}

Point pop(Stack* s) {
    Point p = s->data[s->top];
    s->top--;
    return p;
}

char** createMaze(int rows, int cols) {
    char** maze = (char**)malloc(rows * sizeof(char*));
    for (int i = 0; i < rows; i++) {
        maze[i] = (char*)malloc(cols * sizeof(char));
    }
    return maze;
}

void freeMaze(char** maze, int rows) {
    for (int i = 0; i < rows; i++) {
        free(maze[i]);
    }
    free(maze);
}

bool loadMazeFromFile(char*** maze, int* rows, int* cols, Point* start, Point* end, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Ошибка: не удалось открыть файл %s\n", filename);
        return false;
    }

    if (fscanf(file, "%d %d", rows, cols) != 2) {
        printf("Ошибка: неверный формат файла!\n");
        fclose(file);
        return false;
    }

    fgetc(file);

    if (*rows < 3 || *rows > 100 || *cols < 3 || *cols > 100) {
        printf("Ошибка: недопустимые размеры лабиринта (%d x %d)!\n", *rows, *cols);
        fclose(file);
        return false;
    }

    *maze = createMaze(*rows, *cols);

    bool startFound = false;
    bool endFound = false;

    for (int i = 0; i < *rows; i++) {
        char line[256];
        if (fgets(line, sizeof(line), file) == NULL) {
            printf("Ошибка: не удалось прочитать строку %d!\n", i);
            freeMaze(*maze, *rows);
            fclose(file);
            return false;
        }

        line[strcspn(line, "\n")] = 0;

        if (strlen(line) != *cols) {
            printf("Ошибка: длина строки %d не соответствует объявленной ширине!\n", i);
            freeMaze(*maze, *rows);
            fclose(file);
            return false;
        }

        for (int j = 0; j < *cols; j++) {
            (*maze)[i][j] = line[j];

            if (line[j] == START) {
                start->row = i;
                start->col = j;
                startFound = true;
            }
            else if (line[j] == END) {
                end->row = i;
                end->col = j;
                endFound = true;
            }
            else if (line[j] != WALL && line[j] != PATH && line[j] != SOLUTION) {
                printf("Предупреждение: недопустимый символ '%c' в позиции (%d,%d)\n",
                    line[j], i, j);
                (*maze)[i][j] = PATH;
            }
        }
    }

    fclose(file);

    if (!startFound) 
    {
        if ((*maze)[0][0] == PATH) 
        {
            (*maze)[0][0] = START;
            start->row = 0;
            start->col = 0;
            startFound = true;
            printf("Старт утановлен в левый верхний угол (0,0)\n");
        }
        else 
        {
            bool foundStart = false;
            for (int i = 0; i < *rows && !foundStart; i++) {
                for (int j = 0; j < *cols && !foundStart; j++) {
                    if ((*maze)[i][j] == PATH) {
                        (*maze)[i][j] = START;
                        start->row = i;
                        start->col = j;
                        startFound = true;
                        foundStart = true;
                        printf("Старт установлен в (%d,%d)\n", i, j);
                    }
                }
            }
        }
    }

    if (!endFound) {
        // Если в файле нет конца, ставим его в правый нижний угол
        int lastRow = *rows - 1;
        int lastCol = *cols - 1;
        if ((*maze)[lastRow][lastCol] == PATH) {
            (*maze)[lastRow][lastCol] = END;
            end->row = lastRow;
            end->col = lastCol;
            endFound = true;
            printf("Автоматически установлен выход в правый нижний угол (%d,%d)\n", lastRow, lastCol);
        }
        else {
            // Если правый нижний угол - стена, ищем ближайшую свободную клетку
            bool foundEnd = false;
            for (int i = *rows - 1; i >= 0 && !foundEnd; i--) {
                for (int j = *cols - 1; j >= 0 && !foundEnd; j--) {
                    if ((*maze)[i][j] == PATH && (i != start->row || j != start->col)) {
                        (*maze)[i][j] = END;
                        end->row = i;
                        end->col = j;
                        endFound = true;
                        foundEnd = true;
                        printf("Автоматически установлен выход в (%d,%d)\n", i, j);
                    }
                }
            }
        }
    }

    if (!startFound || !endFound) {
        printf("Ошибка: в лабиринте не найдены старт (S) и/или выход (E)!\n");
        if (!startFound) printf("- Старт (S) ненайден\n");
        if (!endFound) printf("- Выход (E) ненайден\n");
        freeMaze(*maze, *rows);
        return false;
    }

    printf("Лабиринт успешно загружен из файла %s\n", filename);
    printf("Размер: %d x %d\n", *rows, *cols);
    printf("Старт: (%d, %d)\n", start->row, start->col);
    printf("Выход: (%d, %d)\n", end->row, end->col);

    return true;
}

void generateMaze(char** maze, int rows, int cols, Point* start, Point* end) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (i == 0 || i == rows - 1 || j == 0 || j == cols - 1) {
                maze[i][j] = WALL;
            }
            else {
                maze[i][j] = PATH;
            }
        }
    }

    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            if (i % 2 == 0 && j % 2 == 0) {
                maze[i][j] = WALL;
            }
        }
    }

    for (int i = 2; i < rows - 1; i += 2) {
        for (int j = 2; j < cols - 1; j += 2) {
            int direction = rand() % 4;
            int wallRow = i, wallCol = j;

            switch (direction) {
            case 0: wallRow = i - 1; break;
            case 1: wallRow = i + 1; break;
            case 2: wallCol = j - 1; break;
            case 3: wallCol = j + 1; break;
            }

            if (wallRow > 0 && wallRow < rows - 1 &&
                wallCol> 0 && wallCol < cols - 1) {
                maze[wallRow][wallCol] = WALL;
            }
        }
    }

    // Старт всегда в левом верхнем углу (если это не стена)
    start->row = 1;
    start->col = 1;
    if (maze[start->row][start->col] == WALL) {
        maze[start->row][start->col] = START;
    }
    else {
        maze[start->row][start->col] = START;
    }

    end->row = rows - 2;
    end->col = cols - 2;
    if (maze[end->row][end->col] == WALL) {
        maze[end->row][end->col] = END;
    }
    else {
        maze[end->row][end->col] = END;
    }

    // Гарантируем, что старт и конец не находятся в одной клетке
    if (start->row == end->row && start->col == end->col) {
        if (end->row > 1) {
            end->row--;
            maze[end->row][end->col] = END;
        }
        else if (end->col > 1) {
            end->col--;
            maze[end->row][end->col] = END;
        }
    }

    int deadEndPaths = 0;
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            if (maze[i][j] == PATH) {
                int walls = 0;
                if (maze[i - 1][j] == WALL) walls++;
                if (maze[i + 1][j] == WALL) walls++;
                if (maze[i][j - 1] == WALL) walls++;
                if (maze[i][j + 1] == WALL) walls++;

                if (walls >= 3) {
                    deadEndPaths++;
                }
            }
        }
    }

    if (deadEndPaths < 5) {
        for (int i = 0; i < 10; i++) {
            int r = 1 + rand() % (rows - 2);
            int c = 1 + rand() % (cols - 2);

            if (maze[r][c] == WALL) {
                int neighborPaths = 0;
                if (maze[r - 1][c] != WALL) neighborPaths++;
                if (maze[r + 1][c] != WALL) neighborPaths++;
                if (maze[r][c - 1] != WALL) neighborPaths++;
                if (maze[r][c + 1] != WALL) neighborPaths++;

                if (neighborPaths >= 2) {
                    maze[r][c] = PATH;
                }
            }
        }
    }
}

bool isValid(int row, int col, int rows, int cols, char** maze, bool** visited) {
    return (row >= 0 && row < rows && col >= 0 && col < cols &&
        maze[row][col] != WALL && !visited[row][col]);
}


//ЦВЕТА
//30 - черный
//31 - красный
//32 - зеленый
//33 - желтый
//34 - синий
//35 - пурпул
//36 - голубой
//37 - белый

void printMaze(char** maze, int rows, int cols)
{
    printf("\n   ");
    for (int j = 0; j < cols; j++) 
    {
        printf("%2d", j % 100);
    }
    printf("\n");

    for (int i = 0; i < rows; i++) 
    {
        printf("%2d ", i % 100);
        for (int j = 0; j < cols; j++)
        {
            if (maze[i][j] == START) 
            {
                printf("\033[1;32m%c \033[0m", START);
            }
            else if (maze[i][j] == END)
            {
                printf("\033[1;31m%c \033[0m", END);
            }
            else if (maze[i][j] == WALL) 
            {
                printf("\033[1;34m%c \033[0m", WALL);
            }
            else
            {
                printf("%c ", maze[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

bool bfsFindPath(char** maze, int rows, int cols, Point start, Point end) {
    bool** visited = (bool**)malloc(rows * sizeof(bool*));
    Point** parent = (Point**)malloc(rows * sizeof(Point*));

    for (int i = 0; i < rows; i++) {
        visited[i] = (bool*)malloc(cols * sizeof(bool));
        parent[i] = (Point*)malloc(cols * sizeof(Point));
        for (int j = 0; j < cols; j++) {
            visited[i][j] = false;
            parent[i][j].row = -1;
            parent[i][j].col = -1;
        }
    }

    Queue* q = createQueue(rows * cols);

    visited[start.row][start.col] = true;
    enqueue(q, start);

    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };

    bool found = false;

    while (!isEmptyQueue(q)) {
        Point current = dequeue(q);

        if (current.row == end.row && current.col == end.col) {
            Point p = end;
            while (p.row != start.row || p.col != start.col) {
                if (maze[p.row][p.col] != START && maze[p.row][p.col] != END) {
                    maze[p.row][p.col] = SOLUTION;
                }
                p = parent[p.row][p.col];
            }
            found = true;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int newRow = current.row + dr[i];
            int newCol = current.col + dc[i];

            if (isValid(newRow, newCol, rows, cols, maze, visited)) {
                visited[newRow][newCol] = true;
                parent[newRow][newCol] = current;

                Point newPoint;
                newPoint.row = newRow;
                newPoint.col = newCol;
                enqueue(q, newPoint);
            }
        }
    }

    for (int i = 0; i < rows; i++) {
        free(visited[i]);
        free(parent[i]);
    }
    free(visited);
    free(parent);
    freeQueue(q);

    return found;
}

bool dfsFindPath(char** maze, int rows, int cols, Point start, Point end) {
    bool** visited = (bool**)malloc(rows * sizeof(bool*));
    Point** parent = (Point**)malloc(rows * sizeof(Point*));

    for (int i = 0; i < rows; i++) {
        visited[i] = (bool*)malloc(cols * sizeof(bool));
        parent[i] = (Point*)malloc(cols * sizeof(Point));
        for (int j = 0; j < cols; j++) {
            visited[i][j] = false;
            parent[i][j].row = -1;
            parent[i][j].col = -1;
        }
    }

    Stack* s = createStack(rows * cols);

    visited[start.row][start.col] = true;
    push(s, start);

    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };

    bool found = false;

    while (!isEmptyStack(s)) {
        Point current = pop(s);

        if (current.row == end.row && current.col == end.col) {
            Point p = end;
            while (p.row != start.row || p.col != start.col) {
                if (maze[p.row][p.col] != START && maze[p.row][p.col] != END) {
                    maze[p.row][p.col] = SOLUTION;
                }
                p = parent[p.row][p.col];
            }
            found = true;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int newRow = current.row + dr[i];
            int newCol = current.col + dc[i];

            if (isValid(newRow, newCol, rows, cols, maze, visited)) {
                visited[newRow][newCol] = true;
                parent[newRow][newCol] = current;

                Point newPoint;
                newPoint.row = newRow;
                newPoint.col = newCol;
                push(s, newPoint);
            }
        }
    }

    for (int i = 0; i < rows; i++) {
        free(visited[i]);
        free(parent[i]);
    }
    free(visited);
    free(parent);
    freeStack(s);

    return found;
}

void clearSolution(char** maze, int rows, int cols, Point start, Point end) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (maze[i][j] == SOLUTION) {
                maze[i][j] = PATH;
            }
        }
    }
    maze[start.row][start.col] = START;
    maze[end.row][end.col] = END;
}

int getIntInput(const char* prompt, int min, int max) {
    int value;
    char input[100];
    int success = 0;

    while (!success) {
        printf("%s", prompt);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Ошибка ввода!\n");
            continue;
        }

        if (input[0] == '\n') {
            printf("Ошибка: введите число!\n");
            continue;
        }

        char* endptr;
        value = strtol(input, &endptr, 10);

        if (*endptr != '\n' && *endptr != '\0') {
            while (*endptr != '\0' && isspace(*endptr)) {
                endptr++;
            }
            if (*endptr != '\0') {
                printf("Ошибка: введите целое число!\n");
                continue;
            }
        }

        if (value < min || value > max) {
            printf("Значение должно быть от %d до %d.\n", min, max);
            continue;
        }

        success = 1;
    }

    return value;
}

int getMenuChoice() {
    int choice;
    char input[100];
    int success = 0;

    while (!success) {
        printf("Выберите действие: ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Ошибка ввода!\n");
            continue;
        }

        if (input[0] == '\n') {
            printf("Ошибка: введите число!\n");
            continue;
        }

        char* endptr;
        choice = strtol(input, &endptr, 10);

        if (*endptr != '\n' && *endptr != '\0') {
            while (*endptr != '\0' && isspace(*endptr)) {
                endptr++;
            }
            if (*endptr != '\0') {
                printf("Ошибка: введите целое число!\n");
                continue;
            }
        }

        success = 1;
    }

    return choice;
}

int getAlgorithmChoice() {
    int choice;
    char input[100];
    int success = 0;

    while (!success) {
        printf("Выберите алгоритм поиска (1 - BFS, 2 - DFS): ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Ошибка ввода!\n");
            continue;
        }

        if (input[0] == '\n') {
            printf("Ошибка: введите число!\n");
            continue;
        }

        char* endptr;
        choice = strtol(input, &endptr, 10);

        if (*endptr != '\n' && *endptr != '\0') {
            while (*endptr != '\0' && isspace(*endptr)) {
                endptr++;
            }
            if (*endptr != '\0') {
                printf("Ошибка: введите целое число!\n");
                continue;
            }
        }

        if (choice < 1 || choice > 2) {
            printf("Выберите 1 или 2!\n");
            continue;
        }

        success = 1;
    }

    return choice;
}

void saveMazeToFile(char** maze, int rows, int cols, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Ошибка при сохранении файла!\n");
        return;
    }

    fprintf(file, "%d %d\n", rows, cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(file, "%c", maze[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
    printf("Лабиринт сохранен в файл: %s\n", filename);
}

bool getFilename(char* filename, int max_len, const char* prompt) {
    printf("%s", prompt);

    if (fgets(filename, max_len, stdin) == NULL) {
        printf("Ошибка ввода!\n");
        return false;
    }

    filename[strcspn(filename, "\n")] = 0;

    if (strlen(filename) == 0) {
        printf("Ошибка: имя файла не может быть пустым!\n");
        return false;
    }

    return true;
}

int main() {
    setlocale(LC_ALL, "");
    srand(time(NULL));

    int rows = 0, cols = 0;
    char** maze = NULL;
    Point start, end;
    bool mazeLoaded = false;

    printf("=== АЛГОРИТМ ПОИСКА ПУТИ В ЛАБИРИНТЕ ===\n\n");

    printf("Выберите способ создания лабиринта:\n");
    printf("1. Сгенерировать случайный лабиринт\n");
    printf("2. Загрузить лабиринт из файла\n");

    int initialChoice = getIntInput("Выберите вариант (1-2): ", 1, 2);

    if (initialChoice == 1) {
        rows = getIntInput("Введите количество строк (7-100): ", 7, 100);
        if (rows % 2 == 0) rows++;

        cols = getIntInput("Введите количество столбцов (7-100): ", 7, 100);
        if (cols % 2 == 0) cols++;

        maze = createMaze(rows, cols);
        generateMaze(maze, rows, cols, &start, &end);
        mazeLoaded = true;

        printf("\nПараметры лабиринта:\n");
        printf("- Размер: %d x %d\n", rows, cols);
        printf("- Алгоритм генерации: Простой алгоритм\n");
        printf("- Старт: (%d, %d)\n", start.row, start.col);
        printf("- Выход: (%d, %d)\n", end.row, end.col);
    }
    else {
        char filename[256];
        while (!mazeLoaded) {
            if (!getFilename(filename, sizeof(filename), "Введите имя файла с лабиринтом: ")) {
                continue;
            }

            mazeLoaded = loadMazeFromFile(&maze, &rows, &cols, &start, &end, filename);
        }
    }

    printf("\nЛегенда:\n");
    printf("O - старт\n");
    printf("X - выход\n");
    printf("# - стены\n");
    printf("* - путь решения\n");
    printf("\nТекущий лабиринт:\n");
    printMaze(maze, rows, cols);

    int choice;
    do {
        printf("\nМЕНЮ:\n");
        printf("1. Найти путь\n");
        printf("2. Сгенерировать новый лабиринт\n");
        printf("3. Загрузить лабиринт из файла\n");
        printf("4. Изменить параметры\n");
        printf("5. Очистить решение\n");
        printf("6. Сохранить лабиринт в файл\n");
        printf("7. Выход\n");

        choice = getMenuChoice();

        switch (choice) {
        case 1: {
            if (!mazeLoaded) {
                printf("Сначала загрузите или сгенерируйте лабиринт!\n");
                break;
            }

            int algorithm = getAlgorithmChoice();
            clearSolution(maze, rows, cols, start, end);

            bool found;
            clock_t start_time = clock();

            if (algorithm == 1)
            {
                printf("\nПоиск пути с использованием BFS (поиск в ширину)...\n");
                printf("Из (%d,%d) в (%d,%d)\n", start.row, start.col, end.row, end.col);
                found = bfsFindPath(maze, rows, cols, start, end);
            }
            else {
                printf("\nПоиск пути с использованием DFS (поиск в глубину)...\n");
                printf("Из (%d,%d) в (%d,%d)\n", start.row, start.col, end.row, end.col);
                found = dfsFindPath(maze, rows, cols, start, end);
            }

            clock_t end_time = clock();
            double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

            if (found) {
                printf("\nПуть найден за %.4f секунд! (* - путь решения):\n", time_taken);
                printf("Алгоритм: %s\n", algorithm == 1 ? "BFS (поиск в ширину)" : "DFS (поиск в глубину)");
                printf("Старт: (%d,%d), Выход: (%d,%d)\n", start.row, start.col, end.row, end.col);
                printMaze(maze, rows, cols);
            }
            else {
                printf("\nПуть не найден за %.4f секунд!\n", time_taken);
                printf("Старт: (%d,%d), Выход: (%d,%d)\n", start.row, start.col, end.row, end.col);
            }
            break;
        }

        case 2:
            if (maze) freeMaze(maze, rows);

            rows = getIntInput("Введите количество строк (7-100): ", 7, 100);
            if (rows % 2 == 0) rows++;

            cols = getIntInput("Введите количество столбцов (7-100): ", 7, 100);
            if (cols % 2 == 0) cols++;

            maze = createMaze(rows, cols);
            generateMaze(maze, rows, cols, &start, &end);
            mazeLoaded = true;

            printf("\nНовый лабиринт сгенерирован:\n");
            printf("Старт: (%d, %d)\n", start.row, start.col);
            printf("Выход: (%d, %d)\n", end.row, end.col);
            printMaze(maze, rows, cols);
            break;

        case 3:
            if (maze) freeMaze(maze, rows);

            char filename[256];
            if (!getFilename(filename, sizeof(filename), "Введите имя файла с лабиринтом: ")) {
                mazeLoaded = false;
                break;
            }

            mazeLoaded = loadMazeFromFile(&maze, &rows, &cols, &start, &end, filename);
            if (mazeLoaded) {
                printf("\nЛабиринт загружен:\n");
                printf("Старт: (%d,%d), Выход: (%d,%d)\n", start.row, start.col, end.row, end.col);
                printMaze(maze, rows, cols);
            }
            else {
                printf("\nНе удалось загрузить лабиринт.\n");
            }
            break;

        case 4:
            if (!mazeLoaded) {
                printf("Сначала загрузите или сгенерируйте лабиринт!\n");
                break;
            }

            if (initialChoice == 2) {
                printf("Изменение параметров доступно только для сгенерированных лабиринтов!\n");
                printf("Для изменения загруженного лабиринта используйте пункт меню 3.\n");
                break;
            }

            if (maze) freeMaze(maze, rows);

            rows = getIntInput("Введите количество строк (7-100): ", 7, 100);
            if (rows % 2 == 0) rows++;

            cols = getIntInput("Введите количество столбцов (7-100): ", 7, 100);
            if (cols % 2 == 0) cols++;

            maze = createMaze(rows, cols);
            generateMaze(maze, rows, cols, &start, &end);

            printf("\nНовые параметры лабиринта:\n");
            printf("- Размер: %d x %d\n", rows, cols);
            printf("- Алгоритм генерации: Простой алгоритм\n");
            printf("- Старт: (%d, %d)\n", start.row, start.col);
            printf("- Выход: (%d, %d)\n", end.row, end.col);
            printf("\nНовый лабиринт:\n");
            printMaze(maze, rows, cols);
            break;

        case 5:
            if (!mazeLoaded) {
                printf("Сначала загрузите или сгенерируйте лабиринт!\n");
                break;
            }

            clearSolution(maze, rows, cols, start, end);
            printf("\nРешение очищено:\n");
            printf("Старт: (%d,%d), Выход: (%d,%d)\n", start.row, start.col, end.row, end.col);
            printMaze(maze, rows, cols);
            break;

        case 6:
            if (!mazeLoaded) {
                printf("Сначала загрузите или сгенерируйте лабиринт!\n");
                break;
            }

            char saveFilename[256];
            if (getFilename(saveFilename, sizeof(saveFilename), "Введите имя файла для сохранения: ")) {
                saveMazeToFile(maze, rows, cols, saveFilename);
            }
            break;

        case 7:
            printf("Выход из программы.\n");
            break;

        default:
            printf("Неверный выбор. Попробуйте снова.\n");
        }
    } while (choice != 7);

    if (mazeLoaded) {
        freeMaze(maze, rows);
    }

    return 0;
}