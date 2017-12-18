# Magnetometer_Vehicle-data
Collection of vehicle data using two magnetometers and processing the data to find information about the vehicle
Car will drive past couple of magnetometers and an ultrasonic Sensor. Interested in finding following information about the vehicle
1. Speed of the vehicle
2. Length of the vehicle
3. Position of the engine in the vehicle
4. How far away is the vehicle from the curb of the road
5. Location of the vehicle when other calculations are done

Microcontroller: msp430f5529/msp430g2553  
Ultrasonic Sensor: Maxbotix LV-EZ1  
Magnetometer: MAG3110  

Disregard data based on distance of the vehicle from sensors
Try to eliminate data from vehicles coming from opposite lane or vehicle reversing  
Try to eliminate data from vehicles in another lane going in same direction
