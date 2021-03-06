/*********************************************************************
Control del PLL 138XRL
Francisco Gonzalez Ti2LX, Francisco
franciscoti5lx@gmail.com
*********************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Altura incorrecta, arregle el archivo Adafruit_SSD1306.h!");
#endif

//defino el techado que usare
//
const byte ROWS = 4; //cuatro filas
const byte COLS = 4; //cuatro columnas
char keys[ROWS][COLS] = {
  {'D','#','0','*'},
  {'C','8','8','7'},
  {'B','6','5','4'},
  {'A','3','2','1'}
};

//-----------------------------------
// Defino los pines usados para el teclado
//
byte rowPins[ROWS] = {42, 44, 46, 48}; //Filas
byte colPins[COLS] = {34, 36, 38, 40}; //Columnas
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//-----------------------------------
float frecuencia = 27.055 ;//frecuencia de inicio del radio
int velc = 500; //velocidad lenta de cambio de frecuencia 
String frec = String(frecuencia,3); //frec tiene la frecuencia con 3 decimales
String canal;
int frecCB;
//defino los grupos de bits que componen cada BCD de control del PLL
//se requieren 3 grupos para un total de 10 líneas
#define NUM_BIN(ARRAY)    (sizeof(ARRAY) / sizeof(ARRAY[0])) //BCD menos significativo
const uint8_t   pins[]  = { 4, 5, 6, 7 };
#define NUM_BIN2(ARRAY)    (sizeof(ARRAY) / sizeof(ARRAY[0]))//BCD del medio
const uint8_t   pins2[]  = { 8, 9, 10, 11 };
#define NUM_BIN2(ARRAY)    (sizeof(ARRAY) / sizeof(ARRAY[0]))//BCD menos significativo
const uint8_t   pins3[]  = { 12, 13 };

int Ntotal;

void setup()   {                
Serial.begin(9600);
delay(200);

//para el display por default se genera el alto voltaje desde la línea de 3.3 v internamente
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // inicializa la direccion I2C  0x3D (para el 128x64)
}

//escribe los datos del BCD menos significativo a lospines
void setBCD1(uint8_t value)
{
    for ( size_t i = 0; i < NUM_BIN(pins); value >>= 1, i++ )
    {
        digitalWrite(pins[i], ((value & 1) ? HIGH : LOW));
    }
}
//Escribe los datos del BCD del centro a los pines
void setBCD2(uint8_t value)
{
    for ( size_t i = 0; i < NUM_BIN(pins2); value >>= 1, i++ )
    {
        digitalWrite(pins2[i], ((value & 1) ? HIGH : LOW));
    }
}
//Escribe los datos del BCD más significatibo a los pines
void setBCD3(uint8_t value)
{
    for ( size_t i = 0; i < NUM_BIN(pins3); value >>= 1, i++ )
    {
        digitalWrite(pins3[i], ((value & 1) ? HIGH : LOW));
    }
}
//Recupera el contenido del BCD para cada grupo
void convBCD(int value)
{
if (value>99)
  {
    int MSBy = round(value/100); // primer digito MSB
    setBCD3(MSBy);// escribe el puerto con dato mas significativo
    int MedSBy = round(value - MSBy*100); 
    int MedSBy2 = round(MedSBy/10);  // digito central 
    setBCD2(MedSBy2); // escribe puerto con dato medio
    int LSBy = round(MedSBy-MedSBy2*10);   // ultimo digito LSB
    setBCD1(LSBy);  //escribe puerto con dato menos significativo
  }else{
    setBCD3(0);// escribe el puerto con dato mas significativo en 0
    int MedSBy2L = round(value/10);  // digito central 
    setBCD2(MedSBy2L); // escribe puerto con dato medio
    int LSByL = round(value-MedSBy2L*10);   // ultimo digito LSB
    setBCD1(LSByL);  //escribe puerto con dato menos significativo
  }

}
//----------------------------------------------

void loop() {

char key = keypad.getKey();

  if (key != NO_KEY){
 // Serial.println(key);

switch (key){
    case 'C':
    frecuencia = (frecuencia + 0.010); //si esta oprimido el boton suba 10 KHz
    break;
    case 'D':
       frecuencia = (frecuencia - 0.010); //si esta oprimido el boton suba 10 KHz
      break;
    case 'B':
       frecuencia = 27.055; // va a canal 8
      break;
    case 'A':
       frecuencia = 27.455; // va a canal DX
      break;
    case '1':
       frecuencia = 26.965; // va a canal D1
      break;
          case '2':
       frecuencia = 27.405; // va a canal 40
      break;
                case '3':
       frecuencia = 27.555; // va a canal DX
      break;
  default:
      break;
  }
 
}


//-------------------------------------------------
  //Si llega a la frecuencia mínima (26.055) vaya a la máxima
  //Si llega a la frecuencia máxima (30.035) vaya a la mínima
  //Esto permite crear un círculo en el escaneo de frecuencias
  
  
  Ntotal = round((frecuencia-26.055)*100);
   if (Ntotal >= 399 ) 
  {
  frecuencia = 26.065;
  }

   if (Ntotal <= 0 ) 
  {
  frecuencia = 30.035;
  }
  //-----------------------------------------------------



Serial.println(frecuencia,3);
Serial.println(" ");
Serial.println(Ntotal);
 
  
  // escribe datos al PLL
  convBCD(Ntotal); 
  //------------------------------------------------------

  

  frecCB=int(round(frecuencia*1000));// elimina los decimales convirtiendo frec en KHz
  frec = String(frecuencia,3); //escribe la frecuencia con 3 decimales

 //Si la frecuencia coincide con un canal de CB muestrelo en pantalla
  switch (frecCB) {
    case 26965:
    //display.println("Canal 1");
    canal = "Canal 1";
    break;
    case 26975:
   //   display.println("Canal 2");
     canal = "Canal 2";
      break;
    case 26985:
//      display.println("Canal 3");
  canal = "Canal 3";
      break;  

    case 27005:
    canal = "Canal 4";
//      display.println("Canal 4");
      break;  
    case 27015:
 canal = "Canal 5";
 //     display.println("Canal 5");
      break;  
    case 27025:
    canal = "Canal 6";
//      display.println("Canal 6");
      break;  
    case 27035:
  canal = "Canal 7";
//      display.println("Canal 7");
      break;  
    case 27055:
    canal = "Canal 8";
//      display.println("Canal 8");
      break;  
    case 27065:
  //    display.println("Canal 9");
    canal = "Canal 9";
      break;  
    case 27075:
    canal = "Canal 10";
    //  display.println("Canal 10");
      break;  
    case 27085:
    canal = "Canal 11";
    //  display.println("Canal 11");
      break;  
    case 27105:
    canal = "Canal 12";
     // display.println("Canal 12");
      break;  
    case 27115:
    canal = "Canal 13";
    //  display.println("Canal 13");
      break;  
    case 27125:
    canal = "Canal 14";
    //  display.println("Canal 14");
      break;  
    case 27135:
    canal = "Canal 15";
    //  display.println("Canal 15");
      break;  
    case 27155:
    canal = "Canal 16";
    //  display.println("Canal 16");
      break;  
    case 27165:
    canal = "Canal 17";
   //   display.println("Canal 17");
      break;  
    case 27175:
    canal = "Canal 18";
     // display.println("Canal 18");
      break;  
    case 27185:
    canal = "Canal 19";
     // display.println("Canal 19");
      break;  
    case 27205:
    canal = "Canal 20";
     // display.println("Canal 20");
      break;  
    case 27215:
    canal = "Canal 21";
     // display.println("Canal 21");
      break;  
    case 27225:
    canal = "Canal 22";
     // display.println("Canal 22");
      break;  
    case 27255:
    canal = "Canal 23";
      // display.println("Canal 23");
      break;  
    case 27235:
    canal = "Canal 24";
    //  display.println("Canal 24");
      break;  
    case 27245:
    canal = "Canal 25";
    //  display.println("Canal 25");
      break;  
    case 27265:
    canal = "Canal 26";
    //  display.println("Canal 26");
      break;  
    case 27275:
    canal = "Canal 27";
    //  display.println("Canal 27");
      break;  
    case 27285:
    canal = "Canal 28";
    //  display.println("Canal 28");
      break;  
    case 27295:
    canal = "Canal 29";
    //  display.println("Canal 29");
      break;  
    case 27305:
    canal = "Canal 30";
    //  display.println("Canal 30");
      break;  
    case 27315:
    canal = "Canal 31";
     // display.println("Canal 31");
      break;  
    case 27325:
    canal = "Canal 32";
     // display.println("Canal 32");
      break;  
    case 27335:
    canal = "Canal 33";
     // display.println("Canal 33");
      break;  
    case 27345:
    canal = "Canal 34";
     // display.println("Canal 34");
      break;  
    case 27355:
    canal = "Canal 35";
     // display.println("Canal 35");
      break;  
    case 27365:
    canal = "Canal 36";
    //  display.println("Canal 36");
      break;  
    case 27375:
    canal = "Canal 37";
    //  display.println("Canal 37");
      break;  
    case 27385:
    canal = "Canal 38";
   //   display.println("Canal 38");
      break;  
    case 27395:
    canal = "Canal 39";
     // display.println("Canal 39");
      break;  
    case 27405:
     // display.println("Canal 40");
      canal = "Canal 40";
      break;  
    case 27455:
    //  display.println("DX");
      canal = "CANAL DX";
      break; 
    case 27555:
    //  display.println("DX");
      canal = "CANAL DX";
      break; 
  default:
//      display.println(String(Ntotal));
  canal = "NAVEGANTE"; //Pon aqui el nombre de tu estacion en vez de NAVEGANTE
      break;
  }
    
  //Inicializa los settings para escribir el display
  display.clearDisplay(); //borramos el display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0); //iniciamos la escritura en primer renglón 
  display.println("FRECUENCIA"); //título
  display.setCursor(0,18); // va al segundo renglon
  display.setTextSize(2);
  display.println(frec+" MHz");
  display.setCursor(0,38); // va al tercer renglón 
  display.println(canal);
  display.display(); // actualice el display
  //-----------------------------------------------------   
   

}
