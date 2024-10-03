from Vertex import Vertex

"""
Graph Class
----------

This class represents the Graph modelling our courier network. 

Each Graph consists of the following properties:
    - vertices: A list of vertices comprising the graph

The class also supports the following functions:
    - add_vertex(vertex): Adds the vertex to the graph
    - remove_vertex(vertex): Removes the vertex from the graph
    - add_edge(vertex_A, vertex_B): Adds an edge between the two vertices
    - remove_edge(vertex_A, vertex_B): Removes an edge between the two vertices
    - send_message(s, t): Returns a valid path from s to t containing at most one untrusted vertex
    - check_security(s, t): Returns the set of edges that, if any are removed, would result in any s-t path having to use an untrusted edge

Your task is to complete the following functions which are marked by the TODO comment.
Note that your modifications to the structure of the Graph should be correctly updated in the underlying Vertex class!
You are free to add properties and functions to the class as long as the given signatures remain identical.
"""


class Graph():
    # These are the defined properties as described above
    vertices: 'list[Vertex]'

    def __init__(self) -> None:
        """
        The constructor for the Graph class.
        """
        self.vertices = []
        self.de = {}
        self.num = 0
        self.tmp_debug = 0
        self.debug_str = ""
    def get_name_vertex(self, path):
        return [self.de[i] for i in path]
    def get_name_edge(self, path):
        return [(self.de[i[0]], self.de[i[1]]) for i in path]
    def dfs(self, now, t, red):
        if t == now:
            return [now]
        if not now.get_is_trusted():
            red += 1
        #this path is not right
        if red > 1:
            return None
        result = None
        now.visit()
        for next_node in now.get_edges():
            if next_node.get_is_visited():
                continue
            result = self.dfs(next_node, t, red)
            if None != result:
                result.append(now)
                break
        now.leave()
        return result

    def dfs_not_double_red(self, now, t):
        if t == now:
            return [now]
        result = []
        now.visit()
        for next_node in now.get_edges():
            if next_node.get_is_visited() or (not now.get_is_trusted() and not next_node.get_is_trusted()):
                continue
            result = self.dfs_not_double_red(next_node, t)
            if [] != result:
                result.append(now)
                break
        now.leave()
        return result

    def reachable(self, now, t):
        result = self.dfs(now, t, -len(self.vertices) - 3)
        if None == result:
            result = []
        return result
        
    def all_trusted_reachable(self, now, t):
        return self.dfs(now, t, 1)

    def add_vertex(self, vertex: Vertex) -> None:
        """
        Adds the given vertex to the graph.
        If the vertex is already in the graph or is invalid, do nothing.
        :param vertex: The vertex to add to the graph.
        """
        if vertex not in self.vertices:
            self.vertices.append(vertex)
            self.de[vertex] = self.num
            self.num+=1

        # TODO Fill this in

    def remove_vertex(self, vertex: Vertex) -> None:
        self.debug_str += "remove vertex " + str(self.get_name_vertex([vertex])) + "\n"
        """
        Removes the given vertex from the graph.
        If the vertex is not in the graph or is invalid, do nothing.
        :param vertex: The vertex to remove from the graph.
        """
        tmp = self.vertices
        for i in range(len(tmp)):
            if vertex == tmp[i]:
                tmp[-1], tmp[i] = tmp[i], tmp[-1]
                other_nodes = vertex.get_edges()
                for j in range(len(other_nodes)):
                    other_nodes[j].remove_edge(vertex)
                other_nodes = []
                tmp.pop()
                break
        # TODO Fill this in

    def add_edge(self, vertex_A: Vertex, vertex_B: Vertex) -> None:
        self.debug_str += "add " + str(self.get_name_vertex([vertex_A,vertex_B])) + "\n"
        """
        Adds an edge between the two vertices.
        If adding the edge would result in the graph no longer being simple or the vertices are invalid, do nothing.
        :param vertex_A: The first vertex.
        :param vertex_B: The second vertex.
        """
        vertex_A.add_edge(vertex_B)
        vertex_B.add_edge(vertex_A)
        # TODO Fill this in

    def remove_edge(self, vertex_A: Vertex, vertex_B: Vertex) -> None:
        self.debug_str += "remove " + str(self.get_name_vertex([vertex_A,vertex_B])) + "\n"
        """
        Removes an edge between the two vertices.
        If an existing edge does not exist or the vertices are invalid, do nothing.
        :param vertex_A: The first vertex.
        :param vertex_B: The second vertex.
        """
        vertex_A.remove_edge(vertex_B)
        vertex_B.remove_edge(vertex_A)
        # TODO Fill this in

    def send_message(self, s: Vertex, t: Vertex) -> 'list[Vertex]':
        """
        Returns a valid path from s to t containing at most one untrusted vertex.
        Any such path between s and t satisfying the above condition is acceptable.
        Both s and t can be assumed to be unique and trusted vertices.
        If no such path exists, return None.
        :param s: The starting vertex.
        :param t: The ending vertex.
        :return: A valid path from s to t containing at most one untrusted vertex.
        """
        
        result = self.dfs(s, t, 0)
        if None == result:
            result = []
        return result[::-1]
        # TODO Fill this in

    def check_security(self, s: Vertex, t: Vertex) -> 'list[(Vertex, Vertex)]':
        """
        Returns the list of edges as tuples of vertices (v1, v2) such that the removal 
        of the edge (v1, v2) means a path between s and t is not possible or must use
        two or more untrusted vertices in a row. v1 and v2 must also satisfy the criteria
        that exactly one of v1 or v2 is trusted and the other untrusted.        
        Both s and t can be assumed to be unique and trusted vertices.
        :param s: The starting vertex
        :param t: The ending vertex
        :return: A list of edges which, if removed, means a path from s to t uses an untrusted edge or is no longer possible. 
        Note these edges can be returned in any order and are unordered.
        """
        if -1 == self.tmp_debug:
            st = ""
            st += "s: " + str(self.de[s]) + " t: " + str(self.de[t]) + "\n"
            for node in self.vertices:
                st += str(self.de[node]) + " : " + str(node.get_is_trusted())+"\n" + node.debug_str+ "\n"
            for node in self.vertices:
                for next_node in node.get_edges():
                    st += (str(self.de[node]) + " <-> " + str(self.de[next_node]) + "\n")
            assert False,self.debug_str+"\n"+"\n"+st 
        self.tmp_debug+=1
        
        #if None != self.all_trusted_reachable(s, t) and None == self.reachable(s, t):
        #if None != self.all_trusted_reachable(s, t) and [] != self.all_trusted_reachable(s, t):
        #    return []
        if None == self.reachable(s, t) or [] == self.reachable(s, t):
            return []
        ans = set()
        edges_visited = set()
        for node in self.vertices:
            if node.get_is_trusted():
                continue
            for next_node in node.get_edges()[:]:
                #same edge
                if (not next_node.get_is_trusted()) or (next_node, node) in edges_visited:
                    continue
                #mark edge
                edges_visited.add((node, next_node))
                
                self.remove_edge(node, next_node)
                #result = self.send_message(s, t)
                result = self.dfs_not_double_red(s, t)
                #print(self.de[node], self.de[next_node], self.get_name_vertex(result))
                if [] == result or None == result:
                    ans.add((node, next_node))
                self.add_edge(node, next_node)
        return list(ans)
                
