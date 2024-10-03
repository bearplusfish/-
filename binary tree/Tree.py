from Node import Node

"""
Tree
----------

This class represents the Binary Tree used to model our baby mobile. 

Each Tree consists of the following properties:
    - root: The root of the Tree

The class also supports the following functions:
    - put(node, child, left_child): Adds child to the given node as the left or right child depending on the value of left_child
    - move_subtree(node_a, node_b, left_child): Move node_a to the left or right child of node_b depending on the value of left_child
    - find_max_imbalance(): Finds the node with the maximum imbalance in the tree

Your task is to complete the following functions which are marked by the TODO comment.
Note that your modifications to the structure of the tree should be correctly updated in the underlying Node class!
You are free to add properties and functions to the class as long as the given signatures remain identical.
"""


class Tree():
    # These are the defined properties as described above
    root: Node

    def __init__(self, root: Node = None) -> None:
        """
        The constructor for the Tree class.
        :param root: The root node of the Tree.
        """
        self.root = root

    def put(self, node: Node, child: Node, left_child: bool) -> None:
        """
        Adds the given child to the given node as the left or right child depending on the value of left_child.
        If a node already has a child at the indicated position, this function should do nothing.
        You are guranteed that the given node is not already part of the tree
        :param node: The node to add the child to.
        :param child: The child to add to the node.
        :param left_child: True if the child should be added to the left child, False otherwise.
        """

        # TODO Add the child to the node as the left or right child depending on the value of left_child
        if left_child:
            node.add_left_child(child)
        else:
            node.add_right_child(child)

    def move_subtree(self, node_a: Node, node_b: Node, left_child: bool) -> None:
        """
        Moves the subtree rooted at node_a to the left or right child of node_b depending on the value of left_child.
        If node_b already has a child at the indicated position, this function should do nothing
        You can safely assume that node_b is not descendent of node_a.
        :param node_a: The root of the subtree to move.
        :param node_b: The node to add the subtree to.
        :param left_child: True if the subtree should be added to the left child, False otherwise.
        """

        # TODO Move the subtree rooted at node_a to the left or right child of node_b
        if node_a.parent.left_child == node_a:
            node_a.parent.left_child = None
            node_a.parent.left_tree_weight = 0
            node_a.parent.imbalance = node_b.parent.right_tree_weight
        if node_a.parent.right_child == node_a:
            node_a.parent.right_child = None
            node_a.parent.right_tree_weight = 0
            node_a.parent.imbalance = node_b.parent.left_tree_weight

        node_a.parent = None


        if left_child:
            node_b.left_tree_weight = 0
            node_b.left_child = node_a
        else:
            node_b.right_tree_weight = 0
            node_b.right_child = node_a
        node_b.update_parent_imbalance(node_b)

    def find_max_imbalance(self) -> int:
        """
        Finds the node with the maximum imbalance in the tree.
        :return: The node with the maximum imbalance.
        """

        # TODO Find the node with the maximum imbalance
        return self.recursive_max_imbalance(self.root)

    def recursive_max_imbalance(self, node: Node):
        if node is None:
            return 0

        max_left_imbalance = self.recursive_max_imbalance(node.left_child)
        max_right_imbalance = self.recursive_max_imbalance(node.right_child)

        return max(max_left_imbalance, max_right_imbalance, node.get_imbalance())

