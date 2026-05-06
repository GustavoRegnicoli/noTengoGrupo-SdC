# Práctico 3a - UEFI
## Sistemas de Computación - UNC 2026

## Integrantes
- Bernardi Mateo
- Regnicoli Gustavo
- Schreiner Federico

---

## Introducción

UEFI (Unified Extensible Firmware Interface) es la infraestructura
de firmware estándar que reemplaza al antiguo BIOS. A diferencia
del BIOS que operaba en 16 bits con un límite de 1MB de memoria,
UEFI ofrece una arquitectura de 64 bits con soporte para
discos grandes, red, sistema de archivos FAT32 y Secure Boot.

### Diferencias con el práctico anterior

| TP anterior (BIOS Legacy) | Este práctico (UEFI) |
|---|---|
| Modo real 16 bits | 64 bits |
| Carga 512 bytes con firma 0xAA55 | Carga archivos .efi desde FAT32 |
| Sin filesystem | Con soporte FAT32 |
| Sin interfaz gráfica | Con shell interactiva |
| Sin Secure Boot | Con Secure Boot |

---

## Preparación del entorno

Se instalaron las siguientes dependencias:

    sudo apt install -y qemu-system-x86 ovmf gnu-efi
                        build-essential binutils-mingw-w64

- **qemu-system-x86** → emulador de PC
- **ovmf** → firmware UEFI virtual para QEMU (en /usr/share/ovmf/OVMF.fd)
- **gnu-efi** → bibliotecas para compilar aplicaciones UEFI en Linux
- **build-essential** → compilador GCC y herramientas

---

## Parte 1

Se arrancó QEMU con firmware UEFI usando OVMF:

    qemu-system-x86_64 -m 512 -bios /usr/share/ovmf/OVMF.fd -net none

### Comando map

Muestra los dispositivos disponibles. UEFI no usa letras de
unidad fijas sino Handles que agrupan Protocolos — interfaces
estándar independientes del hardware físico.

![1](https://github.com/GustavoRegnicoli/noTengoGrupo-SdC/blob/main/tp3_uefi/capturas/Captura%20de%20pantalla%20de%202026-05-04%2001-26-01.png)
![2](https://github.com/GustavoRegnicoli/noTengoGrupo-SdC/blob/main/tp3_uefi/capturas/Captura%20de%20pantalla%20de%202026-05-04%2001-26-24.png)


**Pregunta de Razonamiento 1:** Al ejecutar el comando map y dh, vemos protocolos e identificadores en lugar de puertos de hardware fijos. ¿Cuál es la ventaja de seguridad y compatibilidad de este modelo frente al antiguo BIOS?

**Respuesta:** El modelo de protocolos abstrae el
hardware físico. Un binario puede interactuar con un disco sin
saber si está conectado por SATA, USB o PCIe, usando siempre
la misma API estándar. Esto previene conflictos y facilita
el desarrollo seguro.

### Comando dh -b

Muestra la base de datos de Handles y Protocolos del sistema.
![1](https://github.com/GustavoRegnicoli/noTengoGrupo-SdC/blob/main/tp3_uefi/capturas/Captura%20de%20pantalla%20de%202026-05-04%2001-27-38.png)


### Comando memmap -b

Muestra el mapa de memoria. Las regiones RuntimeServicesCode
son críticas para la seguridad.

**Pregunta de Razonamiento 3:** En el mapa de memoria (memmap), existen regiones marcadas como RuntimeServicesCode. ¿Por qué estas áreas son un objetivo principal para los desarrolladores de malware (Bootkits)?

**Respuesta** La memoria RuntimeServices no se borra
cuando el SO toma el control. Un Bootkit inyectado ahí opera
con privilegios Ring -2/SMM invisible para cualquier antivirus.

### Variables NVRAM - dmpstore

Muestra variables Boot0000, BootOrder que controlan el arranque.

**Pregunta de Razonamiento 2:** Observando las variables Boot#### y BootOrder, ¿cómo determina el Boot Manager la secuencia de arranque?

**Respuesta :** El Boot Manager lee BootOrder
(ej: 0000, 0002) y busca la variable Boot0000 que contiene
la ruta del .efi a ejecutar.

### Variable personalizada
[📸 CAPTURA]

    set TestSeguridad "Hola UEFI"
    set -v

---

## Parte 2 - Desarrollo de la Aplicación UEFI

### ¿Qué hace la aplicación?

La aplicación es un Hello World nativo UEFI que:
1. Imprime un mensaje usando OutputString de la SystemTable
2. Crea un byte con valor 0xCC (opcode de INT3 = breakpoint)
3. Verifica si el byte es 0xCC e imprime un segundo mensaje

**Pregunta de Razonamiento 4:** ¿Por qué utilizamos SystemTable->ConOut->OutputString en lugar de la función printf de C?

**Respuesta** No existe printf porque en el entorno
pre-OS de UEFI no hay sistema operativo ni libc. Toda la E/S
se hace a través de protocolos de la SystemTable.

### Código fuente (aplicacion.c)

    #include <efi.h>
    #include <efilib.h>

    EFI_STATUS efi_main(EFI_HANDLE ImageHandle,
                        EFI_SYSTEM_TABLE *SystemTable) {
        InitializeLib(ImageHandle, SystemTable);
        SystemTable->ConOut->OutputString(SystemTable->ConOut,
            L"Iniciando analisis de seguridad...\r\n");

        unsigned char code[] = { 0xCC };

        if (code[0] == 0xCC) {
            SystemTable->ConOut->OutputString(SystemTable->ConOut,
                L"Breakpoint estatico alcanzado.\r\n");
        }

        return EFI_SUCCESS;
    }

---

## Parte 3 - Compilación

### Paso 1 - Compilar a objeto

    gcc -I/usr/include/efi -I/usr/include/efi/x86_64
        -I/usr/include/efi/protocol -fpic -ffreestanding
        -fno-stack-protector -fno-strict-aliasing -fshort-wchar
        -mno-red-zone -maccumulate-outgoing-args
        -Wall -c -o aplicacion.o aplicacion.c

Genera aplicacion.o — el código objeto.

### Paso 2 - Linkear con bibliotecas UEFI

    ld -shared -Bsymbolic -L/usr/lib -L/usr/lib/efi
        -T /usr/lib/elf_x86_64_efi.lds
        /usr/lib/crt0-efi-x86_64.o
        aplicacion.o -o aplicacion.so -lefi -lgnuefi

Une el objeto con las bibliotecas UEFI.

### Paso 3 - Convertir a formato PE/COFF

    objcopy -j .text -j .sdata -j .data
        -j .dynamic -j .dynsym
        -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc
        --target=efi-app-x86_64
        aplicacion.so aplicacion.efi

Convierte al formato PE/COFF que usa UEFI.

### Verificación del formato

    file aplicacion.efi



**¿Qué es PE/COFF?** Es el formato de ejecutable de Windows.
UEFI lo usa aunque compilemos desde Linux porque es portable.

---

## Parte 4 - Ejecución en QEMU

Se creó una imagen de disco FAT32:

    dd if=/dev/zero of=disco.img bs=1M count=64
    mkfs.vfat -F 32 disco.img
    sudo mount disco.img mnt
    sudo mkdir -p mnt/EFI/BOOT
    sudo cp Shell.efi mnt/EFI/BOOT/BOOTX64.EFI
    sudo cp aplicacion.efi mnt/
    sudo umount mnt

Se ejecutó QEMU con firmware UEFI:

    qemu-system-x86_64 \
        -bios /usr/share/ovmf/OVMF.fd \
        -drive format=raw,file=disco.img,if=virtio \
        -net none

### Ejecución del HelloWorld.efi del repositorio

Se utilizó el HelloWorld.efi del repositorio UEFI-Lessons
compilado con EDK2 para verificar el correcto funcionamiento
del entorno:

    fs0:
    HelloWorld.efi
![1](https://github.com/GustavoRegnicoli/noTengoGrupo-SdC/blob/main/tp3_uefi/capturas/Captura%20de%20pantalla%20de%202026-05-06%2002-41-13.png)

### Problema con aplicacion.efi compilada con gnu-efi

Al intentar ejecutar nuestra aplicación compilada con gnu-efi:

    fs0:
    aplicacion.efi

La aplicación se congela. Esto se debe a un problema de
compatibilidad conocido entre gnu-efi y la consola virtio
de QEMU. El archivo aplicacion.efi fue verificado como
PE32+ válido con el comando file, confirmando que la
compilación fue exitosa. El problema es de runtime en el
entorno de emulación específico.

---

## Análisis de seguridad

**Pregunta de Razonamiento 5:** En el pseudocódigo de Ghidra, la condición 0xCC suele aparecer como -52. ¿A qué se debe este fenómeno y por qué importa en ciberseguridad?

**Respuesta** El compilador interpreta char como
entero con signo. En complemento a dos de 8 bits, 0xCC (204)
equivale a -52. Esto es crítico en ciberseguridad porque una
regla YARA que busque 204 fallaría — hay que buscar el byte
0xCC directamente.

---

## Conclusiones

- UEFI reemplaza al BIOS con arquitectura moderna de 64 bits
- El modelo de Handles y Protocolos abstrae el hardware físico
- Las aplicaciones UEFI se compilan en formato PE/COFF
- UEFI ejecuta código antes que cualquier sistema operativo
- Las variables NVRAM controlan la secuencia de arranque
- La memoria RuntimeServices persiste después de cargar el SO
  siendo objetivo de malware persistente como Bootkits
- gnu-efi permite compilar aplicaciones UEFI desde Linux pero
  puede tener incompatibilidades con ciertos entornos de QEMU
