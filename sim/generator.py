#!/usr/bin/env python2
# -*- coding: utf-8 -*-
"""
Wrapper around FLORIS predictions.

@author: Timothy Verstraeten
@date: 28/11/2017
"""

import itertools
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats
import pickle
import time

from Parameters import FLORISParameters
from Circle_assembly import floris_assembly_opt_AEP


class FlorisWrapper:
    """
    Call object.run(yaws) to simulate wake and retrieve the power production for each turbine.
    """

    def __init__(self, turbine_positions, wind_angle=0., wind_speed=8.1):
        n_turbines = turbine_positions.shape[0]
        nrel_prop = pickle.load(open('NREL5MWCPCT.p'))
        floris = floris_assembly_opt_AEP(nTurbines=n_turbines, nDirections=1, datasize=nrel_prop.CP.size)
        floris.parameters = FLORISParameters()  # use default FLORIS parameters

        # Define site measurements
        floris.windrose_directions = wind_angle * np.ones(1)  # incoming wind direction (deg)
        self.wind_speed = wind_speed # incoming wind speed (m/s)
        floris.air_density = 1.1716
        floris.initVelocitiesTurbines = np.ones_like(floris.windrose_directions)*floris.windrose_speeds

        # Define turbine properties
        floris.curve_wind_speed = nrel_prop.wind_speed
        floris.curve_CP = nrel_prop.CP
        floris.curve_CT = nrel_prop.CT
        floris.axialInduction = 1.0/3.0 * np.ones(n_turbines)  # used only for initialization
        floris.rotorDiameter = 126.4 * np.ones(n_turbines)
        floris.rotorArea = np.pi * floris.rotorDiameter[0]**2 / 4.0 * np.ones(n_turbines)
        floris.hubHeight = 90.0 * np.ones(n_turbines)
        floris.generator_efficiency = 0.944 * np.ones(n_turbines)
        floris.turbineX, floris.turbineY = np.array(list(zip(*turbine_positions)))

        self.floris = floris
        self.randomizeWind()

    def randomizeWind(self):
        diff = 0.005
        mid = self.wind_speed - diff
        ub = mid + diff
        lb = mid - diff
        self.floris.windrose_speeds = scipy.stats.norm.rvs(mid, diff, size=1)[0]

        self.floris.windrose_speeds = min(self.floris.windrose_speeds, ub)
        self.floris.windrose_speeds = max(self.floris.windrose_speeds, lb)
        #self.floris.windrose_speeds = self.wind_speed

    def run(self, yaws):
        self.floris.yaw = yaws
        self.floris.run()

        return np.array(self.floris.floris_power_0.wt_power)

    def plot_config(self):
        for x, y, yaw in zip(self.floris.turbineX, self.floris.turbineY, self.floris.yaws):
            plt.plot([x, x + 500*np.cos(yaw*np.pi/180)], [y, y + 500*np.sin(yaw*np.pi/180)], 'r-')
            plt.plot(x, y, 'bo')
        plt.show()

use = 3

if (use == 0):
    # Grid structure
    #   (wind)>   0 1 2
    #   (wind)>   3 4 5
    h_distance = 2
    v_distance = 0.7
    turbine_grid = 500 * np.array([[0,  0],          [h_distance, 0],          [2*h_distance, 0],
                                   [0,  v_distance], [h_distance, v_distance], [2*h_distance, v_distance]])
    yaw_range1 = np.array([20, 29, 30, 35])
    yaw_range2 = np.array([30, 34, 37, 40])
    yaw_range3 = np.array([-5, -1, 0, 5])
elif (use == 1):
    # Grid structure
    #   (wind)>   0 1 2
    turbine_grid = 500 * np.array([[0,  0], [1, 0], [2, 0]])
    yaw_range1 = np.array([20, 29, 35])
    yaw_range2 = np.array([30, 34, 40])
    yaw_range3 = np.array([-5, 0, 5])
elif (use == 2):
    # Grid structure
    #   (wind)>   0 1 2 3
    #   (wind)>   4 5 6 7
    h_distance = 2.5
    v_distance = 1.0
    turbine_grid = 500 * np.array([[0,  0],          [h_distance, 0],          [2*h_distance, 0],          [3*h_distance, 0],
                                   [0,  v_distance], [h_distance, v_distance], [2*h_distance, v_distance], [3*h_distance, v_distance]])
    yaw_range1 = np.array([15, 20, 24])
    yaw_range2 = np.array([25, 30, 34])
    yaw_range3 = np.array([15, 20, 25])
    yaw_range4 = np.array([-5, 0, 5])
elif (use == 3):
    # Grid structure
    #   (wind)>     0 7
    #   (wind)>   1
    #   (wind)>     2 8
    #   (wind)>   3
    #   (wind)>     4 9
    #   (wind)>   5
    #   (wind)>     6 10
    h_distance = 2
    v_distance = 0.75
    turbine_grid = 500 * np.array([[h_distance,  0],
                                   [0, 2*v_distance],
                                   [h_distance, 3*v_distance],
                                   [0, 5*v_distance],
                                   [h_distance, 6*v_distance],
                                   [0, 7*v_distance],
                                   [h_distance, 9*v_distance],
                                   [1+h_distance, 0],
                                   [1+h_distance, 3*v_distance],
                                   [1+h_distance, 6*v_distance],
                                   [1+h_distance, 9*v_distance]])
    yaw_range1 = np.array([23, 27, 28])
    yaw_range2 = np.array([-10, -6, -1])
    yaw_range3 = np.array([-2, 1, 4])
    yaw_range4 = np.array([0])


simulator = FlorisWrapper(turbine_grid)

#def main(y1, y2, y3, y4, y5, y6, y7, y8):
def main(y1, y2, y3, y4, y5, y6, y7):
#def main(y1, y2, y3):

    yaws = np.array([
        yaw_range1[y1],
        yaw_range2[y2],
        yaw_range1[y3],
        yaw_range2[y4],
        yaw_range1[y5],
        yaw_range3[y6],
        yaw_range1[y7],
        0,
        0,
        0,
        0
    ])

    #best_yaws          = np.array([27, -1, 27, -1, 27,  1, 27,  0,  0,  0,  0])
    #lowest_single_yaws = np.array([ 23, -10,  23, -10,  23,  -2,  23,   0,   0,   0,   0])

    simulator.randomizeWind()
    q = simulator.run(yaws)
    #max_v = sum(simulator.run(best_yaws))
    #min_single_v = min(simulator.run(lowest_single_yaws))

    pp = np.copy(q[0:7])
    pp[0] += q[7]
    pp[2] += q[8]
    pp[4] += q[9]
    pp[6] += q[10]
    #print("######")
    #print([y1, y2, y3, y4, y5, y6])
    #print(yaws)
    #print(pp.tolist() + [max_v, min_single_v])
    #print("######")
    return pp.tolist() #+ [max_v, min_single_v]

def test():
    print "Running test..."
    best_power, best_yaws = float("-inf"), None
    min_power, min_single_power = float("+inf"), float("+inf")
    min_p_y = None # best_yaws = np.array([27, -1, 27, -1, 27,  1, 27,  0,  0,  0,  0])
    start = time.time()
    #best_yaws          = np.array([27, -1, 27, -1, 27,  1, 27,  0,  0,  0,  0])
    #lowest_single_yaws = np.array([ 23, -10,  23, -10,  23,  -2,  23,   0,   0,   0,   0])
    #print min(simulator.run(lowest_single_yaws))
    #return
    for yyaws in itertools.product(yaw_range1, yaw_range2,
                                   yaw_range1, yaw_range2,
                                   yaw_range1, yaw_range3,
                                   yaw_range1,
                                   yaw_range4, yaw_range4, yaw_range4, yaw_range4):
        yaws = np.array(list(yyaws))
        powers = simulator.run(yaws)
        pp = np.copy(powers[0:7])
        pp[0] += powers[7]
        pp[2] += powers[8]
        pp[4] += powers[9]
        pp[6] += powers[10]
        power = sum(powers)
        if power > best_power:
            best_power, best_yaws = power, yaws
        if power < min_power:
            min_power = power
        minpp = min(pp)
        if minpp < min_single_power:
            min_single_power, min_p_y = minpp, yaws

    print(time.time() - start)
    print(best_power, min_power, min_single_power, best_yaws, min_p_y)

if __name__ == "__main__":
    test()
