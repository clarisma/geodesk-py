from geodesk import *

def test_load():
    world = Features("c:\\geodesk\\tests\\w2.gol")
    world.load()