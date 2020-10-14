import pytest
import os

def SimpleThing():
    os.system("%s %s" % (os.environ['FEXLoader'], os.environ['FEXArgs']))
    print("Yep")

def test_local_execute(benchmark):
    benchmark(SimpleThing)
