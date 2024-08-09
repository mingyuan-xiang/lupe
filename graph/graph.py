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
    def __init__(
            self, name, model, out_path, opt_config, qf_offset=0
        ):
        """Initialize the graph for the DNN model

        The graph is just a dictionary of nodes and their connections.

        Args:
            model: The ONNX model
            opt_config: optimization configuration
        """
        self.name = name
        self.out_path = out_path
        self.graph = OrderedDict()
        self.node_list = OrderedDict()
        self.opt_config = opt_config
        self.qf_offset = qf_offset

        # Infer the shapes of the model
        model = shape_inference.infer_shapes(model)

        # Register the weights first
        self._register_weights(model)

        # Get the node dictionary from the ONNX model
        self._register(model)

        self.no_alter_list = self._register_shortcut()
        self.add_buffer_list = self._register_add()

        self.no_alter_list += self._register_flatten()

    def _register_weights(self, model):
        """Register tensors only in the node_list"""
        weights = model.graph.initializer

        for w in weights:
            wei = get_layer_constructor(LupeType.WEIGHT)(
                w, None, None, self.opt_config, qf_offset=self.qf_offset
            )
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
        self.graph[i.name] = {"parents" : []}
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

        layer = get_layer_constructor(lupe_type)(
            node, model, self.node_list, self.opt_config
        )
        name = layer.name
        self.node_list[name] = layer
        self.graph[name] = {"parents" : []}

        # Assign parents to this node
        self.graph[name]["parents"] += [name_conversion(i) for i in node.input]

        # Assign model outputs
        for output_node in node.output:
            if output_node in self.node_list:
                self.graph[output_node] = {
                    "parents" : [name],
                }

    def _register_shortcut(self):
        """Register the shortcut layers"""
        shortcut_input = {}
        shortcuts = []
        for n in self.graph:
            if "shortcut" in n:
                shortcut_input[self.node_list[n].input] = (n, self.graph[n])
                shortcuts.append(n)


        # Get the layer list of the same input as shortcuts
        lst = []
        for n in self.graph:
            if hasattr(self.node_list[n], 'input'):
                i = self.node_list[n].input
                if (isinstance(i, str) and i in shortcut_input and
                    n not in shortcuts):
                    lst.append(n)

        new_graph = OrderedDict()
        for n in self.graph:
            if n in shortcuts:
                continue

            new_graph[n] = self.graph[n]
            if n in shortcut_input:
                layer, parents = shortcut_input[n]
                new_graph[layer] = parents

        self.graph = new_graph

        return lst

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

    def _register_flatten(self):
        lst = []
        for n in self.graph:
            if 'Flatten' in n:
                lst.append(n)

        return lst

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

    def set_acceleration(self, acc_config):
        """Set acceleration method"""
        for n in self.node_list:
            if n in acc_config:
                self.node_list[n].set_acceleration(
                    acc_config[n]["acceleration"]
                )

    def need_calibration(self):
        """Check if we need calibration"""
        for n in self.node_list:
            if self.node_list[n].get_calibration() is not None:
                return True

        return False

    def update_calibration_idx(self):
        """Update calibration index"""
        for n in self.node_list:
            if self.node_list[n].get_calibration() is not None:
                self.node_list[n].next_calibration_list_idx()

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

    graph = LupeGraph("LeNet", onnx_model, "", {})
    graph.print()
