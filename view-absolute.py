#!/usr/bin/env python3
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.widgets import Slider

import libseek_python as l

mintemp, maxtemp = 0, 100

c = l.ThermalCamera()
f = c.get_frame()

fig = plt.figure()
image = plt.imshow(f, cmap="gnuplot2", animated=True)
plt.clim(mintemp, maxtemp)

ax_maxtemp = plt.axes([0.2, 0.01, 0.65, 0.03])
ax_mintemp = plt.axes([0.2, 0.05, 0.65, 0.03])
s_maxtemp = Slider(ax_maxtemp, 'Maximum', 30, 150, valinit=maxtemp)
s_mintemp = Slider(ax_mintemp, 'Minumum', 0, 40, valinit=mintemp)


def controls_update(_):
    plt.clim(s_mintemp.val, s_maxtemp.val)


def anim(_):
    f = c.get_frame()
    image.set_array(f)
    return image,

s_maxtemp.on_changed(controls_update)
s_mintemp.on_changed(controls_update)
ani = animation.FuncAnimation(fig, anim, interval=100)
plt.show()

