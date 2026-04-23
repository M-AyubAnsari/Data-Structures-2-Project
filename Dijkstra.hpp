#pragma once
#include "Graph.hpp"
 
class Dijkstra { // Takes shortest path from source to destination and prints the path and total distance directly
public:
    void FindShortestPath(Graph& G, string Source, string Destination); // Function to find the shortest path between two locations
};
