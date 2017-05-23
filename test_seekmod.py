#!/usr/bin/env python3
import libseek_python as l

l.lseek.init(0.3, -500)
f = l.lseek.get_frame(True)
l.lseek.deinit()
print(f)
