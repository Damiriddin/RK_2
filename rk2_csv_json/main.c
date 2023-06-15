#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "cJSON.h"

#define MAX_DATA_SIZE 1000
#define MAX_KEYS_SIZE 256

// функция help
void help (){
    puts("Exemple of input:\n\
        .\\main.exe data.json.\n");
}


typedef struct {
    char **keys; // массив указателей на ключи
    int count; // количество ключей
} Keys;

bool read_keys(FILE *file, Keys *keys);
bool read_data(FILE *file, Keys *keys, cJSON *root);
bool write_json(const char *filename, cJSON *root);

int main(int argc, char *argv[]) {
    // проверка количества аргументов
    if (argc < 2) {
        printf("Error: %s <filename>\n", argv[1]);
        help();
        exit(1);
    }
    // проверка на открытие файла 
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", strerror(errno));

        exit(1);
    }

    Keys keys = { .keys = NULL, .count = 0 }; // инициализация структуры keys
    // проверка на успешное чтение 
    if (!read_keys(file, &keys)) {
        printf("Error reading keys from file: %s\n", strerror(errno));
        fclose(file);
        exit(1);
    }
    // создание корнвого элемента JSON дакумента 
    cJSON *root = cJSON_CreateArray();
    if (root == NULL) {  // проверка на успешное создание
        printf("Error creating JSON array: %s\n", cJSON_GetErrorPtr());
        fclose(file);
        exit(1);
    }

    // проверка на успешное чтение данных из файла 
    if (!read_data(file, &keys, root)) {
        printf("Error reading data from file: %s\n", strerror(errno));
        cJSON_Delete(root);
        fclose(file);
        exit(1);
    }

    fclose(file);
    // проверка на запись в json файл
    if (!write_json("output.json", root)) {
        printf("Error writing JSON to file: %s\n", strerror(errno));
        cJSON_Delete(root);
        exit(1);
    }

    cJSON_Delete(root);

    printf("Data written to output.json successfully.\n");

    return 0;
}
// функция для считывания ключей
bool read_keys(FILE *file, Keys *keys) {
    char line[MAX_DATA_SIZE]; // массив для считывани строки
    if (fgets(line, MAX_DATA_SIZE, file) == NULL) {
        return false;
    }

    char *token = strtok(line, ","); // разделяем на строки встретив запятую
    while (token != NULL) {
        if (keys->count >= MAX_KEYS_SIZE) { // проверка на количество ключей
            return false;
        }

        size_t len = strlen(token);
        if (len > 0 && token[len - 1] == '\n') { // если последний символ разделенной строки '\n' заменяем ее на '\0'
            token[len - 1] = '\0';
        }

        keys->keys = realloc(keys->keys, (keys->count + 1) * sizeof(char *));
        if (keys->keys == NULL) {
            return false;
        }

        keys->keys[keys->count] = strdup(token);// выделяем память и копируем туда стрку token
        // проверка успешного копирования
        if (keys->keys[keys->count] == NULL) {
            return false;
        }

        keys->count++; //увеличиваем колич ключей

        token = strtok(NULL, ","); // получение следующей строки
    }

    return true;
}

// функция для чтения днных 
bool read_data(FILE *file, Keys *keys, cJSON *root) {
    char line[MAX_DATA_SIZE]; 
    // считываем пока строки не закончатся
    while (fgets(line, MAX_DATA_SIZE, file) != NULL) {
        cJSON *item = cJSON_CreateObject();
        if (item == NULL) {
            return false;
        }

        char *token = strtok(line, ","); // ',' - разделитель
        for (int i = 0; token != NULL && i < keys->count; i++) {
            size_t len = strlen(token);
            while (len > 0 && isspace(token[len - 1])) {
                len--; // уменьшяем колич ключей
            }
            token[len] = '\0'; 

            cJSON *value = cJSON_CreateString(token); // создаем из token строку json
            if (value == NULL) { // проверка на успешное создание 
                cJSON_Delete(item);
                return false;
            }

            cJSON_AddItemToObject(item, keys->keys[i], value); // создание объекта  json

            token = strtok(NULL, ","); // следующий этап
        }

        cJSON_AddItemToArray(root, item);// добавляем сконструированный объеки в массив json
    }

    return true;
}
// функция для записи в json
bool write_json(const char *filename, cJSON *root) {
    // проверка откытия файла
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }
    // преобразуем json в строку 
    char *json_string = cJSON_Print(root);
    if (json_string == NULL) {// проверка на успешное преобразование
        fclose(file);
        return false;
    }
    // запись строки в файл
    fputs(json_string, file);
    fclose(file);
    free(json_string);

    return true;
}