from setuptools import setup, find_packages

setup(
    name='pyadf435x',
    packages=find_packages(),

    install_requires=['pyusb>=1.0.0'],

    author='Martin Homuth-Rosemann',
    author_email='Ho-Ro@users.noreply.github.com',
    description='A package for automating AD4350/1 boards',
    license='GPLv3',
    url='http://github.com/Ho-Ro/pyadf435x'
)
