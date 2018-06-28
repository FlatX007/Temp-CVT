#include "TM1651.h"

//***************************************** Можно менять настройки *******************************************
#define dCLK 3 // пин TM1651     
#define dDIO 2 // пин TM1651     
#define dTEMP_TCM_ANALOG_PIN 0 // измерение температуры с блока управления вариатором JF011 (50 пин, зеленый провод) 
//напряжение не должно превышать 5 Вольт и на заглушенном двигателе тоже!
//
byte bMean = 1 ; // усреднение да 1, нет 0
byte bRatioMean = 20; // средне арифметический коэффициент
int iDelayReadData = (  2000  )         / 2; // интервал считывания температуры  в мс
int iBlinkDisplay  = (  1000  )         / 2; // интервал мигания в мс

//*************************************************************************************************************

byte bFirstRun = 0;
int iSym = 0, i = 0;
int iTRANS_TEMP = 0;
bool bMoreTimer = true;
TM1651 batteryDisplay(dCLK, dDIO);

class _ClassTimer {

    int number_timer;
    long OnTime;
    long OffTime;
    int timer_state;
    unsigned long previousMillis;

  public:
    bool Ti_Stat, Ti_Stat2;

    _ClassTimer(int timer, long on, long off)
    {
      number_timer = timer;
      OnTime = on;
      OffTime = off;
      timer_state = LOW;
      previousMillis = 0;
    }
    int Update()
    {
      unsigned long currentMillis = millis();
      if ((timer_state == HIGH) && (currentMillis - previousMillis >= OnTime))
      {
        timer_state = LOW;
        previousMillis = currentMillis;
        if (number_timer == 1) Ti_Stat = !Ti_Stat;
        if (number_timer == 2) Ti_Stat2 = !Ti_Stat2;
      }
      else if ((timer_state == LOW) && (currentMillis - previousMillis >= OffTime))
      {
        timer_state = HIGH;
        previousMillis = currentMillis ;
      }
    }
};

_ClassTimer Ti (1, iBlinkDisplay, iBlinkDisplay);
_ClassTimer Ti2(2, iDelayReadData, iDelayReadData);

void setup() {

  Serial.begin(9600);
  batteryDisplay.init();
  batteryDisplay.set(BRIGHTEST);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  batteryDisplay.frame(FRAME_OFF);
}

void loop() {

  Ti.Update();
  Ti2.Update();

  if (Ti.Ti_Stat == false) {
    bMoreTimer = !bMoreTimer;
    Ti.Ti_Stat = !Ti.Ti_Stat;
  }
  if (Ti2.Ti_Stat2 == false) {
    _r_temp(); // считываем температуру с аналогового порта
    Ti2.Ti_Stat2 = !Ti2.Ti_Stat2;
  }
  _write_display_temp();
  
}

void _r_temp() {

  int analogread = map(analogRead(dTEMP_TCM_ANALOG_PIN), 0, 520, 120, 0);
  if (bFirstRun == 0)  iTRANS_TEMP = analogread; // если первый запуск, обновляем температуру сразу
  bFirstRun = 1;

  Serial.print(analogread);
  Serial.print("||" );
  Serial.print(iTRANS_TEMP);
  Serial.println("");

  if (bMean == 1) {

    iSym += analogread;
    i++;
    if (i == bRatioMean) {
      i = 0;
      iTRANS_TEMP = iSym / bRatioMean ;
      iSym = 0;
    }
  }
  else iTRANS_TEMP = analogread;
}

void _write_display_temp() {

  if (iTRANS_TEMP >= 115) { // ПЕРЕГРЕВ!!
    if (bMoreTimer == true) {
      batteryDisplay.frame(FRAME_OFF);
      batteryDisplay.displayLevel(0);
    }
    if (bMoreTimer == false) {
      batteryDisplay.frame(FRAME_ON);
      batteryDisplay.displayLevel(5);
    }
  }

  if (iTRANS_TEMP >= 100 and iTRANS_TEMP <= 114) { // перегрев
    if (bMoreTimer == true) {
      batteryDisplay.displayLevel(4);
    }
    if (bMoreTimer == false) {
      batteryDisplay.displayLevel(5);
    }
  }

  if (iTRANS_TEMP >= 91 and iTRANS_TEMP <= 99) { // повышенная температура
    batteryDisplay.frame(FRAME_ON);
    batteryDisplay.displayLevel(5);
  }

  else if (iTRANS_TEMP >= 76 and iTRANS_TEMP <= 90) { // рабочая температура!
    batteryDisplay.frame(FRAME_ON);
    batteryDisplay.displayLevel(4);
  }

  else if (iTRANS_TEMP >= 61 and iTRANS_TEMP <= 75) {
    batteryDisplay.frame(FRAME_ON);
    batteryDisplay.displayLevel(3);
  }

  else if (iTRANS_TEMP >= 41 and iTRANS_TEMP <= 60) {
    batteryDisplay.frame(FRAME_ON);
    batteryDisplay.displayLevel(2);
  }

  else if (iTRANS_TEMP >= 31 and iTRANS_TEMP <= 40) {
    batteryDisplay.frame(FRAME_ON);
    batteryDisplay.displayLevel(1);
  }

  else if (iTRANS_TEMP >= 20 and iTRANS_TEMP <= 30) {
    batteryDisplay.frame(FRAME_ON);
    batteryDisplay.displayLevel(0);
  }

  else if (iTRANS_TEMP < 20 ) {
    if (bMoreTimer == true) batteryDisplay.frame(FRAME_OFF);
    if (bMoreTimer == false) batteryDisplay.frame(FRAME_ON);
    batteryDisplay.displayLevel(0);
  }
}

