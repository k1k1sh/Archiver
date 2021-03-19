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

void _pack(int archive, char* src_path) {
    // Открытие директории
    DIR *dir = opendir(src_path);

    // Если путь к директории - это путь к файлу
    if (dir == NULL) {
        printf("Ошибка %s - не директория.\n", src_path);

        // Пытаемся закрыть архив
        if (close(archive) == -1) {
            printf("Ошибка. Невозможно закрыть архив.\n");
        }

        exit(1);
    }

    // Упаковываем информацию об архиве
    _pack_info(archive, FOLDER_NAME, src_path);

    // Получаем следующую запись в каталоге
    struct dirent *entry;

    // Пока существует следующий файл в директории
    while ((entry = readdir(dir)) != NULL) {
        // Пропускаем вхождение . и ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Получем путь директории
        char file_path[MAX_PATH_LENGTH] = {0};
        snprintf(file_path, sizeof(file_path), "%s/%s", src_path, entry->d_name);
        _remove_extra_slash(file_path);

        if (entry->d_type == DT_DIR) {
            // Если это папка
            _pack(archive, file_path);
        }
        else {
            // Записываем информацию
            _pack_info(archive, FILE_NAME, file_path);

            // Записываем содержимое
            _pack_content(archive, file_path);

            // Эхо печать в консоль
            printf("Записно в архив %s\n", file_path);
        }
    }

    // Закрываем директорию
    if (closedir(dir) == -1) {
        printf("Ошибка. Невозможно закрыть %s.\n", src_path);
        exit(1);
    }
}

void _pack_info(int fd, path_type p_type, char* path) {

    // Флаг ошибки
    int error = 1;

    do {


        // Записываем длину пути
        int file_path_len = strlen(path) + 1;
        if (write(fd, &file_path_len, sizeof(int)) != sizeof(int)) {
            printf("Ошибка. Невозможно записать длину пути в архив.\n");
            break;
        }

        // Записываем путь файла
        if (write(fd, path, file_path_len) != file_path_len) {
            printf("Ошибка. Невозможно записать путь в архив.\n");
            break;
        }

        error = 0;
    } while (0);

    if (error) {
        if (close(fd) == -1) {
            printf("Ошибка. Невозможно закрыть архив.\n");
        }
        exit(1);
    }
}

void _pack_content(int fd, char* path) {
    int error = 1;      // Флаг ошибка
    int file;           // Файловый дискриптор
    off_t size;         // Размер получаемого содержимого файла
    int file_size;      // размер содержимого файла
    char content_byte;  // Байт для копирования в архив

    // Открываем файл
    file = open(path, O_RDONLY, S_IRUSR);
    if (file == -1) {
        printf("Ошибка. Невозможно открыть %s для чтения.\n", path);
        close(fd);
        exit(1);
    }

    do {
        // Получаем размер файлв
        size = lseek(file, 0, SEEK_END);
        if (size == -1) {
            printf("Ошибка. Невозможно получить %s длину.\n", path);
            break;
        }
        file_size = (int)(size);

        // Переводим на начало
        if (lseek(file, 0, SEEK_SET) == -1) {
            printf("Ошибка. Невозможно поставить курсор %s в начало.\n", path);
            break;
        }

        // Записываем размер содержимого
        if (write(fd, &file_size, sizeof(int)) != sizeof(int)) {
            printf("Ошибка. Невозможно записать размер %s в архив.\n", path);
            break;
        }

        // Чтение содержимого
        for (int num_bytes = 0; num_bytes < file_size; num_bytes++) {
            if (read(file, &content_byte, 1) != 1) {
                printf("Ошибка. Невозможно прочитать содержимое %s.\n", path);
                break;
            }

            // Записывем содержимое
            if (write(fd, &content_byte, 1) != 1) {
                printf("Ошибка. Невозможно записать модержимое %s в архив.\n", path);
                break;
            }
        }

        error = 0;
    } while (0);

    // Закрываем архив
    if (error && close(fd) == -1) {
        printf("Ошибка. Невозможно закрыть архив.\n");
    }

    // Закрываем исходный файл
    if (close(file) == -1) {
        printf("Ошибка. Невозможно закрыть %s.\n", path);
        exit(1);
    }

    // Выход по ошибке
    if (error) {
        exit(1);
    }
}