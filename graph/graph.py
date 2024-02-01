"""The graph for the DNN model"""

from layer import LupeLayer
import colorsys
import numpy as np
import matplotlib.colors as mcolors
import plotly.graph_objects as go

class LupeGraph:
    """The graph for the DNN model"""
    def __init__(self, name, model):
        """Initialize the graph for the DNN model
        
        Args:
            model: The ONNX model
        """
        self.name = name
        self.graph = model.graph
        # For each key (node), the value is a list of nodes that are connected
        # to the key
        self.node_list = {}
        self.root = LupeLayer("root", None)

        # Get the node list from the ONNX model
        node_list = {}
        for node in self.graph.node:
            node_list[node.name] = node

        self._register()

    def _register(self):
        """Use BFS to register all the nodes in the graph"""

    def plot(self):
        """Plot the graph"""

        nodes = self._get_node_type_dict()
        colors = self._generate_colors(nodes.values())

        source_indices = []
        target_indices = []
        values = []
        edge_colors = []

        node_indices = {node: i for i, node in enumerate(nodes)}
        for source in self.node_list:
            for target in self.node_list:
                source_idx = node_indices[source]
                target_idx = node_indices[target]
                source_indices.append(source_idx)
                target_indices.append(target_idx)
                values.append(1)  # Default value for the connection
                edge_colors.append(self._get_lighter_color(
                    colors[nodes[source]],
                    factor=0.3, alpha=0.3
                ))

        # Create the Sankey diagram
        fig = go.Figure(data=[go.Sankey(
            node=dict(
                pad=15,
                thickness=20,
                line=dict(color="black", width=0.5),
                label=["" for _ in nodes],  # Hiding node labels
                color=list(colors.keys()),
            ),
            link=dict(
                source=source_indices,
                target=target_indices,
                value=values,
                color=edge_colors,
            ))])

        # Add legend annotations and color boxes
        for i, ty in enumerate(colors):
            fig.add_shape(
                type="rect",
                xref="paper", yref="paper",
                x0=0.01, y0=0.1 + i * 0.05 - 0.02,
                x1=0.05, y1=0.1 + i * 0.05 + 0.02,
                fillcolor=colors[ty],
                line={"width": 1}
            )
    
            # Annotation for the node name, aligned to the box boundary
            fig.add_annotation(
                x=0.06,
                y=0.08625 + i * 0.05,
                xref="paper",
                yref="paper",
                text=ty,
                showarrow=False,
                xanchor="left",
                font=dict(size=10),
            )

        # Draw a box around the legend
        fig.add_shape(
            type="rect",
            xref="paper", yref="paper",
            x0=0, y0=0.05,
            x1=0.2, y1=0.1 + len(nodes) * 0.05,
            line={"width": 1},
            fillcolor="rgba(255, 255, 255, 0)",  # transparent white background
        )

        # Update layout to make space for the legend
        fig.update_layout(
            title_text=f"The Computational Graph of {self.name}",
            font_size=10,
        )

        fig.show()

    def _generate_colors(self, types):
        """Generate a dictionary of colors for every type in the graph

        According to Chatgpt, this is supposed to color blind friendly.

        Args:
            n: The number of colors to generate
        """
        colors = []
        n = len(types)

        hues = np.linspace(0, 360, n, endpoint=False)

        for _, hue in enumerate(hues):
            if i % 2 == 0:
                # Higher saturation and lightness
                hsl_color = (hue, 80, 70)
            else:
                # Lower saturation and lightness
                hsl_color = (hue, 45, 40)

            # Convert HSL to RGB and then to Hex
            rgb_color = mcolors.hsv_to_rgb([hsl_color[0]/360, hsl_color[1]/100,
                                            hsl_color[2]/100])
            hex_color = mcolors.to_hex(rgb_color)
            colors.append(hex_color)

        dict_colors = {types[i]: colors[i] for i in range(n)}

        return dict_colors

    def _get_lighter_color(self, color, factor=0.5, alpha=0.5):
        """Create a lighter shade of the given color list for edges"""
        r, g, b = int(color[1:3], 16), int(color[3:5], 16), int(color[5:7], 16)
        h, l, s = colorsys.rgb_to_hls(r / 255.0, g / 255.0, b / 255.0)
        l = max(0, min(1, l * (1 + factor)))
        r, g, b = colorsys.hls_to_rgb(h, l, s)
        return f"rgba({int(r * 255)}, {int(g * 255)}, {int(b * 255)}, {alpha})"

    def _get_node_type_dict(self):
        """Get the dictionary of node types

        The nodes are the keys and the values are the types

        Returns:
            The dictionary of node types
        """
        return {}
