"""Assignment_2_controller_closed_encoder controller."""

# You may need to import some classes of the controller module. Ex:
#  from controller import Robot, Motor, DistanceSensor
from controller import Robot
import math
import numpy as np

# create the Robot instance.
robot = Robot()

# get the time step of the current world.
timestep = int(robot.getBasicTimeStep())

# You should insert a getDevice-like function in order to get the
# instance of a device of the robot. Something like:
#  motor = robot.getDevice('motorname')
left_motor = robot.getDevice('left wheel motor')
#left_motor.enable(timestep)
right_motor = robot.getDevice('right wheel motor')
#right_motor.enable(timestep)

#  ds = robot.getDevice('dsname')
#  ds.enable(timestep)

left_motor.setPosition(float('inf'))
right_motor.setPosition(float('inf'))
l = 0.0
r = 0.0
#left_motor.setVelocity(y)
#right_motor.setVelocity(x)
travel_distance=0.25

wheel_rad=0.025 #Searching it in Robot"MyRobot"

distance_between_wheels = (0.045*2+0.01) # 0.01 = height of the wheel, 0.045 = radius of the shape of the robot
x_n_1=-0.25
y_n_1=0.25
theta_n_1=-(math.pi/2)

point1 = [0.04, 0.25]
point2 = [-0.00, -0.09]
point3 = [0.25, -0.17]
point4 = [0.38, 0.25]

n = 0

points = [point1,point2,point3,point4]
    

# Main loop:
# - perform simulation steps until Webots is stopping the controller
while robot.step(timestep) != -1:
    #calculating X_dot, Y_dot of the robot coordinates 
        X_dot = (l*wheel_rad + r*wheel_rad)/2.0
        Y_dot = 0
        
        #calculating Theta_dot, the robot angular velocity 
        Theta_dot=(-l*wheel_rad + r*wheel_rad)/(distance_between_wheels)
        
        
        #Calculating the velocities for the world's coordinates 
        x_n_dot_i = math.cos(theta_n_1)*X_dot - math.sin(theta_n_1)*Y_dot
        y_n__dot_i = math.sin(theta_n_1)*X_dot + math.cos(theta_n_1)*Y_dot
        theta_n_dot_i = Theta_dot
        
         #solve the differential equations to find the robot's position 
        x_n=x_n_dot_i*timestep*0.001+x_n_1
        y_n=y_n__dot_i*timestep*0.001+y_n_1
        theta_n = theta_n_dot_i*timestep*0.001+theta_n_1
        
        
        print(str(x_n)+"      "+str(y_n)+"       "+str(theta_n))
        
        x_n_1 = x_n
        y_n_1 = y_n
        theta_n_1 = theta_n
        

        
        rho = np.sqrt((x_n_1-points[n][0])**2 + (y_n_1 - points[n][1])**2)
        alpha = np.arctan2((points[n][1]-y_n_1),(points[n][0]-x_n_1)) - theta_n_1
            
        #print(alpha*180/np.pi)
        #print(points[n])
        #print(n)
        print(rho)
        p1=1
        p2=5
            
        r = p1*alpha + p2*rho
        l = -p1*alpha + p2*rho
        
        left_motor.setVelocity(l) #Added to update velocity
        right_motor.setVelocity(r) #Added
        
        if (rho <= 0.05):
            if n+1 == len(points):
                l = r = 0   
                left_motor.setVelocity(l) #Added to update velocity
                right_motor.setVelocity(r)
                break 
           
            else:
                n +=1
            
                
     
        pass
        
       
           
        
            
                    
        
            
        
            
       
            
        
        
            
        
        
    

# Enter here exit cleanup code.
