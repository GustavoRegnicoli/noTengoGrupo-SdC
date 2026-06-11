obj-m += CDD.o

# Esta bandera es obligatoria para evitar el error __gnu_mcount_nc en ARMv6
ccflags-y := -fno-optimize-sibling-calls

all:
	make -C /home/gustavo/noTengoGrupo-SdC/tp5_device_drivers/rpi-kernel-src ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- CONFIG_FUNCTION_TRACER=n M=$(PWD) modules

clean:
	make -C /home/gustavo/noTengoGrupo-SdC/tp5_device_drivers/rpi-kernel-src ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- M=$(PWD) clean
