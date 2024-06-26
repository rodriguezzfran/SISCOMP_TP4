# Sistemas de computación
Trabajo práctico 4 - "Módulos de kernel"

## Integrantes
[Bruno Guglielmotti](https://github.com/BrunoGugli)

[Franco Rodriguez](https://github.com/rodriguezzfran)

[Mauricio Valdez](https://github.com/mauriciovaldez19)

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


## Desafio 2 - Programas vs. Módulos

### ¿Qué funciones tiene disponible un programa y un módulo?
Un programa tiene acceso a funciones de la librería estándar de C que se resuelven en tiempo de enlace y a las funciones propias del código del programa, y también, aunque mediante una interfaz brindadapor las funciones de la librería, a las _system calls_ proporcionadas por el kernel; mientras que un módulo tiene acceso solo a estas últimas, las cuales se pueden ver en el contenido del archivo ```/proc/kallsyms```, y puede definir sus propias funciones para interactuar directamente con el sistema operativo (un ejemplo de esto son `module_init` y `module_exit`, que se utilizan para definir el inicio y el fin del módulo, respectivamente).

Para ver las _system calls_ generadas por un programa a partir de una función de la librería estándar compilamos el siguiente script con la flag ```-Wall``` y luego lo ejecutamos con ```strace```:
```C
#include <stdio.h>

int main(void)
{
	printf("hello");
	return 0;
}
```

![syscall_tp4_siscomp](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/129474500/8902016e-ee0f-4dc4-9ca8-0ffdab4f8cee)

Y ahora si queremos ver la lista ordenada por _syscall_ ejecutamos ```$ strace -c ./hello```:

![syscalls_order](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/129474500/e5c02dd7-b1d3-4ec1-aea7-ce7f4b633919)


### Espacio de usuario y espacio de kernel:

La diferencia entre el espacio de kernel y el espacio de usuario se refiere a los distintos niveles de privilegios y acceso en los que se ejecutan los programas y procesos en un sistema operativo.

#### Espacio de Kernel
- __Nivel de Privilegios__: Alto. El código que se ejecuta en el espacio de kernel tiene acceso completo a todo el hardware y la memoria del sistema.
- __Acceso__: Directo a los recursos del sistema, como el hardware, memoria, y periféricos.
- __Responsabilidades__: Manejar tareas críticas del sistema, como la gestión de memoria, administración de procesos, control de dispositivos, y comunicación entre procesos.
- __Estabilidad__: Un error en el espacio de kernel puede causar que todo el sistema se bloquee o se vuelva inestable.


#### Espacio de usuario
- __Nivel de Privilegios__: Bajo. El código que se ejecuta en el espacio de usuario tiene acceso restringido y no puede interactuar directamente con el hardware o la memoria del sistema sin pasar por el núcleo.
- __Acceso__: Indirecto a los recursos del sistema, utilizando llamadas al sistema (system calls) para solicitar servicios del kernel.
- __Responsabilidades__: Ejecutar aplicaciones y procesos de usuario, como navegadores web, editores de texto, y programas de usuario en general.
- __Estabilidad__: Un error en el espacio de usuario generalmente solo afecta al proceso que lo causó, sin comprometer la estabilidad del sistema operativo en su conjunto.

### Espacio de datos
En el espacio de usuario incluye las variables y estructuras de datos que las aplicaciones y procesos de usuario manipulan. Se subdivide en __Datos Estáticos y BSS__ (variables globales y estáticas), __Heap__ (memoria dinámica asignada durante la ejecución) y __Stack__ (memoria para variables locales y control de flujo de las funciones).

En el espacio de Kernel incluye las estructuras de datos y variables que el núcleo utiliza para gestionar el sistema. También se subdivide en __Kernel Data__ (estructuras de datos globales y estáticas del kernel), __Kernel Heap__ (memoria dinámica para el kernel y módulos) y __Kernel Stack__ (memoria para el contexto de ejecución de funciones del kernel).

Hay que tener cuidado cuando un programa tiene muchas variables globales que no son lo suficientemente significativas como para distinguirlas, ya que puede producirse una contaminación del espacio de nombres. En proyectos grandes, se debe hacer un esfuerzo para recordar los nombres reservados y encontrar formas de desarrollar un esquema para nombrar nombres y símbolos de variables únicos.

Al escribir código del kernel, incluso el módulo más pequeño se vinculará con todo el kernel, por lo que esto definitivamente es un problema. La mejor manera de lidiar con esto es declarar todas las variables como estáticas y usar un prefijo bien definido para los símbolos. Por convención, todos los prefijos del kernel están en minúsculas. Si no se desea declarar todo como estático, otra opción es declarar una tabla de símbolos y registrarla en el kernel.

### Drivers. Contenido de ```/dev/```

Los drivers son una clase de modulo que permiten al sistema operativo comunicarse directamente con el hardware. En Unix, cada pieza de hardware está representada por un archivo ubicado en /dev llamado _device file_ que proporciona los medios para comunicarse con el hardware. El controlador del dispositivo proporciona la comunicación en nombre de un programa de usuario.

Los dispositivos se dividen en dos tipos: __Character devices__ y __Block devices__. La diferencia es que los _block devices_ tienen un buffer para solicitudes, por lo que pueden elegir el mejor orden para responder a las solicitudes. Esto es importante en el caso de los dispositivos de almacenamiento, donde es más rápido leer o escribir sectores cercanos entre sí que aquellos que están más separados. Otra diferencia es que los _block devices_ solo pueden aceptar entradas y devolver salidas en bloques (cuyo tamaño puede variar según el dispositivo), mientras que los _character devices_ pueden usar tantos o tan pocos bytes como quieran. La mayoría de los dispositivos en el mundo son _character_ porque no necesitan este tipo de almacenamiento en buffer y no funcionan con un tamaño de bloque fijo.

### Que diferencia hay entre los dos modinfo

Los comandos a evaluar son:
*__modinfo mimodulo.ko__* 
*__modinfo /lib/modules/$(uname -r)/kernel/crypto/des_generic.ko__*

Recordemos que `modinfo` muestra información sobre un módulo del kernel, y la información puede incluir detalles como la versión, el autor, la descripción, la licencia, entre otros.

Al ejecutar `modinfo mimodulo.ko` se obtendrá información específica sobre un módulo del kernel que has creado o que se ha obtenido fuera del árbol principal del kernel (out-of-tree), mientras que al ejecutar `modinfo /lib/modules/$(uname -r)/kernel/crypto/des_generic.ko` se obtendrá información sobre un módulo que forma parte del kernel principal (in-tree). Este módulo estará firmado y validado, lo cual es importante para la seguridad y la estabilidad del sistema

### ¿Qué divers/modulos estan cargados en sus propias pc? 

para poder hacer esta consigna se ejecuta el comando `lsmod` la cual muestra todos los módulos cargados en el sistema, usando una PIPE se puede anexar el comando `head -n 30`para ver una lista acotada

![image](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/122646722/df8366a6-c444-4256-9ee1-e03e1c462829)
![imagen](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/103122420/5cc13508-a181-4767-82c8-974d43b641bf)

ejecuntando diff con 2 de los 3 archivos la salida muestr alago así

![image](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/122646722/575ba363-8c00-4299-81e0-6001df600d17)

indicando que líneas son diferentes en las 2 computadoras

para poder ver cuales están disponibles pero no están cargados se usa el comando `find /lib/modules/$(uname -r)/kernel -type f -name "*.ko"` para listar todos los módulos disponibles, luego con  `lsmod | awk '{print $1}' > /tmp/loaded_modules` se puede comparar con los cargados. El comando completo `find /lib/modules/$(uname -r)/kernel -type f -name "*.ko" | awk -F'/' '{print $NF}' | sed 's/\.ko$//' > /tmp/available_modules` es el usado, finalmente `comm -23 /tmp/available_modules /tmp/loaded_modules` da los disponibles pero no cargados.

ejecutandolos tenemos la siguiente salida

![image](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/122646722/13328beb-2c66-44ac-9ff3-e4f6a49cdc1a)

Qué sucede cuando el driver de un dispositivo no está disponible?

Cuando el driver de un dispositivo no está disponible, pueden ocurrir varios problemas:

- *__Dispositivo no reconocido:__* El sistema operativo no podrá reconocer el dispositivo, esto significa que el dispositivo no aparecerá en las interfaces de usuario, como el administrador de dispositivos en sistemas gráficos.

- *__Funcionalidad limitada:__* El dispositivo puede aparecer pero con funcionalidad limitada, por ejemplo, una tarjeta de red puede ser reconocida pero no funcionar correctamente sin el driver adecuado.

- *__Errores en los registros del sistema:__* Puede haber mensajes de error en los registros del sistema (consultables con dmesg, journalctl, o en archivos de registro en /var/log/) indicando que no se encontró el driver necesario para el dispositivo.

- *__Fallo en la carga del sistema:__* En casos raros, la falta de un driver crítico puede causar que el sistema operativo no cargue correctamente o falle durante la operación.

### Correr hwinfo
Para poder hacer esta parte se tiene que instalar el paquete de hwinfo con `sudo apt-get install hwinfo`, luego con el comando `sudo hwinfo --short > hwinfo_report.txt` se genera el informe, el cual se encuentra en el root del proyecto, con sus correspondientes urls.

![image](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/122646722/2ff4f4c2-8b5c-4f86-b739-67a290ef59d9)

### ¿Qué es un Segmentation Fault?
Un segmentation fault ocurre en las siguientes situaciones típicas:

- Acceder a una dirección de memoria que no pertenece al espacio de direcciones del programa.
- Intentar escribir en una región de memoria de solo lectura.
- Referenciar un puntero nulo o no inicializado.

Cuando un programa causa un segmentation fault, el kernel de Linux maneja la situación de la siguiente manera:

- *__Detección del Error:__* El hardware de la CPU detecta el acceso ilegal a la memoria y genera una excepción, conocida como "page fault".

- *__Generación de la Señal:__* El kernel intercepta esta excepción y genera una señal SIGSEGV (señal de segmentación) para el proceso que causó el fallo.

- *__Terminar el Proceso:__* Por defecto, si el proceso no maneja la señal SIGSEGV, el kernel termina el proceso y opcionalmente crea un archivo core dump (volcado de memoria) que puede ser utilizado para el análisis post-mortem del estado del proceso en el momento del fallo.

- *__Registro de Mensajes:__* El kernel puede registrar un mensaje de error en los logs del sistema, indicando que el proceso fue terminado debido a un segmentation fault.

### ¿Quién carga los módulos del Kernel?
Los módulos del kernel son cargados por el administrador del sistema o automáticamente por el sistema operativo a través de varias utilidades y mecanismos.

- Administrador del Sistema (Manual):
    - Comandos: Los administradores del sistema pueden cargar módulos del kernel manualmente utilizando comandos específicos en la línea de comandos.
        - ```insmod```: Carga un módulo del kernel desde un archivo específico.
        - ```modprobe```: Carga un módulo del kernel y automáticamente resuelve y carga las dependencias necesarias.
    - Ejemplo:
      ```bash
      sudo insmod /path/to/module.ko
      sudo modprobe module_name
      ```  
- Cargadores automáticos:
    - __udev__: Un demonio del sistema que maneja eventos de hardware en tiempo real. Cuando se conecta un nuevo dispositivo, udev puede cargar automáticamente los módulos necesarios.
    - __scripts initrd/initramfs___: Durante el proceso de arranque, scripts específicos en el entorno ```initrd``` (Initial RAM Disk) o ```initramfs``` (Initial RAM File System) pueden cargar módulos necesarios para que el sistema operativo funcione correctamente desde el inicio.
- Configuraciones del Sistema:
    - Archivos de configuración: Algunos sistemas operativos permiten especificar módulos del kernel que deben cargarse automáticamente al inicio mediante archivos de configuración, como /etc/modules en sistemas basados en Debian.

## Firma de los modulos del Kernel
Como ya se explicó antes, firmar los módulos es un buen mecanismo de seguridad. A continuación se muestra cómo se firma y se inserta un módulo al sistema:

![module_sign](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/129474500/c387d101-770c-4c3c-a52e-65e3abdd5804)

Luego de esto se hace un reboot para inscribir la firma en el sistema y se procede con la inserción del módulo:

![module_ins](https://github.com/rodriguezzfran/SISCOMP_TP4/assets/129474500/ebefa158-e95a-4b3b-9848-89abe5f7cb39)

Cándo un amigo con secure boot trata de montar el modulo firmado por otra persona este tira un error, no porque no esté firmado, sino porque la firma no se encuentra dentro de las firmas habilitadas para el kernel de la persona.





