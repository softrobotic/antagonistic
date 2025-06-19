# -*- coding: utf-8 -*-
"""
Setup file for antagonistic package.
@author: Diogo Fonseca, 2025
"""

from setuptools import setup, find_packages

setup(
    name='antagonistic',
    version='0.1.0',
    description='Python interface for Antagonistic Test Bench',
    author='Diogo',
    packages=find_packages(),
    install_requires=[
        'pyserial>=3.5',
        'numpy>=1.22',
        'matplotlib>=3.0'
    ],
)