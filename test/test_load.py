from geodesk import *

def test_load():
    world = Features("c:\\geodesk\\tests\\world.gol")
    world.load()