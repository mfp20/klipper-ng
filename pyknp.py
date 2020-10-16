from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

pyknp = Extension(
    name="pyknp",
    sources=["pyknp.pyx"],
)

pyknp.cython_c_in_temp = True

setup(
    name="pyknp",
    ext_modules=cythonize([pyknp], language_level=3)
)

