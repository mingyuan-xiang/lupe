"""The graph for the DNN model

The plot is kind of broken, but I'll probably fix it in the future.

"""

import json

from .layer.layer_type import LupeType, get_lupe_type, get_layer_constructor
from .layer.in_out import InOut

class LupeGraph:
    """The graph for the DNN model"""
    def __init__(self, name, model, out_path):
        """Initialize the graph for the DNN model

        The graph is just a dictionary of nodes and their connections.

        Args:
            model: The ONNX model
        """
        self.name = name
        self.out_path = out_path
        # For each key (node), the value is a tuple where the first element is
        # the children and the second element is the parent
        self.graph = {}
        self.node_list = {}

        self._register_weights(model)

        # Get the node dictionary from the ONNX model
        self._register(model)

    def _register_weights(self, model):
        """Register the weights"""
        weights = model.graph.initializer

        for w in weights:
            # weights
            if "weight" in w.name:
                wei = get_layer_constructor(LupeType.WEIGHT)(w, None)
                self.node_list[wei.name] = wei
            # biases
            elif "bias" in w.name:
                bias = get_layer_constructor(LupeType.BIAS)(w, None)
                self.node_list[bias.name] = bias

    def _register(self, model):
        """Use BFS to register all the nodes in the graph
        
        Args:
            model: The ONNX model
            dims: The input size
        """

        # Add the input node
        for node in model.graph.input:
            i = InOut(node, None)
            self.node_list[i.name] = i
            self.graph[i.name] = {"parents" : [], "children" : []}

        # Add the output node
        for node in model.graph.output:
            o = InOut(node, None)
            self.node_list[o.name] = o

        for node in model.graph.node:
            self._add_layer(node)

    def _add_layer(self, node):
        """Get the layer from the node

        Args:
            node: The ONNX node
            graph: complete graph as a dictionary
            node_list: the dictionary of nodes

        Returns:
            The dictionary of layers
        """

        lupe_type = get_lupe_type(node.op_type)

        layer = get_layer_constructor(lupe_type)(node, self.node_list)
        name = layer.name
        self.node_list[name] = layer
        self.graph[name] = {"parents" : [], "children" : []}

        # Assign this node to be the child of the input nodes
        for input_node in node.input:
            if input_node in self.graph:
                self.graph[input_node]["children"].append(name)

        # Assign parents to this node
        self.graph[name]["parents"] += list(node.input)

        # Assign model outputs
        for output_node in node.output:
            if output_node in self.node_list:
                self.graph[output_node] = {
                    "parents" : [name],
                    "children" : []
                }
                self.graph[name]["children"].append(output_node)

    def print(self):
        """Print the graph"""
        print("====================================", end="")
        print(f" {self.name} ", end="")
        print("====================================")
        print()
        for node in self.graph:
            self.node_list[node].print()
            print()

    def load_opt_config(self, config):
        """Load the optimization configuration"""
        with open(config, "r", encoding="utf-8") as file:
            opt_config = json.load(file)
            print(opt_config)

if __name__ == "__main__":
    import onnx
    from onnx import checker

    # Load the ONNX model
    onnx_model = onnx.load("models/onnx/LeNet.onnx")

    # Convert ONNX model into a dictionary
    model_dict = {
        "graph": onnx_model.graph,
        "model_metadata": onnx_model.metadata_props,
        "model_ir_version": onnx_model.ir_version,
        "model_producer_name": onnx_model.producer_name
    }

    checker.check_model(onnx_model)

    graph = LupeGraph("LeNet", onnx_model, "")
    graph.print()
