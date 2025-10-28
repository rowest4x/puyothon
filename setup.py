from setuptools import setup, Extension, find_packages
import numpy

ext = Extension(
    "puyothon.puyothon", # パッケージ名.モジュール名
    sources=["src/puyothon/puyo_module.c", "src/puyothon/puyo_func.c"],
    include_dirs=[numpy.get_include()],
)

setup(
    name="puyothon",
    version="1.0",
    description="Python Package with a C extension for Puyo Puyo",
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    ext_modules=[ext],
    zip_safe=False,
)