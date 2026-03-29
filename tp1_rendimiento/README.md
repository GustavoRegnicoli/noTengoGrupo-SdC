# Sistemas de Computación
## TP1 - Rendimiento
## Integrantes

- Federico Schreiner
- Mateo Bernardi
- Gustavo Regnicoli

## Time Profiling
En primer lugar se realiza el tutorial de gprof y perf, mostrando con capturas de pantalla cada paso y adjuntando conclusiones sobre el uso del tiempo de las funciones.

El profiling es una técnica para medir el rendimiento de un programa,identificando cuánto tiempo consume cada función. Las herramientas utilizadas fueron:

- **gprof**: preciso pero invasivo, requiere recompilar con `-pg`
- **perf**: estadístico y liviano, no requiere recompilar

Se utilizó el programa de ejemplo del tutorial, compuesto por 4 funciones con bucles internos para consumir tiempo de CPU:
```
main()
├── func1()
│   └── new_func1()
└── func2()
```
### Análisis con gprof
**1. Compilación con profiling activado:**
```bash
gcc -Wall -pg test_gprof.c test_gprof_new.c -o test_gprof
```

**2. Ejecución del programa**, que genera automáticamente `gmon.out`

**3. Generación del análisis:**
```bash
gprof test_gprof gmon.out > analysis.txt
```

Al procesar el archivo `gmon.out`, se obtuvo el siguiente reporte de tiempos. El programa tardó un total de **30.56 segundos** en ejecutarse:
| % tiempo | Segundos (Self) | Llamadas | Nombre de Función |
| :---: | :---: | :---: | :--- |
| 38.25 | 11.69 | 1 | `new_func1` |
| 36.55 | 11.17 | 1 | `func2` |
| 25.10 | 7.67 | 1 | `func1` |
| 0.10 | 0.03 | - | `main` |

Para una interpretación más clara, se generó un grafo de llamadas utilizando `gprof2dot`:

![Grafo de Llamadas](time_profiling/grafico.png)

### Análisis con Perf

Se ejecutó el comando `sudo perf record ./test_gprof` seguido de `sudo perf report` para validar los datos sin instrumentación:

![Reporte Perf](time_profiling/captura_perf.png)

## Conclusiones Profiling
Al examinar los datos obtenidos, se observa que el programa requiere un tiempo total de 30.56 segundos para completar su ejecución, concentrando el 99.9% de la carga de CPU en tres funciones: new_func1, func2 y func1. Individualmente, new_func1 representa el mayor "cuello de botella" con un 38.25% del tiempo de procesamiento. 
El análisis del árbol de llamadas permite identificar que la rama de func1 es el verdadero camino crítico (Hot Path), siendo responsable del 63.4% de la latencia total. En contraste, la función main resulta técnicamente irrelevante en términos de consumo de recursos con apenas un 0.10%, limitándose a actuar como coordinadora del flujo.
En cuanto a la metodología, existe una diferencia clave entre ambas herramientas. Mientras que gprof utiliza la instrumentación para ofrecer una visión exacta de la jerarquía de funciones y el número de llamadas, perf emplea un muestreo estadístico que permite observar el programa en su entorno real.


## Benchmarks de Hardware

Un benchmark es una prueba estandarizada para medir el rendimiento de un sistema de forma objetiva y comparable entre distintos equipos.

### Tareas diarias y benchmarks representativos

| Tarea diaria | Benchmark |
|---|---|
| Compilar código | pts/build-linux-kernel |
| Programar en IDE | GeekBench single-core |
| Navegar por internet | Speedometer |
| Simular circuitos (LTSpice) | SPEC FP |

### Rendimiento compilando el kernel de Linux
Datos obtenidos de [openbenchmarking.org](https://openbenchmarking.org/test/pts/build-linux-kernel-1.15.0).
El benchmark mide el tiempo en segundos para compilar el kernel 
de Linux — **menos tiempo es mejor rendimiento**.

| Procesador | Núcleos | Tiempo | TDP | Precio aprox |
|---|---|---|---|---|
| Intel Core i5-13600K | 14 | 83s | 125W | ~$300 |
| AMD Ryzen 9 5900X 12-Core | 12 | 88s | 105W | ~$250 |
| AMD Ryzen 9 7950X 16-Core | 16 | 54s | 170W | ~$700 |

### Speedup del AMD Ryzen 9 7950X
Fórmula: `Speedup = T_base / T_7950X`

- Respecto al i5-13600K: 76 / 54 = **1.40x más rápido**
- Respecto al Ryzen 9 5900X: 88 / 54 = **1.63x más rápido**

### Conclusión
**En precio:** El **i5-13600K** ofrece la mejor relación 
precio/rendimiento con ~$300 logrando un rendimiento cercano 
al 7950X que cuesta más del doble.

**En energía:** El **5900X** es el más eficiente energéticamente 
con solo 105W de TDP. El 7950X consume 170W para lograr su mayor 
rendimiento. El i5-13600K puede llegar a 181W bajo carga total.

## Practico ESP32 - Variación de frecuencia

### Consigna
Ejecutar un código que demore alrededor de 10 segundos con dos 
bucles: uno con sumas de enteros y otro con sumas de floats.
Analizar qué sucede con el tiempo al variar la frecuencia del CPU.

### Configuración
- **Hardware:** ESP32 Dev Module
- **Iteraciones:** 57.000.000 por bucle
- **Frecuencias probadas:** 80, 160 y 240 MHz

### Resultados

| Frecuencia | Tiempo enteros | Tiempo floats |
|------------|---------------|---------------|
| 80 MHz | 10212 ms | 10212 ms |
| 160 MHz | 5046 ms | 5046 ms |
| 240 MHz | 3350 ms | 3350 ms |

### Capturas
![80MHz](rendimiento_esp/80MHz.png)
![160MHz](rendimiento_esp/160MHz.png)
![240MHz](rendimiento_esp/240MHz.png)

### Speedup
Tomando 80 MHz como base:

| Comparación | Speedup |
|-------------|---------|
| 160 MHz vs 80 MHz | 10212 / 5046 = **2.02x** |
| 240 MHz vs 80 MHz | 10212 / 3350 = **3.05x** |

### Conclusiones
- Al duplicar la frecuencia de 80 a 160 MHz el tiempo se reduce 
  a casi la mitad, con un speedup de **2.02x**
- Al triplicar la frecuencia de 80 a 240 MHz el tiempo se reduce 
  a un tercio, con un speedup de **3.05x**
- Los tiempos de enteros y floats son prácticamente iguales, 
  lo que indica que el ESP32 tiene una **unidad de punto flotante 
  (FPU) por hardware** que procesa floats igual de rápido que enteros
- La relación entre frecuencia y tiempo es **lineal y directa**: 
  a mayor frecuencia, menor tiempo de ejecución de forma proporcional
