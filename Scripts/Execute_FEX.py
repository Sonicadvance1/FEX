import pytest
import os

def pov_ray_bench():
    os.system("%s %s /usr/bin/povray +i/usr/share/doc/povray/examples/advanced/biscuit.pov +L/usr/share/povray-3.7/include/ -f -d +w160 +h120 +a0.3 +wt1" % (os.environ['FEXLoader'], os.environ['FEXArgs']))

def test_povray(benchmark):
    benchmark(pov_ray_bench)
