#include "Graph.hpp"

void Graph::AddLocation(string Name) {
    if (FindIndex(Name) != -1) {                              // Check if location already exists.
        cout << "Location already exists: " << Name << endl;  // Output a message upon check returning the location already exists.
        return;                                               
    }
    Node NewNode;                                             // Else, Craete a new node
    NewNode.Name = Name;                                      // Assign a name for the new node or new location
    Nodes.push_back(NewNode);                                 // Add it to the graph
}

void Graph::AddRoad(string Start, string End, double Distance) {
    int i = FindIndex(Start);                                    // Get the index of the starting location
    int j = FindIndex(End);                                      // Get the index of the ending location

    if (i == -1 || j == -1) {                                    // Check if both the locations are present in the graph and exist
        cout << "One or both locations not found!" << endl;      // Outputting an error message upon the condition check
        return;
    }

    Edge E1;                            // Create an edge from the starting location to the ending location
    E1.Destination = End;               // Marking the destination node as the ending location
    E1.Distance = Distance;             // Marking the distance between the two nodes or locations 
    Nodes[i].Neighbors.push_back(E1);   // An edge created between two nodes results it to be added as a neighbour to the starting location as it can be directly accessed through that location or node

    Edge E2;                            // Create an edge from the ending location to the starting location
    E2.Destination = Start;             // Marking the destination node as the starting location
    E2.Distance = Distance;             // Marking the distance between the two nodes or locations 
    Nodes[j].Neighbors.push_back(E2);   // An edge created between two nodes results it to be added as a neighbour to the ending location as it can be directly accessed through that location or node
}

int Graph::FindIndex(string Name) {
    for (int i = 0; i < Nodes.size(); i++) { // Looping through all the nodes or locations
        if (Nodes[i].Name == Name)           // Check if the name matches to the current index
            return i;                        // Return its index upon condition check
    }
    return -1;                               // Returning -1 if name if not found upon condition check
}

void Graph::Print() {
    cout << "\n--- Road Network ---" << endl; // Printing the header line 
    for (int i = 0; i < Nodes.size(); i++) {  // Looping through all the nodes or locations
        cout << Nodes[i].Name << " -> ";      // Printing the location name
        for (int j = 0; j < Nodes[i].Neighbors.size(); j++) { // Looping through its neighbour
            cout << Nodes[i].Neighbors[j].Destination << " (" << Nodes[i].Neighbors[j].Distance << " KM)";  // Printing each neighbor and its distance
            if (j < Nodes[i].Neighbors.size() - 1) { // Putting in a commma between all neigbours of a spcific node except after the last neighbor
                cout << ",  ";
            }
        }
        cout << endl; 
    }
    cout << "--------------------\n" << endl; // Printing the ending line
}
