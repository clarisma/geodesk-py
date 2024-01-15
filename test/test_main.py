# Copyright (c) 2024 Clarisma / GeoDesk contributors
# SPDX-License-Identifier: LGPL-3.0-only

from geodesk import *
import pytest

if __name__ == '__main__':
    """
    features = Features('c:\\geodesk\\tests\\de.gol')
    routes = features('r[route=bicycle]')
    for rel in routes:
        print(rel)
        for member in rel:
            print(f"- {member} as {member.role}")
    """
    
    # pytest.main(["-rA", "-s", "--durations=9999"])
    # pytest.main(["test_features.py::test_query_parser", "-rA", "--durations=9999"])
    # pytest.main(["test_rtree.py::test_rtree", "-rA", "--durations=9999"])
    # pytest.main(["test_polygonizer.py::test_polygonizer", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_within.py::test_within", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_ways.py::test_waynode_match", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_box.py", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_area.py", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_map.py", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_match.py", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_crosses.py", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_map.py", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_fast_within.py", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_intersects.py", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_performance.py", "-rA", "-s", "--durations=9999"])
    pytest.main(["test_format.py::test_numeric_custom_ids", "-rA", "-s", "--durations=9999"])
    # pytest.main(["test_anonymous_nodes.py", "-rA", "-s", "--durations=9999"])


    