#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <virtuabotixRTC.h>
#include <PS2Keyboard.h>
#include <SPI.h>

//Setup Dot Matrix
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES  4
#define CLK_PIN   13  // CLK
#define DATA_PIN  11  // DIN
#define CS_PIN    10  // CS
#define BUZZER_PIN 5  // Buzzer

//Setup Keyboard
#define DATA_P 3
#define DATA_M 2

String hours;
String minutes;
String temp_hours = String(00);
String temp_minutes = String(00);
String pesan_alarm5;
char buf[40] = {""};
float ldr;
int temperature;
int state = 1;
int menu_alarm = 1;
int select_alarm;
int temp_duration = 0;
unsigned long prev_time;

MD_Parola myD = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
virtuabotixRTC myRTC(6,7,8);
PS2Keyboard myK;

struct alarm {
  bool active;
  int hours;
  int minutes;
  int duration;
};

alarm alarm1 = {false, 0, 0, 0};
alarm alarm2 = {false, 0, 0, 0};
alarm alarm3 = {false, 0, 0, 0};
alarm alarm4 = {false, 0, 0, 0};
alarm alarm5 = {false, 0, 0, 0};
alarm alarms[5] = {alarm1, alarm2, alarm3, alarm4, alarm5};


void setup() {
  Serial.begin(9600);
  myD.begin();
  myD.displayClear();
  myD.setCharSpacing(1);
  myD.displayText(buf, PA_CENTER, 50, 0, PA_PRINT, PA_NO_EFFECT);
  attachInterrupt(digitalPinToInterrupt(DATA_P), interruptHandler, FALLING);
  myK.begin(DATA_P, DATA_M, PS2Keymap_US);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  myD.setTextEffect(PA_PRINT, PA_NO_EFFECT);
  ldr = analogRead(A0);
  myD.setIntensity((ldr / 1000) * 15);
  temperature = ((float) analogRead(A1) / 1023) * 30;
  myRTC.updateTime();

  if (String(myRTC.hours).length() > 1) {
    hours = String(myRTC.hours);
  } else {
    hours = "0" + String(myRTC.hours);
  }
  if (String(myRTC.minutes).length() > 1) {
    minutes = String(myRTC.minutes);
  } else {
    minutes = "0" + String(myRTC.minutes);
  }

  if (state == 1){
    for (int i = 0; i < 5; i++) {
      if (alarms[i].hours == myRTC.hours && alarms[i].minutes == myRTC.minutes && alarms[i].active) {
        myD.displayReset();
        myD.displayClear();
        select_alarm = i + 1;
        prev_time = millis();
        state = 10;
      }
    }
    if (myRTC.seconds == 40 || myRTC.seconds == 10){
      jam_display (String (temperature));
      delay(1000);
    } 
    else {
      jam_display (hours + ":" + minutes);
      delay(500);
      jam_display (hours + " " + minutes);
      delay(500);
    }
  }
  else if (state == 2){
    jam_display ("Jam");
  }
  else if (state == 3){
    jam_display ("Alarm");
  }

  else if (state == 4) {
    cek_jam ();    
    jam_display (temp_hours + ":" + temp_minutes);
    delay(500);
    jam_display ("  :" + temp_minutes);
    delay(500);
  }
  else if (state == 5) {
    cek_jam();
    jam_display (temp_hours + ":" + temp_minutes);
    delay(500);
    jam_display (temp_hours + ":  ");
    delay(500);
  }
  else if (state == 6) {
    if (menu_alarm == 1){
      jam_display ("Alarm 1");
    }
    else if (menu_alarm == 2){
      jam_display ("Alarm 2");
    }
    else if (menu_alarm == 3){
      jam_display ("Alarm 3");
    }
    else if (menu_alarm == 4){
      jam_display ("Alarm 4");
    }
    else if (menu_alarm == 5){
      jam_display ("Alarm 5");
    }
  }
  else if (state == 7) {
    cek_jam ();    
    jam_display (temp_hours + ":" + temp_minutes);
    delay(500);
    jam_display ("  :" + temp_minutes);
    delay(500);
  }
  else if (state == 8) {
    cek_jam();
    jam_display (temp_hours + ":" + temp_minutes);
    delay(500);
    jam_display (temp_hours + ":  ");
    delay(500);
  }
  else if (state == 9) {
    jam_display (String(temp_duration) + "s");
  }
  else if (state == 10) {
    if (millis() - prev_time > (alarms[select_alarm - 1].duration * 1000)) {
      alarms[select_alarm - 1].active = false;
      state = 1;
      digitalWrite(BUZZER_PIN, LOW);
    }
    else {
      digitalWrite(BUZZER_PIN, HIGH);
      if (select_alarm == 1){
        alarm_display ("07211940000003");
      }
      else if (select_alarm == 2){
        alarm_display ("Alfan Miftah Arzaqi");
      }
      else if (select_alarm == 3){
        alarm_display ("07211940000003 Alfan Miftah Arzaqi");
      }
      else if (select_alarm == 4){
        alarm_display (String (alarms[select_alarm - 1].duration));
      }
      else if (select_alarm == 5){
        alarm_display (pesan_alarm5);
      }
    }
  }
  else if (state == 11){
    alarm_display (pesan_alarm5);
  }
}

void jam_display (String alarm) {
  String (alarm).toCharArray(buf, 40);
  myD.displayReset();
  myD.setTextBuffer(buf);
  myD.displayAnimate();
}

void alarm_display (String alarm){
  myD.setTextEffect(PA_SCROLL_LEFT, PA_SCROLL_LEFT );
  String(alarm).toCharArray(buf, 40);
  myD.setTextBuffer(buf);
  if (myD.displayAnimate()) {
    myD.displayReset();
    myD.displayClear();
  }
}

void cek_jam (){
  if (temp_hours.length() == 1) {
      temp_hours = "0" + temp_hours;
  }
  else if (temp_minutes.length() == 1) {
      temp_minutes = "0" + temp_minutes;
  }
  else if (hours.length() == 1) {
      hours = "0" + hours;
  }
  else if (minutes.length() == 1) {
    minutes = "0" + minutes;
  }
}

void interruptHandler() {
  if (state == 1) {
    if (myK.available()) {
        char k = myK.read();
        if (k == PS2_ENTER) {
          state = 2;
          temp_hours = hours;
          temp_minutes = minutes;
        }
    }
  }
  else if (state == 2 || state == 3) {
    if (myK.available()){
        char k = myK.read();
        if (k == PS2_LEFTARROW){
          state = 2;
        }
        else if (k == PS2_RIGHTARROW) {
          state = 3;
        }
        else if (k == PS2_ENTER && state == 2) {
          state = 4;
        }
        else if (k == PS2_ENTER && state == 3) {
          state = 6;
        }
        else if (k == PS2_ESC){
          state = 1;
        }
    }
  }
  else if (state == 4 || state == 5 || state == 7 || state == 8) {
    if (myK.available()){
        char k = myK.read();
        if (k == PS2_LEFTARROW && state == 5){
          state = 4;
         }
         else if (k == PS2_RIGHTARROW && state == 4) {
          state = 5;
         }
         else if (k == PS2_LEFTARROW && state == 8) {
          state = 7;
         }
         else if (k == PS2_RIGHTARROW && state == 7) {
          state = 8;
         }
        else if (k == PS2_UPARROW){
          if (state == 4 || state == 7) {
            if (temp_hours.toInt() < 23 && temp_hours.toInt() >= 0) {
              temp_hours = String(temp_hours.toInt() + 1);
            } else if (temp_hours.toInt() == 23) {
              temp_hours = String(0);
            }
          } 
          else if (state == 5 || state == 8) {
            if (temp_minutes.toInt() < 59 && temp_minutes.toInt() >= 0) {
              temp_minutes = String(temp_minutes.toInt() + 1);
            } else if (temp_minutes.toInt() == 59) {
              temp_minutes = String(0);
            }
          }
        }
        else if (k == PS2_DOWNARROW){
          if (state == 4 || state == 7) {
            if (temp_hours.toInt() <= 23 && temp_hours.toInt() > 0) {
              temp_hours = String(temp_hours.toInt() - 1);
            } else if (hours.toInt() == 0) {
              temp_hours = String(23);
            }
          } 
          else if (state == 5 || state == 8) {
            if (temp_minutes.toInt() <= 59 && temp_minutes.toInt() > 0) {
              temp_minutes = String(temp_minutes.toInt() - 1);
            } else if (temp_minutes.toInt() == 59) {
              temp_minutes = String(59);
            }
          }
        }
        else if (k == PS2_ENTER){
          if (state == 4 || state == 5){
            myRTC.setDS1302Time(00, temp_minutes.toInt(), temp_hours.toInt(), 0, 0, 0, 0000);
            temp_hours = hours;
            temp_minutes = minutes;
            state = 1;
          }
          else if (state == 7 || state == 8){
            state = 9;
          }
        }
        else if (k == PS2_ESC){
          state = 1;
        }
    }
  }
  else if (state == 6){
    if (myK.available()){
      char k = myK.read();
      if (k == PS2_DOWNARROW){
        menu_alarm = menu_alarm + 1;
        if (menu_alarm > 5){
          menu_alarm = 1;
        }
      }
      else if (k == PS2_UPARROW){
        menu_alarm = menu_alarm - 1;
        if (menu_alarm < 1) {
          menu_alarm = 5;
        }
      }
      else if (k == PS2_ENTER){
        select_alarm = menu_alarm;
        if (menu_alarm == 5){
          state = 11;
        }
        else {
          state = 7;
        }
      }
      else if (k == PS2_ESC){
        state = 1;
      }
    }
  }
  else if (state == 9){
    if (myK.available()){
      char k = myK.read();
      if (k == PS2_UPARROW) {
        temp_duration = temp_duration + 1;
      }
      else if (k == PS2_DOWNARROW){
        temp_duration = temp_duration - 1;
      }
      else if (k == PS2_ENTER) {
        if (select_alarm == 1){
          alarms[0].active = true;
          alarms[0].hours = temp_hours.toInt();
          alarms[0].minutes = temp_minutes.toInt();
          alarms[0].duration = temp_duration;
        }
        else if (select_alarm == 2){
          alarms[1].active = true;
          alarms[1].hours = temp_hours.toInt();
          alarms[1].minutes = temp_minutes.toInt();
          alarms[1].duration = temp_duration;
        }
        else if (select_alarm == 3){
          alarms[2].active = true;
          alarms[2].hours = temp_hours.toInt();
          alarms[2].minutes = temp_minutes.toInt();
          alarms[2].duration = temp_duration;
        }
        else if (select_alarm == 4){
          alarms[3].active = true;
          alarms[3].hours = temp_hours.toInt();
          alarms[3].minutes = temp_minutes.toInt();
          alarms[3].duration = temp_duration;
        }
        else if (select_alarm == 5){
          alarms[4].active = true;
          alarms[4].hours = temp_hours.toInt();
          alarms[4].minutes = temp_minutes.toInt();
          alarms[4].duration = temp_duration;
        }
        state = 1;
      }
      else if (k == PS2_ESC){
        state = 1;
      }
    }
  }
  else if (state == 11) {
    if (myK.available()){
      char k = myK.read();
      if ((k >= 'a' && k <= 'z') || (k >= 'A' && k <= 'Z')) {
        pesan_alarm5 += String(k);
        myD.displayReset();
        myD.displayClear();
      }
      else if (k == PS2_BACKSPACE) {
        pesan_alarm5.remove(pesan_alarm5.length() - 1);
      }
      else if (k == PS2_ENTER) {
        state = 7;
      }
      else if (k == PS2_ESC){
        state = 1;
      }
    }
  }
}
