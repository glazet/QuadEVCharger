#list of libraries
import time
import RPi.GPIO as GPIO

#Overview of process
'''
There are two processes in this code: main and interrupt. 
Main controls the overall flow of charging but does not change
the variables, it just runs the charging process with whatever value
the variables have.
Interrupt is triggered when the cars or power grid change. Interrupt stops main,
adjusts all the variables, (like PP to determine which cars are here and CP
if a car starts or stops), and then restarts main, restarting the charging process
with the new conditions. 
Almost all the variables are global variables so that main and interrupt can
modify them at any time. This means we will rarely have functions that return values
'''
#To do:
#Check Python version correspond with RPi's version
#Remember to reset ExitCondition after the interrupt is done


#define global variables
ExitCondition = False#the variable the interrupt sets to exit the Charging Loop
PlugArr = [0,0,0,0]#Array that holds which plug is used
PPValue = [0,0,0,0]#The voltage value from the PP ADC pins
CParr=[0,0,0,0]#Yet to be assigned
CPValue=[0,0,0,0]#The voltage value from the CP ADC pins
EVPlugIn=0#A variable that tells when at least one car is plugged in, used to
#break out of the Idle Loop
BatteryLevel=[0,0,0,0]#Array that holds battery level
ChargeTime=[0,0,0,0]#The amount of time the switching devices stay open
SwitchingDeviceState=[0,0,0,0]#Whether the switching device is open/closed

'''
MAIN SECTION
'''

if __name__ == '__main__':
    #Initialize safety interrupt

    #Initialize pilot interrupt
    GPIO.add_event_detect(#PIN NUMBER,GPIO.BOTH,callback=IntFunct,bouncetime=50)

    EVPlugIn=PlugArr[1]+PlugArr[2]+PlugArr[3]+PlugArr[4]#checks to see if any EV is plugged in
    while (EVPlugIn == 0):
        getPPVoltage()
    
    setChargingPattern()
    setControlPilot()
    setSwitchingDevices()
    while (ExitCondition == False):#while interrupt has not been triggered, continue charging cars
#end main()

def setChargingPattern():
    getCarInfo()
    # 
    #This is where the charging algorithm goes
    #

    #Should modify ChargeTime
#end setChargingPattern()

def getCarInfo():#gets info about car, like battery level and vehicle size
    #
    #Interfacing with the screen
    #

    #Should grab battery level, vehicle size from screen's controller
#end getCarInfo

def setControlPilot():
    #Should create PWM with amplitude and duty cycle, done through PWM board
#end setControlPilot()

def setSwitchingDevices():
    #Should modify SwitchingDeviceState
#end setSwitchingDevices()

'''
INTERRUPT SECTION
'''

def IntFunct():#The function that the interrupt calls
    #Should read PP,CP
    #Should adjust PlugArr
    #Should set ExitCondition = True
#end IntFunct()

def IntSafety():#In case of error, the interrupt will call this function
    #Should flash LED
    #Should demand that screen displays red
    #Should trap pointer in this function
#end IntSafety

