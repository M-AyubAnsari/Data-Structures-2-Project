#pragma once
#include <vector>
#include <string>
#include <iostream>
using namespace std;

struct Edge { // Defining an edge struct which would provide a connection from one location to another location
    string Destination; // Name of the location or node the edge points to.
    double Distance; // Road distance between locations in Kilometres.
};


struct Node { // Defining a node struct where each node would be a location for our GIS.
    string Name; // Name of the node or location.
    vector<Edge> Neighbors; // All directly reachable nodes from this specific node or location
};

class Graph { 
public:
    vector<Node> Nodes;
    void AddLocation(string Name); // This function adds a new location to the graph.
    void AddRoad(string Start, string End, double Distance); // This function adds a two way road or an edge between two locations.
    int FindIndex(string Name); // This function returns the index of a location in Nodes[], or returns -1 if not found.
    void Print(); // This function prints the complete adjacency list for debugging.
};
