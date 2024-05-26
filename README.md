# Sistemas de computación
Trabajo práctico 4 - "Módulos de kernel"

## Integrantes
@Rodriguezzfran

## Kernel Modules - Introducción

¿Qué es exactamente un módulo del núcleo? Los módulos son fragmentos de código que se pueden cargar y descargar en el kernel según se requiera. Extienden la funcionalidad del kernel sin necesidad de reiniciar el sistema. Por ejemplo, un tipo de módulo es el controlador de dispositivo, que permite que el núcleo acceda al hardware conectado al sistema. Sin módulos, tendríamos que construir kernels monolíticos y agregar nuevas funciones directamente en la imagen del kernel. Además de tener kernels más grandes, esto tiene la desventaja de requerir que reconstruyamos y reiniciemos el kernel cada vez que queramos una nueva funcionalidad.

Para poder comenzar con esta experiencia necesitamos utilizar ciertos archivos que se nos brindó desde la cátedra, para eso es necesario utilizar los siguientes comandos:

```bash
git clone https://gitlab.com/sistemas-de-computacion-unc/kenel-modules.git
```
Luego también es necesario instalar algunas dependencias
```bash
$ sudo apt install build-essential 
$ sudo apt install checkinstall 
$ sudo apt install linux-source
```
`build-essential` instala el paquete del mismo nombre, es un paquete meta que contiene una lista de paquetes necesarios para compilar y crear paquetes debian, dentro de ella se encuentran algunas cosas como libc, gcc, g++, make, etc.
`checkinstall`es una herramienta que facilita instalación de programas desde el código fuente.
`linux-source` instala el código fuente del kernel de LInux en el sistema, muy útil para la creación de drivers.

Una vez ejecutados los anteriores comando debemos dirigirnos a la ruta `cd /usr/src` para extraer el source, usando ls vemos que se encuentra lo siguiente

```bash
franco@hp-pavilion-15:/usr/src$ ls
linux-headers-6.5.0-28          linux-headers-6.5.0-35          linux-source-6.5.0          nvidia-535.171.04
linux-headers-6.5.0-28-generic  linux-headers-6.5.0-35-generic  linux-source-6.5.0.tar.bz2
```

Y usando el comando `sudo tar -xvf linux-source-6.8.0.tar bz2` lo extraemos.

ahora nos dirigimos al repositorio otorgado de la cátedra para ejecutar los siguientes comandos

```bash
$ make
$ sudo dmesg -C
$ sudo insmod mimodulo.ko
$ sudo dmesg
$ sudo rmmod mimodulo
$ sudo dmesg
```
los cuales se utilizan para compilar, limpiar los mensajes del anillo del kernel de linux, insertar el modulo al kernel de linux, imprimir los mensajes resultantes, eliminar el modulo y borrar nuevamente los mensajes.

![image](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/122646722/6122fbc7-af71-4c45-bd86-cbd7f701f757)

Como vemos el kernel rechaza la inserción del módulo debido a que no está firmado, para eso tendremeos que seguir investigando el resto de desafíos y preguntas para ver como solucionarlo

## Desafío 1 - Checkinstall

*__¿Qué es checkinstall y para qué sirve?__* Es una herramienta muy útil para la gestión de paquetes en sistemas UNix-like, mantiene un registro de todos los archivos creados o modificados, permite también construir paquetes binarios como *__.deb, .rpm, .tgz__*, permitiendo instalarlo desde el código fuente pero añadiendolo a la lista de paquetes, para luego poder eliminarlo con la herramienta de gestión de paquetes, siendo esta última la principal ventaja ya que de la forma tradicional generalmente uno no sabe dónde terminan los archivos de la instalación, de esta manera, ahora el make install queda envuelto junto al resto de archivos necesarios en un .deb

Para probar esta herramienta creamos un simple hello word en un archivo .c

```C
#include <stdio.h>

int main() {
    printf("Hecho para la cátedra de Sistemas de computación\n");
    printf("Hello, World!\n");
    return 0;
}
```
y un cmake para poder compilar, con instrucciones para usar checkinstall

```make
all: hello

hello: hello.c
    gcc -o hello hello.c

install: hello
    install -m 755 hello /usr/local/bin/hello

clean:
    rm -f hello
```
Ahora utilizando el comando `make` generamos la compilación del código para que luego `sudo checkinstall` genere el `.deb`de nuestro código

![image](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/122646722/119dfadc-4911-4f7d-9715-1039131390bd)
![image](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/122646722/c18696ed-0539-41dc-97bf-c944e97853e3)
![image](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/122646722/f2656cbf-27ed-4545-9d84-5850c6794809)

Si navegamos hasta la ruta de binarios encontraremos nuestro paquete instalado y que además funciona

![image](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/122646722/f515a244-9448-4831-b3ac-620fac1b1f3a)

### Seguridad del kernel

Para aumentar el nivel de seguridad del sistema se opta por usar módulos de kernel de firmados, si el *__arranque seguro__* está habilitado los cargadores de arranque del sistema operativo UEFI, el kernel de linux y todos los módulos tienen que estar firmados con una clave privada y autenticado con una pública, de otro modo, el sistema no podrá terminar el proceso de arranque

Para poder firmar módulos de kernel construidos externamente es que se utilizan los siguientes comandos:
- `openssl` genera un par de claves X.509 públicas y privadas
- `sign-file`usado para firmar un módulo de kernel con la clave privada
- `mokutil` opcionalmente utilizado para inscribir manualmente la clave pública 

Junto a esas instrucciones también se usan una serie de llaveros:

*__builtin_trusted_keys__* es un llavero que se construye en el arranque
contiene claves públicas de confianza. Los privilegios de root son necesarios para ver las claves

*__.platform__* es un llavero que se construye en el arranque contiene claves de proveedores de plataformas de terceros y claves públicas personalizadas. Sólo root puede ver las claves

*__.blacklist__* es un llavero con claves X.509 que han sido revocadas. Un módulo firmado por una clave de .blacklist fallará la autenticación incluso si su clave pública está en .builtin_trusted_keys

De esta manera cuando se carga un módulo del núcleo, éste comprueba la firma del módulo con las claves públicas X.509 del llavero del sistema del núcleo `.builtin_trusted_keys` y del llavero de la plataforma del núcleo `.platform`, el llavero `.platform` contiene claves de proveedores de plataforma de terceros y claves públicas personalizadas. Las claves del llavero del sistema del kernel `.blacklist` están excluidas de la verificación.

Es necesario cumplir ciertas condiciones para cargar módulos del kernel si UEFI Secure Boot está activada: 

- Si el arranque seguro de UEFI está activado o si se ha especificado el parámetro de kernel `module.sig_enforce` sólo puede cargar aquellos módulos del kernel cuyas firmas fueron autenticadas contra claves del llavero del sistema `.builtin_trusted_keys` y del llavero de la plataforma `.platform`, además, la clave pública no debe estar en el llavero de claves revocadas del sistema.

- Si el arranque seguro de UEFI está desactivado y no se ha especificado el parámetro de kernel module.sig_enforce, puedes cargar módulos del núcleo sin firmar y módulos del núcleo firmados sin clave pública.

- Si el sistema no está basado en UEFI o si el arranque seguro de UEFI está desactivado, en `.builtin_trusted_keys` y `.platform` sólo se cargan las claves incrustadas en el núcleo, no se puede aumentar ese conjunto de claves sin reconstruir el núcleo.





















