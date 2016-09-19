#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
  FILE *fd1, *fd2;
  char *str1, *str2;
  char *salt, *hash, *key, *key1;
  char buf[13], word[100], pass[100];
  
  if (argc != 2) {
   fprintf(stderr, "Usage: %s <file shadow>\n", argv[0]);
   exit(-1);
  }

  str1 = (char*)malloc(100);
  str2 = (char*)malloc(100);

  // открываем файл с зашифрованными паролями
  fd1 = fopen(argv[1], "r");
  
  fprintf(stderr, "Please, wait...\n");
  
  // читаем в цикле каждую строчку файла    
  while(fgets(str1, 100, fd1) != NULL)
  {
    str2 = strstr(str1, "$1$"); // ищем в строке символы $1$

    if (str2 != NULL) // и если находим, то 
    {
      // выделяем зашифрованный пароль
      key = strtok(str2, ":"); 
      snprintf(pass, sizeof(pass), "%s", key);    
      printf ("pass=%s (%d)\n", pass, strlen(pass));
    
      // выделяем salt в зашифрованном пароле
      strtok(key, "$");    
      salt = strtok(NULL, "$");
      hash = strtok(NULL, "\0"); // эту операцию можно не делать

      // формируем salt в виде $1$salt$    
      snprintf(buf, sizeof(buf), "$1$%s$", salt);        
    
      // открываем файл словаря
      fd2 = fopen("/usr/share/dict/words", "r"); 

      // читаем в цикле каждое слово из словаря
      while(fgets(word, 100, fd2) != NULL)
      {
        // удаляем символ новой строки
    	(&word[strlen(word)])[-1] = '\0';
	
	// вычисляем новый зашифрованный пароль
        key1 = crypt(word, buf);

	// сравниваем зашифрованные пароли
        if (!strncmp(key1, pass, strlen(key1))) {
          printf("OK! Password: %s\n\n", word);
	  break;
        }
      }
    }
  }

  fclose(fd1);
  fclose(fd2);
  free(str1);
  free(str2);

  return 0;
}
