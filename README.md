# PROYECTO HORUS: Cámara de alerta de distanciamiento social COVID-19

Código demo en C de ARDUINO para el consumo de la API de reconocimiento (Proyecto Horus)

El Proyecto Horus consiste en una API REST que permite de forma simple identificar imagenes via redes neuronales.
# De que trata este proyecto

Esta es una nueva demo de utilización de las APIs de Proyecto Horus consumidas desde una ESP32 las cuales son combinadas con un Shield Arduino de nombre Dfplayer para alertar a las personas muy proximas entre si que se encuentren haciendo fila, o bien frente a una góndola de supermercado que deben respetar el distanciamiento social mientras prevalezca la alerta sanitaria por COVID.

# Como cargar el codigo en la ESP32-CAM

Para copiar el codigo a la ESP32-Cam se requiere de un adaptador USB-TTL el cual ira conectado de la siguiente manera:

![Conexion entre TTL y ESP32-Cam](Conexionado.jpg)

La configuración del entorno arduino para la carga sera:

![Configuracion en entorno Arduino](Config_Arduino.png)


Nota: Si hay problemas para subir el codigo a la placa y todo parace corresponder correctamente debera probar invirtiendo el RX y el TX de la placa TTL.


# Conexionado entre la ESP32-cam y el MP3 Player

![Esquema de conexionado entre el mp3 y la esp32](Esquema.jpg)

Una vez todo conectado se debe descomprimir el contenido del archivo ZIP en una tarjeta SD y luego instalarla en el reproductor MP3

La URL a usar en el codigo de ejemplo es:
https://server1.proyectohorus.com.ar

El usuario, Password y Perfil se obtienen en https://www.proyectohorus.com.ar.

Ejemplo de como usar el administrador aca:

https://www.youtube.com/watch?v=pf7yy0KpRks&t=3s

# Contactanos en:
https://www.linkedin.com/company/35599193/admin/

# Seguime en:
https://www.linkedin.com/in/fernando-p-maniglia/

# Conocenos más en:
https://www.seamansrl.com.ar
