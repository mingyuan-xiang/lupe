"""Generates code for each layer of the graph"""

#TODO: opt_init()

def layergen(code_dir, graph, opt_config):
    """Generates code for each layer of the graph
    
    Args:
        code_dir: The directory to save the code
        graph: The graph object
        opt_config: The optimization configuration
    """
    # for node in graph.graph:
    #     code = graph.node_list[node].get_code()
