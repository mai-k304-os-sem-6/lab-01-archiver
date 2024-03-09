#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
     
int getFileSize(const char* file_name, FILE **bout) { // Определение размера файла и запись в bout
    int file_size = 0;
	FILE* fd = fopen(file_name, "rb");
	char byte[1];
	
	if (fd == NULL) {
        file_size = -1;
	} else {
		while(fread(byte, 1, 1 , fd) == 1) {
			file_size++;
			fwrite(byte,1,1,*bout);
		}
		fclose(fd);
	}
	return file_size;
}
   
void printdir(char *dir, int depth, char rep[100], FILE **info_files, FILE **info_direct, FILE **bout) {
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    
    if ((dp = opendir(dir)) == NULL) return; // Проверка на возможность открыть файл
    
    chdir(dir); // Изменяет текущий каталог на заданный
    
    while((entry = readdir(dp)) != NULL) { // Пока чтение директории не закончится
        lstat(entry->d_name, &statbuf); // Возвращение информации о файле
     
		char *Temp1 = rep;
		char *Temp2 = entry->d_name;
		char Temp3[100];
       
       	strcpy(Temp3, Temp1);
       	strcat(Temp3, "/"); 
       	strcat(Temp3, Temp2); 
        if (S_ISDIR(statbuf.st_mode)) { // Проверка на каталог
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0) continue;      
            fprintf(*info_direct, "%s||", Temp3 );
            printdir(entry->d_name, depth+1, Temp3, info_files, info_direct, bout); // Рекурсивный вызов с новый отступом
        } else  {
            if((strcmp(entry->d_name, "info_files.txt")==0) || (strcmp(entry->d_name, "info_direct.txt")==0) || (strcmp(entry->d_name, "bout.txt") ==0)) continue;
            fprintf(*info_files, "%s|%d|%s||", entry->d_name, getFileSize(entry->d_name, bout), Temp3 );
        }
    }
    chdir(".."); // Изменяет текущий каталог на заданный
    closedir(dp); // Закрывает заданый каталог
}

void files_archivation(char name_info_direct_bout[4][100], char* RestOfDir, char* StartDir) {
	char char_info_files[100] = "/info_files.txt"; // Файл информации о файле
    char char_info_direct[100] = "/info_direct.txt"; // Файл информации о директории
    char char_bout[100] = "/bout.txt";
    char char_main[100];
        
    strcpy(char_main, StartDir);
    strcat(char_main, ".pzip"); // Расширение архива

	for(int i = 0; i < 4;i++) {
		char Temp[256];
		strcpy(Temp, RestOfDir);			
		if (i != 3) strcat(Temp, StartDir);
		strcat(Temp, name_info_direct_bout[i]);
		strcpy(name_info_direct_bout[i], Temp);
	}
		
	strcat(name_info_direct_bout[0], char_info_files);
	strcat(name_info_direct_bout[1], char_info_direct);
	strcat(name_info_direct_bout[2], char_bout);
	strcat(name_info_direct_bout[3], char_main);
		
	return;
}
	
void arhivation (char* dir, char* password) { // Функция архивации
    //Выбор директории 
    char StartDir[100];
    char RestOfDir[256];
        
    // Рабочие файлы (в конце удаляются)
    FILE *info_files; // Хранятся названия файлов, размер, директория этого файла
    FILE *info_direct; // Хранится директория папок
    FILE *bout; // Хранится информация файлов
    // Файл архива
    FILE *main;	// Итоговый файл, в котором info_files, info_direct, bout
        
    // Инициализация и очистка массива
    char name_info_direct_bout[4][100];	
    for(int i = 0; i < 4; i++) strcpy(name_info_direct_bout[i], "");
                                
    // Определение конечной папки
	char sep[2] = "/";
	int count1 = 0;
	int count2 = 0;
    		
    while (dir[count2] != 0) count2++;
    			
    count1 = count2;
    			
    while (dir[count1] != *sep) count1--;	
    		
    for(int i = 0; i < (count2-count1 + 1); i++) StartDir[i] = dir[count1+i]; // Опеределение конечной папки 	 
    	
    if (count1 != 0) { // Опеределние директории конечной папки
    	for(int i = 0; i < count1; i++) RestOfDir[i] = dir[i];
    	RestOfDir[count1] = 0;
    }
    	
    // Определение директории для рабочих файлов и файла архива
	files_archivation(name_info_direct_bout, RestOfDir, StartDir );
        
    // Открытие файлов
    info_files = fopen(name_info_direct_bout[0], "wb");
    info_direct = fopen(name_info_direct_bout[1], "wb");
    bout = fopen(name_info_direct_bout[2], "wb");
    	
    // Запись в архив информации о файлах       
    fprintf(info_direct, "%s||", StartDir); 
    printdir(dir, 0, StartDir, &info_files, &info_direct, &bout);    
    fprintf(info_files, "\n");
    fprintf(info_direct, "\n");

    // Закрытие файлов  
    fclose(info_files);
    fclose(info_direct);
    fclose(bout);
        
    char byte[1];  // единичный буфер для считывания одного байта

	// Открытие файлов для чтения        
	info_files = fopen(name_info_direct_bout[0], "rb");
	info_direct = fopen(name_info_direct_bout[1], "rb");
	bout = fopen(name_info_direct_bout[2], "rb");
        
    // Открытие файла для записи
    main = fopen(name_info_direct_bout[3], "wb");

    fprintf(main, password);
    fprintf(main, "\n");

    // переписываем информацию с файлами в файл main
    while(!feof(info_files)) if(fread(byte,1,1,info_files)==1) fwrite(byte,1,1,main);
        
    // переписываем информацию с директориями в файл main
    while(!feof(info_direct)) if(fread(byte,1,1,info_direct)==1) fwrite(byte,1,1,main);
        
    // Переписываем данные в файл main
    while(!feof(bout)) if(fread(byte,1,1,bout)==1) fwrite(byte,1,1,main);
        
    // Закрытие файлов
    fclose(info_files);
    fclose(info_direct);
    fclose(bout);
        
    // Удаление рабочих файлов
    for(int i = 0; i < 3; i++) remove(name_info_direct_bout[i]);

    // Закрытие файла с архивом
    fclose(main);
}
   
void unrar (char* dir, char* password) { // Функция разархивации
   	FILE *main = fopen(strcat(dir, ".pzip"), "rb");

    // Проверка на пароль
    char strPass[10000];
    if (strcmp(fgets(strPass, sizeof(strPass), main), strcat(password, "\n")) != 0) {
        printf("Пароль не верен\n");
        return;
    }
   
   	char str[10000]; // Типо буфера
   	char *first_str; // Для первой строки из main
   	char *istr;
   	first_str = fgets (str, sizeof(str), main); // Разделил первую строку
	char sep [4]  = "|";	
   	istr = strtok (first_str, sep); // Разделил на Name, Size, Path	
   	char *file [100][3];
   	int count = 0;
   	
   	while ((istr != NULL) && (istr[0] != 10)) { // Разделение на составляющие (Name, Size, Path)
   		for (int i = 0; i < 3; i++) {
   			file[count][i] = istr;
   			istr = strtok (NULL,sep);
   		}
   		count++;
   	}
	
   	// Разделил на директории
   	char str1[10000]; // Второй буфер
   	char *second_str; // Для второй строки из main
   	char *istr1;
   	second_str = fgets (str1, sizeof(str1), main);
    
	istr1 = strtok (second_str, sep);
   	char *file1 [100];
   	int count_dir = 0;
    
   	while ((istr1 != NULL) && (istr1[0] != 10) && (istr[0] != 32)) {
		file1[count_dir] = istr1;
		istr1 = strtok (NULL,sep);
   		count_dir++;
   	}
   	
    char cwd[PATH_MAX];
   	char* DirOutPut = getcwd(cwd, sizeof(cwd));
    
   	for (int i = 0; i < count_dir; i++) {
   	    char *CreateNewDir = DirOutPut;
   	    char* Temp = file1[i];
        char PathDir[100];
   	
   	    // Конкотенация
        strcpy(PathDir, CreateNewDir);
        strcat(PathDir, Temp);
    
        mkdir(PathDir, 00777);
    }
      	    
    char byte[1];

	// Создание файлов и побайтовая их перепись согласно количеству файлов
	for (int j = 0; j < count; j++) {
		char* PathSet = DirOutPut;
   		char* PathFile = file[j][2];
   		char PathRes[256];
   	
   		strcpy(PathRes, PathSet);
   		strcat(PathRes, PathFile);
   		
    	int Size = atoi(file[j][1]);
	
	    FILE *Temp = fopen(PathRes, "wb");
	    
	    // Побайтовая перепить согласно заданному размеру
		for(int i = 0; i < Size; i++)
    		if(fread(byte,1,1, main) == 1)
    			fwrite(byte,1,1,Temp);
    	fclose(Temp);
    }
    fclose(main);
}

int main(int argc, char** argv) {
    char cwd[PATH_MAX];
    char* path = strcat(getcwd(cwd, sizeof(cwd)), "/"); // Путь до файла
    strcat(path, argv[1]); // Вместе с файлом
    char* password = argv[3];
    if (strcmp(argv[2], "-a") == 0) arhivation(path, password); // Архивация
    if (strcmp(argv[2], "-u") == 0) unrar(path, password); // Разархивация     
    return 0;
}