
#include <stdio.h>
#include <stdlib.h> 
#include "cJSON.h"

// функция для записи заголовков
void write_header(cJSON *json, FILE *file) { 
    cJSON *tmp = json; 
    while (tmp != NULL) { // жо тех пор пока не останется объект json
        fprintf(file, "%s,", tmp->string); 
        tmp = tmp->next; 
    } 
}


 // функция для записи основных данных
void write_data(cJSON *json, FILE *file) { 
    cJSON *tmp = json; 
    while (tmp != NULL) { 
        if (tmp->type == cJSON_String) {  // если тип элемента строка 
            fprintf(file, "%s", tmp->valuestring); //записываем строку в файл
        } 
        else if (tmp->type == cJSON_Number) {  // если тип поля число 
            double value = tmp->valuedouble; 
            fprintf(file, "%g", value); // записываем в файл
        } 
        fprintf(file, ","); // не забываем добавить разделитель
        tmp = tmp->next; 
    } 
    fprintf(file, "\n"); 
}

// функция help
void help (){
    puts("Exemple of input:\n\
        .\\main.exe data.json.\n");
}

int main(int argc, char *argv[]) { 

    // проверяем количество аргументов
    if (argc < 2 ){
        fprintf(stderr,"\nIncorect input!\n");
        help();
        exit(1);
    }

    FILE * file_json= fopen(argv[1], "r");
    // Проверяем на  открытие 
    if (file_json == NULL) {
        printf("Error: %s <filename>\n", argv[1]);
        return 1;
    }

    // определяем размер файл
    fseek(file_json, 0, SEEK_END);
    long fileSize = ftell(file_json);
    fseek(file_json, 0, SEEK_SET);

    // считываем файл в строку 
    char *jsonStr = (char *) malloc((fileSize + 1) * sizeof(char));
    fread(jsonStr, sizeof(char), fileSize, file_json);
    jsonStr[fileSize] = '\0';

    fclose(file_json);

    // создаем структуру json
    cJSON *root = cJSON_Parse(jsonStr);

    // проверяем на успех
    if (root == NULL) {
        printf("Json string conversion error.\n");
        free(jsonStr);
        return 1;
    }


    // Разбиваем на массив элементов (без использования структуры DataItem)
    int sizeArr = cJSON_GetArraySize(root); // получаем количество элементов массива 
    cJSON **items = (cJSON **) malloc(sizeArr * sizeof(cJSON *)); // выделяем память под размер массива
    // проходим по всем элементам массива и добавляем в массив элементов
    for (int i = 0; i < sizeArr; i++) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        items[i] = item;
    }

    // выходной файл
    FILE *file_csv = fopen("output.csv", "w");

    // проверка 
    if (file_csv == NULL) {
        printf("Error opennig the output file.\n");
        cJSON_Delete(root);
        free(items);
        free(jsonStr);
        return 1;
    }

    // записываем заголовки 
    write_header(items[0]->child, file_csv);
    fprintf(file_csv, "\n");

    // записываем поля
    for (int i = 0; i < sizeArr; i++) {
        write_data(items[i]->child, file_csv);
    }

    
    fclose(file_csv);

    printf("The data has been successfully converted to csv format.\n");

    // Очищаем память
    cJSON_Delete(root);
    free(items);
    free(jsonStr);

    return 0;
}


