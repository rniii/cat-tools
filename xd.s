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

  aout  db "a.out",0
  alpha db "0123456789abcdef"

section .text

_start:
  mov edi, [esp]
  lea esi, [esp+4]

  xor eax, eax
  mov al, sys_open
  mov ebx, aout
  xor ecx, ecx
  int 80h

  push eax            ; eax = fd
  mov ebx, eax
  xor eax, eax
  mov al, sys_lseek
  xor ecx, ecx
  xor edx, edx        ; mov edx, 2
  inc edx             ;
  inc edx
  int 80h

  mov ecx, eax
  xor eax, eax
  mov al, sys_mmap2
  xor ebx, ebx
  xor edx, edx        ; mov edx, 1
  inc edx             ;
  mov esi, edx        ; mov esi, 1
  pop edi             ; edi = fd
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
  xor eax, eax
  mov al, sys_write
  xor ebx, ebx        ; mov ebx, 1
  inc ebx             ;
  lea ecx, [esp+16]
  mov edx, ebx        ; mov edx, 2
  inc edx             ;
  int 80h

  call spc
  popad

  inc eax
  loop l

  xor eax, eax
  mov al, sys_munmap
  pop ecx
  pop ebx
  int 80h

  xor eax, eax        ; mov eax, 1 (sys_exit)
  inc eax             ;
  xor ebx, ebx
  int 80h

s db ' '
spc:
  xor eax, eax
  mov al, sys_write
  xor ebx, ebx        ; mov ebx, 1
  inc ebx             ;
  lea ecx, [rel s]
  mov edx, ebx        ; mov edx, 1
  int 80h
  ret

fisz equ $ - $$

; vim: set ft=nasm:
