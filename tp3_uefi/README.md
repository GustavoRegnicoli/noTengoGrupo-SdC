# Práctico 3a - UEFI
## Sistemas de Computación - UNC 2026

## Integrantes
- Bernardi Mateo
- Schreiner Federico
- Regnicoli Gustavo

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

### Fases de arranque UEFI
- **SEC** → fase pre-memoria, establece confianza inicial
- **PEI** → inicializa RAM y chipset
- **DXE** → carga drivers, inicializa hardware completo
- **BDS** → decide qué bootear leyendo variables NVRAM
- **Runtime** → el SO toma control, quedan servicios UEFI activos
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

## Parte 1 - Exploración de la Shell UEFI

Se arrancó QEMU con firmware UEFI:

    qemu-system-x86_64 -m 512 -bios /usr/share/ovmf/OVMF.fd -net none

<img width="1280" height="860" alt="image" src="https://github.com/user-attachments/assets/b1b661af-9a83-4913-8938-00ea511a2be7" />

### map — Dispositivos disponibles

<img width="1280" height="860" alt="image" src="https://github.com/user-attachments/assets/b8ced45f-98d6-40f2-8a35-0695ad197923" />


UEFI no usa letras de unidad fijas. Usa Handles que agrupan
Protocolos — interfaces estándar independientes del hardware.

**Pregunta de Razonamiento 1:** Al ejecutar el comando map y dh, vemos protocolos e identificadores en lugar de puertos de hardware fijos. ¿Cuál es la ventaja de seguridad y compatibilidad de este modelo frente al antiguo BIOS?

**Respuesta:** El firmware puede interactuar con un disco sin saber si está
conectado por SATA, USB o PCIe — usa siempre la misma API.
Esto previene conflictos y facilita el desarrollo seguro.

### dh -b — Handles y Protocolos

<img width="1280" height="860" alt="image" src="https://github.com/user-attachments/assets/8333e47c-3a8c-4913-8207-ecc49cfc650e" />
...
<img width="1280" height="860" alt="image" src="https://github.com/user-attachments/assets/b8cf0c99-833d-4dbe-b266-12db9dcd28cc" />

Muestra todos los objetos del sistema UEFI y sus interfaces.

### memmap -b — Mapa de memoria

<img width="1280" height="860" alt="image" src="https://github.com/user-attachments/assets/c51e764c-d26b-4624-9252-4c73fc89677c" />
...
<img width="1280" height="860" alt="image" src="https://github.com/user-attachments/assets/99a151bb-fa72-476c-bef5-6cf6d3676fae" />

**Pregunta de Razonamiento 3:** En el mapa de memoria (memmap), existen regiones marcadas como RuntimeServicesCode. ¿Por qué estas áreas son un objetivo principal para los desarrolladores de malware (Bootkits)?

**Respuesta** La memoria RuntimeServices no se borra
cuando el SO toma el control. Un Bootkit inyectado ahí opera
con privilegios Ring -2/SMM invisible para cualquier antivirus.

### dmpstore — Variables NVRAM
<img width="1280" height="860" alt="image" src="https://github.com/user-attachments/assets/ec5ea690-b599-4460-af37-41b91f379440" />


**Pregunta de Razonamiento 2:** Observando las variables Boot#### y BootOrder, ¿cómo determina el Boot Manager la secuencia de arranque?

**Respuesta :** El Boot Manager lee BootOrder
(ej: 0000, 0002) y busca la variable Boot0000 que contiene
la ruta del .efi a ejecutar.

### Variable personalizada
<img width="960" height="660" alt="image" src="https://github.com/user-attachments/assets/0d990c28-3adb-4305-9b8b-a24acab99042" />


    set TestSeguridad "Hola UEFI"
    set -v

---

## Parte 2 - Desarrollo de la Aplicación UEFI

### Código fuente

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

**Pregunta de Razonamiento 4:** ¿Por qué utilizamos SystemTable->ConOut->OutputString en lugar de la función printf de C?

**Respuesta** No existe printf porque en el entorno
pre-OS de UEFI no hay sistema operativo ni libc. Toda la E/S
se hace a través de protocolos de la SystemTable.

**¿Por qué 0xCC?**
Es el opcode de INT3 — un breakpoint de software. El código
demuestra cómo detectarlo estáticamente, relevante en análisis
de firmware malicioso.

---

## Parte 3 - Compilación

### Paso 1 - Compilar a objeto

    gcc -I/usr/include/efi -I/usr/include/efi/x86_64
        -I/usr/include/efi/protocol -fpic -ffreestanding
        -fno-stack-protector -fno-strict-aliasing -fshort-wchar
        -mno-red-zone -maccumulate-outgoing-args
        -Wall -c -o aplicacion.o aplicacion.c

### Paso 2 - Linkear con UEFI

    ld -shared -Bsymbolic -L/usr/lib -L/usr/lib/efi
        -T /usr/lib/elf_x86_64_efi.lds
        /usr/lib/crt0-efi-x86_64.o
        aplicacion.o -o aplicacion.so -lefi -lgnuefi

### Paso 3 - Convertir a PE/COFF

    objcopy -j .text -j .sdata -j .data
        -j .dynamic -j .dynsym
        -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc
        --target=efi-app-x86_64
        aplicacion.so aplicacion.efi

### Verificación
<img width="1366" height="728" alt="image" src="https://github.com/user-attachments/assets/ac697742-89e2-4d93-9bfa-900c691543b5" />


    file aplicacion.efi
    → PE32+ executable (EFI application) x86-64

**¿Qué es PE/COFF?**
Formato de ejecutable de Windows. UEFI lo usa aunque compilemos
desde Linux porque es portable entre arquitecturas.

---

## Parte 4 - Ejecución en QEMU

Se creó una imagen FAT32 con la shell UEFI y la aplicación:

    dd if=/dev/zero of=disco.img bs=1M count=64
    mkfs.vfat -F 32 disco.img
    sudo mount disco.img mnt
    sudo mkdir -p mnt/EFI/BOOT
    sudo cp Shell.efi mnt/EFI/BOOT/BOOTX64.EFI
    sudo cp aplicacion.efi mnt/
    sudo umount mnt

    qemu-system-x86_64 \
        -bios /usr/share/ovmf/OVMF.fd \
        -drive format=raw,file=disco.img,if=virtio \
        -net none

### HelloWorld.efi del repositorio UEFI-Lessons
<img width="1280" height="860" alt="image" src="https://github.com/user-attachments/assets/1e41ce0c-d851-4208-b319-3a8709f1223c" />


    fs0:
    HelloWorld.efi

Se utilizó el HelloWorld.efi del repositorio UEFI-Lessons
compilado con EDK2 para verificar el correcto funcionamiento
del entorno UEFI en QEMU.

### Problema con aplicacion.efi compilada con gnu-efi

Al intentar ejecutar aplicacion.efi la aplicación se congela.
Esto se debe a un problema de compatibilidad conocido entre
gnu-efi y la consola virtio de QEMU. El archivo fue verificado
como PE32+ válido — la compilación fue exitosa pero hay una
incompatibilidad de runtime con el entorno de emulación.

---

## Análisis de seguridad

**Pregunta 5:** ¿Por qué 0xCC aparece como -52 en Ghidra?
El compilador interpreta char como entero con signo. En
complemento a dos de 8 bits, 0xCC (204) equivale a -52.
Esto es crítico en ciberseguridad porque una regla YARA
que busque 204 fallaría — hay que buscar el byte 0xCC.

---

## Conclusiones

- UEFI reemplaza al BIOS con arquitectura moderna de 64 bits
- El modelo de Handles y Protocolos abstrae el hardware físico
- Las aplicaciones UEFI se compilan en formato PE/COFF
- UEFI ejecuta código antes que cualquier sistema operativo
- Las variables NVRAM controlan la secuencia de arranque
- RuntimeServices persiste después de cargar el SO siendo
  objetivo de Bootkits y malware persistente
- gnu-efi permite compilar desde Linux pero puede tener
  incompatibilidades con ciertos entornos de QEMU
