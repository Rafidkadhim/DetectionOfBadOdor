#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

int redLampa = 12;
int gulLampa = 11;
int buzzer = 10;
int MQ2 = 0;
int RL = 5;
int ROrenLuft = 9.83;
float Ro;

#define         KalibreringsProver    (25)    //Antal prover under kalibreringsfasen
#define         KalibreringsInterval  (500)   //Tid i millisekunder mellan proverna                                                   
#define         AntalProver           (50)   //Antal prover under mätning av gas
#define         ProvInterval          (5)    //Tid mellan varje prov av gas

#define         GAS_LPG                      (0)
#define         GAS_CO                       (1)
#define         GAS_SMOKE                    (2)

float           LPGCurve[3]  =  {2.3,0.21,-0.47};   //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve. 
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59) 
float           COCurve[3]  =  {2.3,0.72,-0.34};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15) 
float           SmokeCurve[3] ={2.3,0.53,-0.44};    //two points are taken from the curve. 
                                                    //with these two points, a line is formed which is "approximately equivalent" 
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.53), point2: (lg10000,  -0.22)                                                     



void setup() {
  pinMode(redLampa, OUTPUT);
  pinMode(gulLampa, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1,5);
  display.print("Kalibrering");
  display.display();
  Ro = MQ2Kalibrering(MQ2);   
  display.clearDisplay();
  display.setCursor(1,5); 
  display.print("Klart");
  display.setCursor(1,15);
  display.print("Ro=");
  display.print(Ro);
  display.print("kOhm");
  display.display();
  delay(5000);
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1,5);
  display.print("LPG:  ");  
  display.print(MQ2GetGasPercentage(MQ2Read(MQ2)/Ro,GAS_LPG));
  display.print("ppm");
  display.display();
  display.setCursor(1,15);
  display.print("CO:  ");
  display.print(MQ2GetGasPercentage(MQ2Read(MQ2)/Ro,GAS_CO) );
  display.print("ppm");
  display.display();
  display.setCursor(1,25);
  display.print("Smoke:  ");
  display.print(MQ2GetGasPercentage(MQ2Read(MQ2)/Ro,GAS_SMOKE) );
  display.print("ppm");
  display.display();

  if(MQ2GetGasPercentage(MQ2Read(MQ2)/Ro,GAS_LPG)>500 or MQ2GetGasPercentage(MQ2Read(MQ2)/Ro,GAS_CO)>500 or MQ2GetGasPercentage(MQ2Read(MQ2)/Ro,GAS_SMOKE)>500 ){
    digitalWrite(redLampa, HIGH);
    digitalWrite(gulLampa, LOW);
    tone(buzzer, 1000, 200);
  }
  else
  {
    digitalWrite(redLampa, LOW);
    digitalWrite(gulLampa, HIGH);
    noTone(buzzer); 
  }
}


//-----------------------------------Funktioner---------------------------------------------//

//Kalibrering av MQ2-sensorn, checkar av så att sensorn är i ren luft.
float MQ2Kalibrering(int mq_pin)
{
  int i;
  float val=0;
 
  for (i=0;i<KalibreringsProver;i++) {            
    val += MQ2Resistans(analogRead(mq_pin));
    delay(KalibreringsInterval);
  }
  val = val/KalibreringsProver;                   
 
  val = val/ROrenLuft;                       
 
  return val; 
}

//Resistans uträkning för MQ2-sensorn
float MQ2Resistans(int raw_adc)
{
  return ( ((float)RL*(1023-raw_adc)/raw_adc));
}

//
float MQ2Read(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<ProvInterval;i++) {
    rs += MQ2Resistans(analogRead(mq_pin));
    delay(AntalProver);
  }
 
  rs = rs/ProvInterval;
 
  return rs;  
}

//Räknar ut ppm av den specifika gasen
int MQ2GetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQ2GetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQ2GetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQ2GetPercentage(rs_ro_ratio,SmokeCurve);
  }    
 
  return 0;
}

//
int  MQ2GetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
