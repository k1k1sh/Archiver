#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>

int main(int argc, char** argv) {
	setlocale(LC_ALL, "Rus");

	char* action = argv[1];
	char* source = argv[2];

	if (access(source, 0) == -1) {
		printf("Ошибка. %s не существует.\n", source);
	}
	
	exit(0);
}
