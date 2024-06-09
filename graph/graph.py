"""The graph for the DNN model

The plot is kind of broken, but I'll probably fix it in the future.

"""

from collections import OrderedDict

from onnx import shape_inference

from .layer.layer_type import LupeType, get_lupe_type, get_layer_constructor
from .layer.in_out import InOut
from .layer.layer_utils import name_conversion

class LupeGraph:
    """The graph for the DNN model"""
    def __init__(self, name, model, out_path, opt_config):
        """Initialize the graph for the DNN model

        The graph is just a dictionary of nodes and their connections.

        Args:
            model: The ONNX model
            opt_config: optimization configuration
        """
        self.name = name
        self.out_path = out_path
        # For each key (node), the value is a tuple where the first element is
        # the children and the second element is the parent
        self.graph = OrderedDict()
        self.node_list = OrderedDict()
        self.opt_config = opt_config

        # Infer the shapes of the model
        model = shape_inference.infer_shapes(model)

        # Register the weights first
        self._register_weights(model)

        # Get the node dictionary from the ONNX model
        self._register(model)

        self.add_buffer_list = self._register_add()

    def _register_weights(self, model):
        """Register tensors only in the node_list"""
        weights = model.graph.initializer

        for w in weights:
            wei = get_layer_constructor(LupeType.WEIGHT)(w, None, None, self.opt_config)
            self.node_list[wei.name] = wei

        for node in model.graph.node:
            if "Constant" in node.name:
                lupe_type = get_lupe_type(node.op_type)

                layer = get_layer_constructor(lupe_type)(node, None, None, self.opt_config)
                name = layer.name
                self.node_list[name] = layer

    def _register(self, model):
        """Use BFS to register all the nodes in the graph
        
        Args:
            model: The ONNX model
            dims: The input size
        """

        # Assume there will only be one input and one output
        if len(model.graph.input) != 1 or len(model.graph.output) != 1:
            raise ValueError("There should only be one input or output")
        # Add the input node
        node = model.graph.input[0]
        i = InOut(node, None, None, self.opt_config, io="input")
        self.node_list[i.name] = i
        self.graph[i.name] = {"parents" : [], "children" : []}
        self.input_name = i.name

        # Add the output node
        node = model.graph.output[0]
        o = InOut(node, None, None, self.opt_config, io="output")
        self.node_list[o.name] = o
        self.output_name = o.name

        for node in model.graph.node:
            if "Constant" in node.name:
                # Skip the constant node
                continue
            self._add_layer(node, model)

    def _add_layer(self, node, model):
        """Get the layer from the node

        Args:
            node: The ONNX node
            graph: complete graph as a dictionary
            node_list: the dictionary of nodes

        Returns:
            The dictionary of layers
        """

        lupe_type = get_lupe_type(node.op_type)

        layer = get_layer_constructor(lupe_type)(node, model, self.node_list, self.opt_config)
        name = layer.name
        self.node_list[name] = layer
        self.graph[name] = {"parents" : [], "children" : []}

        # Assign this node to be the child of the input nodes
        for input_node in node.input:
            if input_node in self.graph:
                self.graph[input_node]["children"].append(name)

        # Assign parents to this node
        self.graph[name]["parents"] += [name_conversion(i) for i in node.input]

        # Assign model outputs
        for output_node in node.output:
            if output_node in self.node_list:
                self.graph[output_node] = {
                    "parents" : [name],
                    "children" : []
                }
                self.graph[name]["children"].append(output_node)

    def _register_add(self):
        """Register the layers to be stored in the extra add buffer"""
        def get_previous_key(d, key):
            # Get previous key
            keys = list(d.keys())
            key_index = keys.index(key)
            if key_index > 0:
                return keys[key_index - 1]
            return None

        add_buffer_list = OrderedDict()
        for n in self.graph:
            if "Add" in n:
                prev = get_previous_key(self.graph, n)
                if prev is None:
                    raise AssertionError("The Add has not previous layer")

                add_node = self.node_list[n]
                buffered_input = None
                for l in add_node.input:
                    if prev != l:
                        buffered_input = l
                        break

                add_buffer_list[n] = buffered_input

        return add_buffer_list

    def print(self):
        """Print the graph"""
        print("====================================", end="")
        print(f" {self.name} ", end="")
        print("====================================")
        print()

        for node in self.graph:
            self.node_list[node].print()
            print()

    def get_hidden_layers(self):
        """Get the names of hidden layers"""
        return list(self.graph.keys())[1:-1]

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
