"""The graph for the DNN model"""

from layer import LupeLayer

class LupeGraph:
    """The graph for the DNN model"""
    def __init__(self, model):
        """Initialize the graph for the DNN model
        
        Args:
            model: The ONNX model
        """
        self.graph = model.graph
        self.node_list = {}
        self.root = LupeLayer('root', None)

        # Get the node list from the ONNX model
        node_list = {}
        for node in self.graph.node:
            node_list[node.name] = node

        self._register()

    def _register(self):
        """Use BFS to register all the nodes in the graph"""
