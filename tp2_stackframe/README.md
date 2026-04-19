# Sistemas de Computación
## TP2 - Stack Frame y Convención de Llamadas

## Integrantes
- Federico Schreiner
- Mateo Bernardi
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
