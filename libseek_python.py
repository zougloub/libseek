import pylibseek as lseek


class ThermalCamera(object):
    def __init__(self, k=None, c=None):
        if k: k = float(k)
        if c: c = float(c)
        lseek.init(k, c)
        (width, height) = lseek.get_dimensions()
        self.width = width
        self.height = height

    def get_frame(self, absolute=True):
        return lseek.get_frame(absolute)

    def get_flat_frame(self, absolute=True):
        return lseek.get_flat_frame(absolute)

    def __del__(self):
        lseek.deinit()
