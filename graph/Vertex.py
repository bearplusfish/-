"""
Vertex Class
----------

This class represents an individual Vertex in a Graph. 

Each Vertex consists of the following properties:
    - is_trusted: boolean indicating whether this Vertex is a trusted courier or not
    - edges: A list of vertices connected to this Vertex (adjlist)

The class also supports the following functions:
    - add_edge(Vertex): Adds an edge to the Vertex. 
    - remove_edge(Vertex): Removes an edge from the Vertex
    - get_edges(): Returns a list of all the vertices connected to the Vertex
    - update_status(is_trusted): Updates the status of the Vertex to trusted or not trusted
    - get_is_trusted(): Returns whether the Vertex is trusted or not

Your task is to complete the following functions which are marked by the TODO comment.
You are free to add properties and functions to the class as long as the given signatures remain identical.
Good Luck!
"""


class Vertex():
    # These are the defined properties as described above
    is_trusted: bool
    vis: bool
    edges: 'list[Vertex]'
    debug_str: str
    def __init__(self, is_trusted: bool) -> None:
        """
        The constructor for the Vertex class.
        :param is_trusted: Whether the Vertex is trusted or not.
        """
        self.is_trusted = is_trusted
        self.edges = []
        self.vis = False
        self.debug_str = ""
    def visit(self):
        self.vis = True

    def leave(self):
        self.vis = False
    
    def get_is_visited(self):
        return self.vis
    
    def able_to_add_edge(self, v):
        if v == self or v in self.edges:
            return False
        return True

    def add_edge(self, vertex: 'Vertex') -> None:
        """
        Adds an edge to the given vertex.
        If adding the given vertex would result in the graph no 
        longer being simple or the vertex is invalid, do nothing.
        :param vertex: The vertex to add an edge to.
        """
        if self.able_to_add_edge(vertex):
            self.edges.append(vertex)
            vertex.edges.append(self)
        # TODO Fill this in

    def remove_edge(self, vertex: 'Vertex' ) -> None:
        """
        Removes the given vertex from the adjlist.
        If the vertex is invalid or does not exist, do nothing.
        :param vertex: The vertex to remove.
        """
        tmp = self.edges
        for i in range(len(tmp)):
            if tmp[i] == vertex:
                tmp[i], tmp[-1] = tmp[-1], tmp[i]
                tmp.pop()
                break

        tmp = vertex.edges
        for i in range(len(tmp)):
            if tmp[i] == self:
                tmp[i], tmp[-1] = tmp[-1], tmp[i]
                tmp.pop()
                break
        # TODO Fill this in

    def get_edges(self) -> 'list[Vertex]':
        """
        Returns the list of vertices connected to the current node.
        :return: The list of vertices connected to the current node.
        """

        return self.edges

    def update_status(self, is_trusted: bool) -> None:
        self.debug_str += "set " + str(is_trusted) + "\n"
        """
        Updates the status of whether the vertex can be trusted or not.
        """
        self.is_trusted = is_trusted
        # TODO Fill this in

    def get_is_trusted(self) -> bool:
        """
        Returns whether the vertex is trusted or not.
        :return: True if the vertex is trusted, False otherwise.
        """

        return self.is_trusted
