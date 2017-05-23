from distutils.core import setup, Extension

pylibseek = Extension('pylibseek',
                    sources = ["pylibseek.cpp"],
                    extra_compile_args = ["--std=c++11"],
                    include_dirs = ["./"],
                    library_dirs = ["./build", "./cmake_build"],
                    libraries = ["seek", "usb-1.0"],
                    #define_macros = [("DEBUG", 1)]
                    )

setup (name = 'LibSeek Python interface',
       version = '1.0',
       description = 'Thermal Imager interface module',
       ext_modules = [pylibseek])
