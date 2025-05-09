%define PAGE 1000h

%define sys_exit   1
%define sys_write  4
%define sys_open   5
%define sys_lseek  19
%define sys_munmap 91
%define sys_mmap2  192

bits 32

  org 0x720000

  db 7fh,"ELF",1,1,1,9 dup (0)  ; ident
  dw 02h,03h                    ; type mach
  dd 01h,_start,52,0,0          ; vers entr phof shof flag
  dw 52,32,1,0,0,0              ; ehsz phsz phnm shsz shnm shix

  dd 01h,00h,$$,00h       ; type offs vadr padr
  dd fisz,fisz,07h,PAGE   ; fisz mmsz flag algn

section .rodata

  aout  db `a.out`,0
  low   db `\e[36m`
  mid   db `\e[37m`
  hgh   db `\e[97m`
  csz   equ 5
  alpha db "0123456789abcdef"

section .text

_start:
  mov edi, [esp]
  lea esi, [esp+4]

  mov eax, sys_open
  mov ebx, aout
  xor ecx, ecx
  int 80h

  test eax, eax
  js   fatal

  push eax
  mov ebx, eax
  mov eax, sys_lseek
  mov ecx, 0
  mov edx, 2
  int 80h

  test eax, eax
  js   fatal

  mov ecx, eax
  mov eax, sys_mmap2
  xor ebx, ebx
  mov edx, 1
  mov esi, 2
  pop edi
  xor ebp, ebp
  int 80h

  push eax
  push ecx

l movzx edi, byte [eax]
  shr   edi, 4
  movzx edx, byte [eax]
  and   edx, 15

  mov bl, [alpha+edi]
  mov bh, [alpha+edx]

  pushad
  mov eax, sys_write
  mov ebx, 1
  lea ecx, [esp+16]
  mov edx, 2
  int 80h

  call spc
  popad

  inc eax
  loop l

  mov eax, sys_munmap
  pop ecx
  pop ebx
  int 80h

  mov eax, sys_exit
  xor ebx, ebx
  int 80h

fatal:
  mov eax, sys_exit
  mov ebx, eax
  int 80h

s db ' '
spc:
  mov eax, sys_write
  mov ebx, 1
  lea ecx, [rel s]
  mov edx, 1
  int 80h
  ret

fisz equ $ - $$

; vim: set ft=nasm:
