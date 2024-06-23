/*Итерация РКТ перехода с Nano на Мега через переходную плату; ножки в схеме Kicad в лаборатории; 23.06.24
*/
#define PIN_ENA 10 // Вывод управления скоростью вращения левого мотора №1
#define PIN_ENB 8 // Вывод управления скоростью вращения правого мотора №2
#define PIN_IN1 6 // Вывод управления направлением вращения левого мотора №1
#define PIN_IN2 11 // Вывод управления направлением вращения левого мотора №1
#define PIN_IN3 7 // Вывод управления направлением вращения правого мотора №2
#define PIN_IN4 9 // Вывод управления направлением вращения правого мотора №2



#include <ros.h>
#include <ros/time.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/UInt16.h>
#include <std_msgs/String.h>

ros::NodeHandle  nh;

String powers;
String directions;

char powersChar[50];
char directionsChar[50];

float BASE = 0.42;// начально база 0.142; факт база как расстояние между колесом и центорм 0,42
float RADIUS = 0.11;//радиус колеса фактический 0.11, начально 0.033

int power_L = 157;    //0.5 m/s = 15.15 rad/s per wheel
int power_R = 170;    //0.5 m/s = 15.15 rad/s per wheel


float kLeft = 0;      //power_L / 15.15 rad/s
float kRight = 0;     //power_R / 15.15 rad/s

int power_to_left = 0;
int power_to_right = 0;
bool L_F = 0;
bool L_B = 0;
bool R_F = 0;
bool R_B = 0;

float w_left = 0;
float w_right = 0;

float demandx=0;
float demandz=0;

double demand_speed_X = 0;
double demand_speed_Z = 0;

float leftWheelSpeed (float V, float W)
  {
      float W_l = (V-W*BASE/2)/RADIUS;
      return W_l;
  }

float rightWheelSpeed (float V, float W)
  {
    float W_r = (V+W*BASE/2)/RADIUS;
      return W_r;
  }

ros::Subscriber<geometry_msgs::Twist> sub("cmd_vel", cmd_vel_cb );
void cmd_vel_cb( const geometry_msgs::Twist& twist){
  demandx = twist.linear.x;
  demandz = twist.angular.z;
}

std_msgs::String str_msg;
ros::Publisher report("report", &str_msg);

void setup() {
  nh.initNode();
  nh.subscribe(sub);
  nh.advertise(report);
  // Установка всех управляющих пинов в режим выхода
  pinMode(PIN_ENA, OUTPUT);
  pinMode(PIN_ENB, OUTPUT);
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  // Команда остановки двум моторам
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
  digitalWrite(PIN_IN3, LOW);
  digitalWrite(PIN_IN4, LOW);

  kLeft = power_L/(0.5/RADIUS);
  kRight = power_R/(0.5/RADIUS);
}

void loop() {
// put your main code here, to run repeatedly:
demand_speed_X = demandx;
demand_speed_Z = demandz;
w_left = leftWheelSpeed(demand_speed_X, demand_speed_Z);
w_right = rightWheelSpeed(demand_speed_X, demand_speed_Z);
power_to_left = int(w_left*kLeft);
power_to_right = int(w_right*kRight);
if ((w_left >= 0) && (w_right >= 0))
{
L_F = 1;
L_B = 0;
R_F = 1;
R_B = 0;
}
if ((w_left < 0) && (w_right >= 0))// убрал два слеша перед &&
{
L_F = 0;
L_B = 1;
R_F = 1;
R_B = 0;
}
if ((w_left >= 0) && (w_right < 0))
{
L_F = 1;
L_B = 0;
R_F = 0;
R_B = 1;
}
 if ((w_left < 0) && (w_right < 0))
{
L_F = 0;
L_B = 1;
R_F = 0;
R_B = 1;
}
powers = "power_to_left = ";
powers += power_to_left;
powers += "; power_to_right = ";
powers += power_to_right;
directions = "L_F = ";
directions += L_F;
directions += ", ";
directions += "L_B = ";
directions += L_B;
directions += "; ";
directions += "R_F = ";
directions += R_F;
directions += ", ";
directions += "R_B = ";
directions += R_B;
powers.toCharArray(powersChar, 50);
directions.toCharArray(directionsChar, 50);
str_msg.data = powersChar;
report.publish( &str_msg );
str_msg.data = directionsChar;
report.publish( &str_msg );
power_to_left = abs(power_to_left);
power_to_right = abs(power_to_right);
if (power_to_left > 255)
{power_to_left = 255;}
if (power_to_right > 255){
    power_to_right = 255;}
  
  analogWrite(PIN_ENA, power_to_left); // Устанавливаем скорость 1-го мотора
  analogWrite(PIN_ENB, power_to_right); // Устанавливаем скорость 2-го мотора
    // Задаём направление для 1-го мотора
  digitalWrite(PIN_IN1, L_F);
  digitalWrite(PIN_IN2, L_B);
    // Задаём направление для 2-го мотора
  digitalWrite(PIN_IN3, R_F);
  digitalWrite(PIN_IN4, R_B);

  nh.spinOnce();
  delay(1);
}
