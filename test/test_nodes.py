def notest_nodes(features):
    streets=features("w[highway=service]")
    barriers=features("[barrier=lift_gate]")
    for street in streets:
        barrier_nodes = barriers.nodes_of(street)
        if barrier_nodes:
            print(f"{street} has {barrier_nodes.count} barriers")