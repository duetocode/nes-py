"""The setup script for installing and distributing the nes-py package."""
import os
from glob import glob
from setuptools import setup, find_packages, Extension


# set the compiler for the C++ framework
os.environ['CC'] = 'g++'
os.environ['CCX'] = 'g++'


# The prefix name for the .so library to build. It will follow the format
# lib_nes_env.*.so where the * changes depending on the build system
LIB_NAME = 'nes_py.lib_nes_env'
# The source files for building the extension. Globs locate all the cpp files
# used by the LaiNES subproject. MANIFEST.in has to include the blanket
# "cpp" directory to ensure that the .inc file gets included too
SOURCES = glob('nes_py/nes/src/*.cpp') + glob('nes_py/nes/src/mappers/*.cpp')
# The directory pointing to header files used by the LaiNES cpp files.
# This directory has to be included using MANIFEST.in too to include the
# headers with sdist
INCLUDE_DIRS = ['nes_py/nes/include']
# Build arguments to pass to the compiler
EXTRA_COMPILE_ARGS = ['-std=c++1y', '-pipe', '-O3']
# The official extension using the name, source, headers, and build args
LIB_NES_ENV = Extension(LIB_NAME,
    sources=SOURCES,
    include_dirs=INCLUDE_DIRS,
    extra_compile_args=EXTRA_COMPILE_ARGS,
)


setup(
    packages=find_packages(exclude=['tests', '*.tests', '*.tests.*']),
    ext_modules=[LIB_NES_ENV],
    zip_safe=False,
    install_requires=[
        'gymnasium>=0.29.1',
        'numpy>=1.18.5',
        'pyglet<=1.5.21,>=1.4.0',
        'tqdm>=4.48.2',
    ],
)
