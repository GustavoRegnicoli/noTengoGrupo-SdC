section .text
global gini_calc

gini_calc:
    ; PRÓLOGO
    push ebp
    mov ebp, esp
    sub esp, 4          ; reserva 4 bytes para variable local

    ; CONVERTIR FLOAT A ENTERO
    fld dword [ebp+8]   ; carga el float (42.7) en la FPU
    fistp dword [ebp-4] ; convierte a int y guarda en variable local

    ; CARGAR EL ENTERO Y SUMARLE 1
    mov eax, [ebp-4]    ; carga el entero en EAX (42)
    add eax, 1          ; suma 1 → EAX = 43

    ; EPÍLOGO
    mov esp, ebp
    pop ebp
    ret
