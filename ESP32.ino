// EL SIGUIENTE CODIGO ES DESARROLLADO COMO EJEMPLO SIN GARANTIA PARA LA VERSION BETA DE LA API DE ANALISIS DE IMAGEN POR DEEP LEARNING CON NOMBRE CLAVE "PROYECTO HORUS".
// PARA CONOCER MAS SOBRE EL PROYECTO Y SUS AVANCES ENTRA EN HTTP://www.proyectohorus.com.ar


#include "esp_http_client.h"
#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "Arduino.h"
#include <DFMiniMp3.h>

// VALORES DE CONEXION A INTERNET
const char* ssid = "ACA VA LA SSID DE TU WIFI";
const char* password = "ACA VA LA CLAVE DE TU WIFI";

// INDICA LOS ms ENTRE ENVIO Y ENVIO DE SOLICITUD A LA API
int capture_interval = 5000;

// PARA OBTENER EL UUID DEL PERFIL COMO EN USUARIO Y LA CLAVE PODES ENTRAR EN HTTP://www.proyectohorus.com.ar Y BAJAR EL ADMINISTRADOR
String APIprofileuuid = "ACA VA EL PERFIL DE HORUS";
String APIUser = "ACA VA EL USUARIO DE TU CUENTA HORUS";
String APIPassword = "ACA VA LA CLAVE DE TU CUENTA HORUS";

String APItoken = "";

// INDICA SI SE CONECTO CORRECTAMENTE AL WIFI
bool internet_connected = false;
long current_millis;
long last_capture_millis = 0;

// INDICA SI SE RECIBIO LA SOLICITUD DESDE LA API ANTES DE ENVIAR UNA NUEVA
bool Ready = true;

// ARCHIVO DE AUDIO
int audio_file = 1;
long last_audio_millis = 0;

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// LILYGO® TTGO T-Journal ESP32 Camera Module Development Board OV2640
// #define Y2_GPIO_NUM 17
// #define Y3_GPIO_NUM 35
// #define Y4_GPIO_NUM 34
// #define Y5_GPIO_NUM 5
// #define Y6_GPIO_NUM 39
// #define Y7_GPIO_NUM 18
// #define Y8_GPIO_NUM 36
// #define Y9_GPIO_NUM 19
// #define XCLK_GPIO_NUM 27
// #define PCLK_GPIO_NUM 21
// #define VSYNC_GPIO_NUM 22
// #define HREF_GPIO_NUM 26
// #define SIOD_GPIO_NUM 25
// #define SIOC_GPIO_NUM 23
// #define PWDN_GPIO_NUM -1
// #define RESET_GPIO_NUM 15

typedef struct Detections
{
  int X;
  int Y;
  int RADIO;  
};

class Mp3Notify
{
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action)
  {
    if (source & DfMp3_PlaySources_Sd) 
    {
        Serial.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb) 
    {
        Serial.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash) 
    {
        Serial.print("Flash, ");
    }
    Serial.println(action);
  }
  static void OnError(uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);  
  }
  static void OnPlaySourceOnline(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved(DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "removed");
  }
};

DFMiniMp3<HardwareSerial, Mp3Notify> mp3(Serial);

// EN ESTA FUNCION OBTENEMOS EL TOKEN
String GetToken(String user, String passwd, String profileuuid)
{
  String Token = "";
  
  HTTPClient http;
  
  http.begin("http://server1.proyectohorus.com.ar/api/v2/functions/login"); 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST("user=" + user + "&password=" + passwd + "&profileuuid=" + profileuuid);
  
  if (httpCode > 0) 
  {
    String payload = http.getString();
    
    if (Split(payload,'|',0) == "200")
    {
      Token = "Bearer " + Split(payload,'|',1);
      Serial.println(Token);
    }
    else
    {
      Token = "";
      Serial.println(Split(payload,'|',1));
    }
  }
  
  http.end();
  
  return Token;
}

// ESTA FUNCION ES SIMILAR A SPLIT EN C# O PYTHON
String Split(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
  
  for(int i=0; i<=maxIndex && found<=index; i++)
  {
    if(data.charAt(i)==separator || i==maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// CONECTA AL WIFI
bool init_wifi()
{
  int connAttempts = 0;
  Serial.println("\r\nConnecting to: " + String(ssid));
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED ) 
  {
    delay(500);
    Serial.print(".");
    if (connAttempts > 10) 
      return false;
          
    connAttempts++;
  }
  return true;
}


// ESTE EVENTO SE EJECUTA AL ENTRAR UN RESPONSE DESDE LA API
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
  switch (evt->event_id) 
  {
    case HTTP_EVENT_ON_DATA:
    String Data = (char*)evt->data;
    String Boxes = Data.substring(0,evt->data_len);
    Detections ToMesure[30];
    
    if (Boxes != "")
    {
      String ErrorCode = Split(Boxes,'|',0);

      // SI LA API RESPONDIO CON CODIGO DE ERROR 200 SIGNIFICA QUE TODO LLEGO OK.
      if (ErrorCode == "200")
      {
        String Box = ".";
        int Personas = 0;
        
        // VERIFICO SOLO LAS PRIMERAS 30 DETECCIONES
        for (int Index = 0; Index <= 30; Index++)
        {
          // BUSCO CADA CADA DETECCION DEVUELTA
          Box = Split(Boxes,'\n',Index);
  
          if (Box != "")
          {
            int StatusCode = Split(Box,'|',1).toInt();
            float ymin = Split(Box,'|',2).toFloat();
            float xmin = Split(Box,'|',3).toFloat();
            float ymax = Split(Box,'|',4).toFloat();
            float xmax = Split(Box,'|',5).toFloat();
            String UUIDDetection = Split(Box,'|',6);
            String UUIDProfile = Split(Box,'|',7);
            float Confidence = Split(Box,'|',8).toFloat();
            
            
            if (UUIDDetection != "NOT FOUND" and UUIDDetection != "FAIL")
            {
              // SOLO AMPARO LAS DETECCIONES QUE SEAN PERSONAS
              if (UUIDDetection == "07bd1b29563911eabb289c5a44391055") 
              {
                int RADIO = (((xmax - xmin) * 1000)) / 2;
                int X = (xmin + (((xmax - xmin)) / 2)) * 1000;
                int Y = (ymin + (((ymax - ymin)) / 2)) * 1000;

                ToMesure[Index].X = X;
                ToMesure[Index].Y = Y;
                ToMesure[Index].RADIO = RADIO;

                Personas++;
              }
            }
            else
            {
              // SI SE DEVOLVIO COMO OBJETO NO RECONOCIDO SIGNIFICA QUE SOLO HAY UN OBJETO Y NO ES VALIDO POR LO QUE SALGO DEL LOOP.
              break;  
            }
          }
          else
          {
            // SI NO SE OBTUVO DATOS ASUMO LLEGAR AL FINAL DE LA LISTA Y SALGO DEL LOOP.
            break;
          }
        }

        Serial.println("Personas: " + String(Personas));

        // SI SE DETECTARON A MAS DE DOS PERSONAS EN LA TOMA SE VERIFICA LA DISTANCIA
        if (Personas >= 2)
        {
          if (Verificar(ToMesure))
            PlayAlert();
        }  
      }
      else
      {
        // SI NO LLEGO CON CODIGO 200 IMPLICA QUE ALGO OCURRIO Y PONEMOS EN CERO LA VARIABLE DE TOKEN PARA QUE INTENTE RECONECTAR
        if (ErrorCode.toInt() <= 500)
          APItoken = "";
      }
      
      Ready = true;
    }
    break;
  }
  return ESP_OK;
}

// REPRODUZCO AUDIO DE ADEVERTENCIA
void PlayAlert()
{
  // VERIFICAR SI PASO MENOS DE 3 VECES EL TIEMPO ENTRE RECONOCIMIENTOS, DE SER ASI CAMBIA EL AUDIO.
  if (millis() - last_audio_millis < capture_interval * 3)
  {
    // SI LLEGUE AL ULTIMO AUDIO EMPIEZO DE CERO
    if (audio_file > 5)
      audio_file = 1;
    else
      audio_file++;
  }
  else
  {
    audio_file = 1;
  }

  mp3.playMp3FolderTrack(audio_file); waitMilliseconds(12000);
  
  long last_audio_millis = millis();
}

// VERIFICADOR DE DISTANCIAS
bool Verificar(Detections *ToMesure)
{
  // VERIFICO SOLO LAS PRIMERAS 30 DETECCIONES
  for (int Index = 0; Index <= 30; Index++)
  {
    // HAGO UN SICLO ANIDADO EN EL CUAL VERIFICO CADA ELEMENTO DE LA LISTA CON LOS DEMAS
    int X1 = ToMesure[Index].X;
    int Y1 = ToMesure[Index].Y;
    int RADIO = ToMesure[Index].RADIO;
    
    for (int SubIndex = Index + 1; Index <= 30; Index++)
    {
      int X2 = ToMesure[SubIndex].X;
      int Y2 = ToMesure[SubIndex].Y;

      // CALCULO LA DISTANCIA
      int Distancia = sqrt(pow(((X1 - X2) + (Y1 - Y2)),2));

      // SI LA DISTANCIA ES MENOR A 2.5 VECES EL ANCHO DE LA DETECCION MAS O MENOS 1,5 METROS LO TOMO COMO CERCA.
      if (Distancia < (RADIO * (1.5 * 2)))
        return true;
    }
  }

  return false;
}

// ENVIA LA IMAGEN CAPTURADA DE LA CAMARA A LA API DE HORUS
static esp_err_t take_send_photo()
{
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  
  fb = esp_camera_fb_get();
  
  if (!fb) 
  {
    Serial.println("Camera capture failed");
    return ESP_FAIL;
  }
  
  esp_http_client_handle_t http_client;
  esp_http_client_config_t config_client = {0};
  
  config_client.url = "http://server1.proyectohorus.com.ar/api/v2/functions/object/detection?responseformat=pipe";
  config_client.event_handler = _http_event_handler;
  config_client.method = HTTP_METHOD_POST;
  
  http_client = esp_http_client_init(&config_client);
  
  esp_http_client_set_post_field(http_client, (const char *)fb->buf, fb->len);
  esp_http_client_set_header(http_client, "Content-Type", "image/jpg");
  
  esp_http_client_set_header(http_client, "Authorization",  APItoken.c_str());
  
  esp_err_t err = esp_http_client_perform(http_client);
  
  esp_http_client_cleanup(http_client);
  
  esp_camera_fb_return(fb);
}

void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    // calling mp3.loop() periodically allows for notifications 
    // to be handled without interrupts
    mp3.loop(); 
    delay(1);
  }
}

void setup()
{
  Serial.begin(115200);
  
  if (init_wifi()) 
  { 
    internet_connected = true;
    Serial.println("Internet connected");
  }
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SXGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  esp_err_t err = esp_camera_init(&config);
  
  if (err != ESP_OK) 
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
   mp3.begin();
  
  uint16_t volume = mp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  mp3.setVolume(24);
  
  uint16_t count = mp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
}

void loop()
{
  // SI EL TOKEN ESTA EN NULO SOLICITO UN TOKEN
  if (APItoken == "")
  {
    APItoken = GetToken(APIUser, APIPassword, APIprofileuuid);
  }
  else
  {
    // SI TENGO TOKEN HAGO MI SOLICITUD DE ANALISIS A LA API
    
    current_millis = millis();
    if (current_millis - last_capture_millis > capture_interval) 
    { 
      last_capture_millis = millis();
      if (Ready == true)
      {
        Ready =  false;
        take_send_photo();
      }
    }
  }
}
