# Sistemas de Computación
## TP2 - Stack Frame y Convención de Llamadas

## Integrantes
- Federico Schreiner
- Mateo Bernardi
- Gustavo Regñicoli

---

## Descripción
El objetivo de este TP es demostrar el funcionamiento del 
stack frame y las convenciones de llamadas entre lenguajes 
de distintos niveles de abstracción.

Se implementó una calculadora del índice GINI de Argentina 
utilizando tres capas de software que se comunican entre sí:

- **Python** consulta una API REST del Banco Mundial
- **C** actúa como capa intermedia
- **Assembler** realiza los cálculos usando el stack

## Arquitectura del sistema

La aplicación tiene tres capas:

**Capa 1 - Python** consulta la API REST del Banco Mundial y obtiene el GINI de Argentina automáticamente desde internet.

**Capa 2 - C** actúa como intermediario, recibe el float de Python y llama a la función ASM pasando el parámetro por la pila.

**Capa 3 - Assembler** recibe el float de `[EBP+8]`, lo convierte a entero usando la FPU, le suma 1 y devuelve el resultado en EAX.


## Stack Frame de gini_calc

Cuando C llama a `gini_calc(42.7)` la pila se organiza así:

| Dirección | Contenido | Descripción |
|---|---|---|
| EBP + 8 | 42.7 | Parámetro float |
| EBP + 4 | dir. retorno | Dirección de retorno a main |
| EBP | EBP anterior | EBP de main guardado |
| EBP - 4 | var. local | Entero temporal |

## Código ASM explicado

El prólogo crea el stack frame:
- `push ebp` guarda el EBP de main
- `mov ebp, esp` establece el nuevo frame
- `sub esp, 4` reserva espacio para variable local

El cuerpo hace el cálculo:
- `fld dword [ebp+8]` carga el float 42.7 en la FPU
- `fistp dword [ebp-4]` convierte a entero y guarda en variable local
- `mov eax, [ebp-4]` carga el entero en EAX
- `add eax, 1` suma 1

El epílogo restaura todo:
- `mov esp, ebp` restaura ESP
- `pop ebp` restaura EBP de main
- `ret` vuelve a main

## Resultados obtenidos

| GINI original (float) | GINI entero + 1 |
|---|---|
| 42.7 | 44 |

## Depuración con GDB

Al ejecutar `x/8xw $esp` dentro de `gini_calc` se observó:

    0xffffca24:  0xf7fc6000  0xffffca58  0x5655620a  0x422acccd

El valor `0x422acccd` es la representación del float `42.7` 
en formato IEEE 754, confirmando que el parámetro llegó 
correctamente a la función ASM a través de la pila.

## Estructura del proyecto

    tp2_stackframe/
    ├── gini.asm    → función en Assembler
    ├── main.c      → programa en C
    └── gini.py     → script Python con API REST

## Conclusiones

- Se implementó exitosamente la comunicación entre Python, C y ASM usando convenciones de llamadas
- Los parámetros se pasan por la pila siguiendo la convención cdecl
- La FPU del procesador convierte el float a entero redondeando al entero más cercano
- GDB permitió verificar el estado del stack frame en tiempo real
- El registro EBP actúa como ancla del stack frame permitiendo acceder a parámetros con offsets positivos y variables locales con offsets negativos





