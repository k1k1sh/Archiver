#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <termio.h>
#include <unistd.h>


#include "../archiver.h"

void pack(char* dir_path, char* archive_path) {
    int archive;      // Дескриптор архива
    int root_dir_len; // Длина адреса директории

    // Проверка на существования пути архива
    if (access(archive_path, F_OK) != -1) {
        // Если уже существует
        printf("Ошибка. %s уже существует.\n", archive_path);
        exit(1);

    }

    // Создание выходного архива
    archive = open(archive_path, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if (archive == -1) {
        printf("Ошибка. %s не может быть создан.\n", archive_path);
        exit(1);
    }

    // Запись в архив мета данных
    if (write(archive, META, META_LENGTH) != META_LENGTH) {
        printf("Ошибка. Запись мета данных в %s не возможна.\n", archive_path);
        exit(1);
    }

    // Запись длины названия директории
    root_dir_len = strlen(dir_path);
    if (write(archive, &root_dir_len, sizeof(int)) != sizeof(int)) {
        printf("Ошибка. Невозможна запись длины названия директории в %s.\n", archive_path);
        exit(1);
    }

    // Запаковка данных в архив
    _pack(archive, dir_path);

    // Закрытие итогового архива
    if (close(archive) == -1) {
        printf("Ошибка. Невозможно закрыть %s.\n", archive_path);
        exit(1);
    }
}