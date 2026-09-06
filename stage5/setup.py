from setuptools import setup
from setuptools.dist import Distribution

try:
    from setuptools.command.bdist_wheel import bdist_wheel as _bdist_wheel
except ImportError:  # older setuptools
    from wheel.bdist_wheel import bdist_wheel as _bdist_wheel

class BinaryDistribution(Distribution):
    def has_ext_modules(self):
        return True

class bdist_wheel(_bdist_wheel):
    def get_tag(self):
        _, _, plat = super().get_tag()
        return "py3", "none", plat

setup(distclass=BinaryDistribution)
