// File:          test.cpp
// Date:          3/1/2025
// Description:
// Author:        Alex Armatis, Carlos Lara, Huy Huong, Juan Rosario
// Modifications:


#include <webots/Robot.hpp>
//Added
#include <webots/Motor.hpp>
#include <webots/GPS.hpp>
#include <webots/DistanceSensor.hpp>
#include <webots/Keyboard.hpp>
#define TIME_STEP 64
#define MAX_SPEED 6.28

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <sstream>
using namespace std;

// All the webots classes are defined in the "webots" namespace
using namespace webots;

// This is the main program of your controller.
// It creates an instance of your Robot instance, launches its
// function(s) and destroys it at the end of the execution.
// Note that only one instance of Robot should be created in
// a controller program.
// The arguments of the main function can be specified by the
// "controllerArgs" field of the Robot node
 
  //Variables:
int f = 0;
int b = 0;
int l = 0;
int r = 0;

int counter = 0;
int reverse_after_sensor = 0;
int where_we_are = 0; //For reverse only
int sub_r_sec;

int f_fix = 0;
int b_fix = 0;
int l_fix = 0;
int r_fix = 0;



  
  //Functions:
/*
void forward(){
  leftspeed = MAX_SPEED;
  rightspeed = MAX_SPEED;
  leftMotor->setVelocity(leftspeed);
  rightMotor->setVelocity(rightspeed);
  
}
void backward(){
  leftspeed = -MAX_SPEED;
  rightspeed = -MAX_SPEED;
  leftMotor->setVelocity(leftspeed);
  rightMotor->setVelocity(rightspeed);
}
void left(){
  leftspeed = -MAX_SPEED;
  rightspeed = MAX_SPEED;
  leftMotor->setVelocity(leftspeed);
  rightMotor->setVelocity(rightspeed);
}
void right(){
  leftspeed = MAX_SPEED;
  rightspeed = -MAX_SPEED;
  leftMotor->setVelocity(leftspeed);
  rightMotor->setVelocity(rightspeed);
}
void forward_left(){
  leftspeed = MAX_SPEED;
  rightspeed = MAX_SPEED/2;
  leftMotor->setVelocity(leftspeed);
  rightMotor->setVelocity(rightspeed);
}
void forward_right(){
  leftspeed = MAX_SPEED/2;
  rightspeed = MAX_SPEED;
  leftMotor->setVelocity(leftspeed);
  rightMotor->setVelocity(rightspeed);
}
void backward_left(){
  leftspeed = -MAX_SPEED;
  rightspeed = -MAX_SPEED/2;
  leftMotor->setVelocity(leftspeed);
  rightMotor->setVelocity(rightspeed);
}
void backward_right(){
  leftspeed = -MAX_SPEED/2;
  rightspeed = -MAX_SPEED;
  leftMotor->setVelocity(leftspeed);
  rightMotor->setVelocity(rightspeed);
}
*/
int modeSelection(Keyboard *keyboard, Robot *robot) {
  cout << "Press V for Manual Mode or N for Automatic Mode\n";
    
  int selectedMode = -1; // -1 means no valid input yet

  while (selectedMode == -1) {
      robot->step(TIME_STEP); // Step simulation to detect keys
      int key = keyboard->getKey();
        
      switch (key) {
          case 'V': {
              selectedMode = 1; // Manual mode
              cout << "Manual mode selected.\n";
              ofstream MyFile("Coordinates.txt", ios::out); //Empty it out cuz it keeps appending
              MyFile.close();
              break;
              }
          case 'N': {
              selectedMode = 0; // Automatic mode
              cout << "Automatic mode selected.\n";
              ifstream MyFile("Coordinates.txt");
              if (!MyFile){
              cout << "NO FILE NAMED THAT";
              }
              break;
              }
          default:
              if (key != -1) {
                  cout << "Invalid key! Press V, N or B.\n";
              }
      }
  }
  return selectedMode;
} 

int main(int argc, char **argv) {
  // create the Robot instance.
  Robot *robot = new Robot();

  //GPS:
  GPS *gps = robot->getGPS("gps");
  gps->enable(TIME_STEP);
  
  //Wheel Motor:
  Motor *leftMotor = robot->getMotor("left wheel motor");
  Motor *rightMotor = robot->getMotor("right wheel motor");
  leftMotor->setPosition(INFINITY);
  rightMotor->setPosition(INFINITY);
  leftMotor->setVelocity(0.0);
  rightMotor->setVelocity(0.0);
  
  //DistanceSensor:
  DistanceSensor *ds_left = robot->getDistanceSensor("ds_left");
  DistanceSensor *ds_right = robot->getDistanceSensor("ds_right");
  ds_left ->enable(TIME_STEP);
  ds_right ->enable(TIME_STEP);
  
  //Keyboard (Manual Mode)
  Keyboard *keyboard = robot->getKeyboard();
  keyboard->enable(TIME_STEP);
  
  //Random Number generator (REMOVED)
  
    
  //Deciding if you want to use Manual Mode or Automatic Mode
  int mode = modeSelection(keyboard,robot);
  cout << mode << endl;
  
  // Main loop:
  // - perform simulation steps until Webots is stopping the controller
  
  while (robot->step(TIME_STEP) != -1) {
    int key = keyboard->getKey();
    double leftspeed;
    double rightspeed;
    
    double ds_left_val = ds_left->getValue();
    double ds_right_val = ds_right->getValue();
    //cout << "Distance sensor left:" << ds_left_val << endl;
    //cout << "Distance sensor right:" << ds_right_val << endl;
    
    
    if (mode == 0){     
      ifstream MyFile("Coordinates.txt");
      string line;
      string word;
      //counter = 1;
      vector <string> data_lines;
      vector <string> data_words;
      while (getline(MyFile,line)){
        data_lines.push_back(line);
      }
      for(int i = 0; i <= data_lines.size() - 1; i++){
        stringstream ss(data_lines[i]);
        
        while(ss >> word){ //Read words one by ome
        data_words.push_back(word); //Append
        }
      }
      int actual_size = data_words.size()/5;
      
      if (counter == 0){
        for(int i = actual_size; i > 0; i--){
            int index = data_words.size() - (5*i);
            double r_x = std::stod(data_words[index]); //Applied when using vectors (into doubles)
            double r_y = std::stod(data_words[index + 1]); //Same 
            double r_z = std::stod(data_words[index + 2]); //Same
            char r_input = data_words[index + 3][0];
            int r_sec = std::stoi(data_words[index + 4]); //Same but with ints
            
            while(r_sec > 0){
              double ds_left_val = ds_left->getValue();
              double ds_right_val = ds_right->getValue();
              cout << "Distance sensor left:" << ds_left_val << endl;
              cout << "Distance sensor right:" << ds_right_val << endl;
              cout << r_x << " " << r_y << " " << r_z << " " << r_input << " " << r_sec << " " << where_we_are << endl; //NEVER FORGET endl WHEN ITERATING FILES
              if (ds_left_val >= 560 || ds_right_val >= 560){
                leftspeed = 0;
                rightspeed = 0;
                leftMotor->setVelocity(leftspeed);
                rightMotor->setVelocity(rightspeed);
                counter = 1;
                sub_r_sec = r_sec;
                break;
              }else{
                switch(r_input){
                  case 'F':
                    leftspeed = MAX_SPEED/3;
                    rightspeed = MAX_SPEED/3;
                    break;
                  case 'B':
                    leftspeed = -MAX_SPEED/3;
                    rightspeed = -MAX_SPEED/3;
                    break;
                  case 'L':
                    leftspeed = -MAX_SPEED/3;
                    rightspeed = MAX_SPEED/3;
                    break;
                  case 'R':
                    leftspeed = MAX_SPEED/3;
                    rightspeed = -MAX_SPEED/3;
                    break;
                  default:
                    leftspeed = 0;
                    rightspeed = 0;
                
              }
              leftMotor->setVelocity(leftspeed);
              rightMotor->setVelocity(rightspeed);
      
              robot->step(TIME_STEP); // Ensures the robot moves step-by-step
              r_sec--;
              
            }
            }
            //cout << where_we_are;
            where_we_are++;
        }
        leftspeed = 0;
        rightspeed = 0;
        leftMotor->setVelocity(leftspeed);
        rightMotor->setVelocity(rightspeed);
        counter = 1;
        
      }if (counter == 1){ //Iterating in reverse
          for(int i = 0; i < where_we_are; i++){
            int index = data_words.size() - (5*(i+1));
            double r_x = std::stod(data_words[index]); //Applied when using vectors (into doubles)
            double r_y = std::stod(data_words[index + 1]); //Same 
            double r_z = std::stod(data_words[index + 2]); //Same
            char r_input = data_words[index + 3][0];
            int r_sec = std::stoi(data_words[index + 4]); //Same but with ints
            int real_sec = r_sec - sub_r_sec;
            while(real_sec > 0){
              cout << "Automatic Mode on " << sub_r_sec << endl;
              cout << r_x << " " << r_y << " " << r_z << " " << r_input << " " << real_sec << endl; //NEVER FORGET endl WHEN ITERATING FILES
              switch(r_input){
                case 'F':
                  leftspeed = -MAX_SPEED/3;
                  rightspeed = -MAX_SPEED/3;
                  break;
                case 'B':
                  leftspeed = MAX_SPEED/3;
                  rightspeed = MAX_SPEED/3;
                  break;
                case 'L':
                  leftspeed = MAX_SPEED/3;
                  rightspeed = -MAX_SPEED/3;
                  break;
                case 'R':
                  leftspeed = -MAX_SPEED/3;
                  rightspeed = MAX_SPEED/3;
                  break;
                default:
                  leftspeed = 0;
                  rightspeed = 0;  
              }
              leftMotor->setVelocity(leftspeed);
              rightMotor->setVelocity(rightspeed);
      
              robot->step(TIME_STEP); // Ensures the robot moves step-by-step**
              real_sec--;
              sub_r_sec = 0;
              
            }
            //cout << where_we_are;
            
            
            }
            //where_we_are--;
        }
        leftspeed = 0;
        rightspeed = 0;
        leftMotor->setVelocity(leftspeed);
        rightMotor->setVelocity(rightspeed);
        MyFile.close();
        break;
        
    }else{
      if(mode == 1){
        const double *gps_value = gps->getValues();
        cout << "GPS Values";
        for (int i = 0; i < 3; i++){
          cout << fixed << setprecision(5) << gps_value[i] << " ";
        }
        cout << endl;
        cout << "Distance sensor left:" << ds_left_val << endl;
        cout << "Distance sensor right:" << ds_right_val << endl;
                
          ofstream MyFile("Coordinates.txt", ios::app);
          if (f == 1 || b == 1 || l == 1|| r == 1 || f_fix == 1 || b_fix == 1 || l_fix == 1 || r_fix == 1){
            for (int i = 0; i < 3; i++){
              MyFile << fixed << setprecision(5) << gps_value[i] << " ";
            }
            if (f == 1){
              MyFile << "F" << " ";
            }else if (b == 1){
              MyFile << "B" << " ";
            }
            else if (l == 1){
              MyFile << "L" << " ";
            }
            else if (r == 1){
              MyFile << "R" << " ";
            }
            else if (f_fix == 1){
              MyFile << "FF" << " ";
            }else if (b_fix == 1){
              MyFile << "BF" << " ";
            }
            else if (l_fix == 1){
              MyFile << "LF" << " ";
            }
            else if (r_fix == 1){
              MyFile << "RF" << " ";
            }
            //MyFile << "\n";
          }
     
          // Close the file
          //MyFile.close();
          
        if (ds_right_val >= 360){
          leftspeed = -MAX_SPEED/3;
          rightspeed = MAX_SPEED/3;
          
          if(f_fix > 0){
            MyFile << f_fix << "\n";
          }else if(b_fix > 0){
            MyFile << b_fix << "\n";
          }else if(r_fix > 0){
            MyFile << r_fix << "\n";
          }else if(b > 0){
            MyFile << b << "\n";
          }else if(l > 0){
            MyFile << l << "\n";
          }else if (r > 0){
            MyFile << r << "\n";
          }else if (f > 0){
            MyFile << f << "\n";
          }
          f_fix = 0;
          b_fix = 0;
          l_fix++;
          r_fix = 0;
          l = 0;
          r = 0;
          f = 0;
          b = 0;    
          
        }else if (ds_left_val >= 360){  
          leftspeed = MAX_SPEED/3;
          rightspeed = -MAX_SPEED/3;
          
          if(f_fix > 0){
            MyFile << f_fix << "\n";
          }else if(b_fix > 0){
            MyFile << b_fix << "\n";
          }else if (l_fix > 0){
            MyFile << l_fix << "\n";
          }else if(b > 0){
            MyFile << b << "\n";
          }else if(l > 0){
            MyFile << l << "\n";
          }else if (r > 0){
            MyFile << r << "\n";
          }else if (f > 0){
            MyFile << f << "\n";
          }
          f_fix = 0;
          b_fix = 0;
          l_fix = 0;
          r_fix++;
          l = 0;
          r = 0;
          f = 0;
          b = 0;
          
        }else if (ds_right_val >= 360 && ds_left_val >= 360){
          leftspeed = -MAX_SPEED/3;
          rightspeed = -MAX_SPEED/3;
           
          if(f_fix > 0){
            MyFile << f_fix << "\n";
          }else if(r_fix > 0){
            MyFile << r_fix << "\n";
          }else if (l_fix > 0){
            MyFile << l_fix << "\n";
          }else if(b > 0){
            MyFile << b << "\n";
          }else if(l > 0){
            MyFile << l << "\n";
          }else if (r > 0){
            MyFile << r << "\n";
          }else if (f > 0){
            MyFile << f << "\n";
          }
          f_fix = 0;
          b_fix++;
          l_fix = 0;
          r_fix = 0;
          l = 0;
          r = 0;
          f = 0;
          b = 0;
        }else{
          switch (key){     
            case 'W': //Forward
              //forward();
              leftspeed = MAX_SPEED/3;
              rightspeed = MAX_SPEED/3;
              if (b > 0){
                MyFile << b << "\n";
              }else if (l > 0){
                MyFile << l << "\n";
              }else if (r > 0){
                MyFile << r << "\n";
              }else if(f_fix > 0){
                MyFile << f_fix << "\n";
              }else if(r_fix > 0){
                MyFile << r_fix << "\n";
              }else if (l_fix > 0){
                MyFile << l_fix << "\n";
              }else if (b_fix > 0){
                MyFile << b_fix << "\n";
              }
              f++;
              b = 0;
              l = 0;
              r = 0;
              f_fix = 0;
              b_fix = 0;
              l_fix = 0;
              r_fix = 0;
              break;
            case 'S': //Backward
              //backward()
              leftspeed = -MAX_SPEED/3;
              rightspeed = -MAX_SPEED/3;
              if (f > 0){
                MyFile << f << "\n";
              }else if (l > 0){
                MyFile << l << "\n";
              }else if (r > 0){
                MyFile << r << "\n";
              }else if(f_fix > 0){
                MyFile << f_fix << "\n";
              }else if(r_fix > 0){
                MyFile << r_fix << "\n";
              }else if (l_fix > 0){
                MyFile << l_fix << "\n";
              }else if (b_fix > 0){
                MyFile << b_fix << "\n";
              }
              f = 0;
              b++;
              l = 0;
              r = 0;
              f_fix = 0;
              b_fix = 0;
              l_fix = 0;
              r_fix = 0;
              break;
            case 'D': //Right
              //right();
              leftspeed = MAX_SPEED/3;
              rightspeed = -MAX_SPEED/3;
              if (f > 0){
                MyFile << f << "\n";
              }else if (l > 0){
                MyFile << l << "\n";
              }else if (b > 0){
                MyFile << b << "\n";
              }else if(f_fix > 0){
                MyFile << f_fix << "\n";
              }else if(r_fix > 0){
                MyFile << r_fix << "\n";
              }else if (l_fix > 0){
                MyFile << l_fix << "\n";
              }else if (b_fix > 0){
                MyFile << b_fix << "\n";
              }
              f = 0;
              b = 0;
              l = 0;
              r++;
              f_fix = 0;
              b_fix = 0;
              l_fix = 0;
              r_fix = 0;
              break;
            case 'A': //Left
              //left();
              leftspeed = -MAX_SPEED/3;
              rightspeed = MAX_SPEED/3;
              if (f > 0){
                MyFile << f << "\n";
              }else if (r > 0){
                MyFile << r << "\n";
              }else if (b > 0){
                MyFile << b << "\n";
              }else if(f_fix > 0){
                MyFile << f_fix << "\n";
              }else if(r_fix > 0){
                MyFile << r_fix << "\n";
              }else if (l_fix > 0){
                MyFile << l_fix << "\n";
              }else if (b_fix > 0){
                MyFile << b_fix << "\n";
              }
              f = 0;
              b = 0;
              l++;
              r = 0;
              f_fix = 0;
              b_fix = 0;
              l_fix = 0;
              r_fix = 0;
              break;
            default:
              leftspeed = 0.0;
              rightspeed = 0.0;
              if (f > 0){
                MyFile << f << "\n";
              }else if (b > 0){
                MyFile << b << "\n";
              }else if (l > 0){
                MyFile << l << "\n";
              }else if (r > 0){
                MyFile << r << "\n";
              }else if (f_fix > 0){
                MyFile << f_fix << "\n";
              }else if (b_fix > 0){
                MyFile << b_fix << "\n";
              }else if (r_fix > 0){
                MyFile << r_fix << "\n";
              }else if (l_fix > 0){
                MyFile << l_fix << "\n";
              }
              f = 0;
              b = 0;
              l = 0;
              r = 0;
              f_fix = 0;
              b_fix = 0;
              l_fix = 0;
              r_fix = 0;
         }
         MyFile.close();
      }
      
      }
      
      
      
    }
      //This will be used for all modes!
      leftMotor->setVelocity(leftspeed);
      rightMotor->setVelocity(rightspeed);
      cout << "Forward:" << " " << f << endl;
      cout << "Backward:" << " " << b << endl;
      cout << "Left:" << " " << l << endl;
      cout << "Right:" << " " << r << endl;
      cout << "Forward_Fix:" << " " << f_fix << endl;
      cout << "Backward_Fix:" << " " << b_fix << endl;
      cout << "Left_Fix:" << " " << l_fix << endl;
      cout << "Right_Fix:" << " " << r_fix << endl;
  };

  // Enter here exit cleanup code.

  delete robot;
  return 0;
}