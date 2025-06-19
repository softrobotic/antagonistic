# -*- coding: utf-8 -*-
"""
Module with useful classes and methods for interacting with Antagonistic Test Bench
Created on Mon Dec 11 09:49:34 2023

@author: Diogo Fonseca
"""

# Imports
import serial
import time
import matplotlib.pyplot as plt
import math
import numpy as np


class Antagonistic:
    # class variables
    pass

    # constructor method with instance variables
    def __init__(self):
        self.arduino = object()  # will store serial object when handshake method is called
        # connected flag
        self.connected = False
        # 0-5000 [gf] force on left load cell
        self.force_left = 0
        # 0-5000 [gf] force on right load cell
        self.force_right = 0
        # 0-360 [deg] angle on encoder
        self.encoder_pos = 0
        self.encoder_zero = 0  # variable used to zero the angle
        # 0-500 [kPa] pressure on left ptam
        self.pressure_left = 0
        # 0-500 [kPa] pressure on right ptam
        self.pressure_right = 0
        # whole message for debugging
        self.msg = []

    # other methods
    def handshake(self, port):  # handshake with bench controller. Returns connection flag
        try:
            self.arduino = serial.Serial(
                port=port, baudrate=115200, timeout=.1)
            start_time = time.time()
            while not self.connected:
                msg = self.arduino.readline()
                self.connected = (msg == b'a\r\n')
                if time.time() - start_time > 3:
                    raise Exception
            self.arduino.write(b'a')
            time.sleep(.5)
            self.arduino.flushInput()
            print("successfully connected")
            return True
        except:
            print("connection failed")
            return False

    def update(self):  # DONE updates instance variables
        if self.connected:
            # flush serial port
            self.arduino.flushInput()
            # ask for next package of data
            self.arduino.write(b'n')
            # clear previous message
            self.msg = []
            # receive new 16 byte message over serial
            while len(self.msg) < 11:  # confirm that our package has at least 7 bytes (X,X,X,X,X\r\n)
                self.msg = self.arduino.readline()  # msg is of class 'bytes'
            # decode bytes using utf-8 into a string
            self.msg = self.msg.decode('utf-8')  # msg is of class 'str'
            # convert to string
            self.msg = list(map(int, self.msg.split(',')))
            # update instance variables with new data
            self.force_left = self.msg[0]
            self.force_right = self.msg[1]
            self.encoder_pos = ((self.msg[2] - self.encoder_zero)/4000)*360
            self.pressure_left = self.msg[3]
            self.pressure_right = self.msg[4]
        else:
            print("test bench controller is not connected")

    def tare_left(self, actuator):  # TODO
        print('tare_left method not programmed')
        pass

    def tare_right(self):  # TODO
        print('tare_right method not programmed')
        pass

    def tare_encoder(self):  # resets angle to zero on current position
        self.update()
        self.encoder_zero = self.encoder_pos
        print("Encoder value tared")
        pass

    def close_connection(self):  # TODO
        if self.connected:
            self.arduino.close()
            print('Bench connection closed')
        pass


class Processing:
    # Vertically stacked subplots
    def plot_raw_data(data):
        fig, axs = plt.subplots(3, dpi=300)
        fig.suptitle('Raw data')
        axs[0].plot(data["Time [s]"], data["Angle [deg]"])
        axs[0].set_ylabel("Angle [deg]")
        axs[0].set_xlabel("Time [s]")
        axs[1].plot(data["Time [s]"], data["Force Left [gf]"], label="Left")
        axs[1].plot(data["Time [s]"], data["Force Right [gf]"], label="Right")
        axs[1].set_ylabel("Force [gf]")
        axs[1].set_xlabel("Time [s]")
        axs[1].legend(loc="upper right")
        axs[2].plot(data["Time [s]"],
                    data["Pressure Left [kPa]"], label="Left")
        axs[2].plot(data["Time [s]"],
                    data["Pressure Right [kPa]"], label="Right")
        axs[2].set_ylabel("Pressure [kPa]")
        axs[2].set_xlabel("Time [s]")
        axs[2].legend(loc="upper right")

    # Calculate the engineering stress on the left actuator (assumming constant cross sectional area)
    def engineering_stress_left(data, diameter):
        area = math.pi * (diameter/2)**2
        force_max = max(data["Force Left [gf]"])  # [gf]
        force_max = (force_max/1000)*9.81
        stress = force_max/area
        print("Maximum Force Left = " + str(force_max) + " [N]")
        print("Maximum Stress Left = " + str(stress) + " [MPa]")
        return stress

    # Calculate the engineering stress on the right actuator (assumming constant cross sectional area)
    def engineering_stress_right(data, diameter):
        area = math.pi * (diameter/2)**2
        force_max = max(data["Force Right [gf]"])  # [gf]
        force_max = (force_max/1000)*9.81
        stress = force_max/area
        print("Maximum Force Right = " + str(force_max) + " [N]")
        print("Maximum Stress Right = " + str(stress) + " [MPa]")
        return stress

    # Calculate and plot the angular rate
    def angular_rate(data):
        angle_rate = []
        for idx in range(len(data["Time [s]"])-1):
            del_angle = data["Angle [deg]"][idx+1] - data["Angle [deg]"][idx]
            del_time = data["Time [s]"][idx+1] - data["Time [s]"][idx]
            angle_rate.append(del_angle/del_time)
        return angle_rate

    # Calculate the linear strain and strain rate. Ensure data was captured with encoder calibrated to 0deg = horizontal
    def linear_strain(data, rs, so, po):
        """
        geometric drawing is in my paper notebook 2. 
            arguments: 
                rs = dist [mm] between top actuator support and center of bench
                so = vertical dist [mm] between top and bottom supports of actuators
                op = dist [mm] between bottom actuator support and axis of rotation
        """
        # calculate strain [%] and strain rate [%/s]
        # we use numpy arrays for more efficient pointwise operations (compared with list comprehensions)
        theta = np.radians(np.array(data["Angle [deg]"]))
        beta = np.arctan(rs/so)
        alpha = np.radians(90)-theta-beta
        pq = po * np.sin(alpha)
        ro = rs / np.sin(beta)
        qo = po * np.cos(alpha)
        rq = ro - qo
        rp = np.sqrt(np.square(rq) + np.square(pq))
        strain = (rp - so) / so
        strain = strain * 100  # convert to percentage
        # calculate strain rate
        strain_rate = []
        for idx in range(len(data["Time [s]"])-1):
            del_strain = strain[idx+1] - strain[idx]
            del_time = data["Time [s]"][idx+1] - data["Time [s]"][idx]
            strain_rate.append(del_strain/del_time)
        # return results
        return strain.tolist(), strain_rate
