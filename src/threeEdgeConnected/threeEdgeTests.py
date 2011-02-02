import unittest
import sys

from cactus.shared.test import parseCactusSuiteTestOptions
from sonLib.bioio import system

class TestCase(unittest.TestCase):
    def test3Edge(self):
        """Run the 3-edge connected CuTests, fail if any of them fail.
        """
        system("3EdgeTests")

def main():
    parseCactusSuiteTestOptions()
    sys.argv = sys.argv[:1]
    unittest.main()
        
if __name__ == '__main__':
    main()