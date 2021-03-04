#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

int main(int argc, char** argv) {
	setlocale(LC_ALL, "Rus");

	if (argc != 4) {
		printf("Ошибка. Возможные аргументы\n");
		printf("--pack <Путь исходного файла> <Путь итогового архива>\n");
		printf("--unpack <Путь архива> <Путь распаковынных файлов>\n");
		return 1;
	}

	char* action = argv[1];
	char* source = argv[2];
	char* output = argv[3];

	if (access(source, 0) == -1) {
		printf("Ошибка. %s не существует.\n", source);
		return 1;
	}



	printf("Успешно.")
	return 0;
}
