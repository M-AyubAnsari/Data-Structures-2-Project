#include "Dijkstra.hpp"

const double INF = 999999.0; // Using a large number to represent an unvisited distance.

void Dijkstra::FindShortestPath(Graph& G, string Source, string Destination) {
    int n = G.Nodes.size();                           // Defines the total number of locations or nodes in the graph           
    int SourceIndex  = G.FindIndex(Source);           // Get the index of the source location
    int DestinationIndex = G.FindIndex(Destination);  // Get the index of the destination location

    if (SourceIndex == -1 || DestinationIndex == -1) {  // Check if both the locations are present in the graph and exist
        cout << "Source or Destination not found in the graph!" << endl; // Outputting an error message upon the condition check.
        return;
    }

    vector<double> ShortestDistance(n, INF); // Shortest known distance from the source location to node i which is initially set as the large number.
    vector<bool>   Visited(n, false);        // Set to true once we've finalized the shortest path to node i from the source location
    vector<int>    Previous(n, -1);          // Index of the node we came from to reach node i

    ShortestDistance[SourceIndex] = 0.0;     // Setting the distance from the source itself as 0

    for (int Step = 0; Step < n; Step++) { // Iterate the numbe of nodes times where each iteration finalizes one node
        int Current = -1;
        for (int i = 0; i < n; i++) {      // Looping through all the nodes
            if (!Visited[i]) {             // Only considering nodes that have not been visited
                if (Current == -1 || ShortestDistance[i] < ShortestDistance[Current]) // Check for the smallest distance
                    Current = i;                                                      // Picking and assigning upon condition check
            }
        }

        if (ShortestDistance[Current] == INF) // Check if the smallest node is still infinity
            break;                            // Break as all reamining nodes are unreachable
        Visited[Current] = true;              // Mark the current node being probed by step as finalized

        for (int j = 0; j < G.Nodes[Current].Neighbors.size(); j++) { // Loop through all the nieghbours of the current node
            Edge& edge = G.Nodes[Current].Neighbors[j];               // Gettiing the edge or road between the current node and its neighbour.
            int NeighborIndex = G.FindIndex(edge.Destination);        // Getting the neighbor's index 

            double NewDistance = ShortestDistance[Current] + edge.Distance; // Calculate distance if we go through the current node

            if (NewDistance < ShortestDistance[NeighborIndex]) { // Check if the new distance outputted is the shorter route to the neighbor
                ShortestDistance[NeighborIndex] = NewDistance;   // Update the shortest distance upon conditon check
                Previous[NeighborIndex] = Current;               // Record that we came from the current.
            }
        }
    }

    cout << "\n=== Shortest Path: " << Source << " -> " << Destination << " ===" << endl;      // Print result

    if (ShortestDistance[DestinationIndex] == INF) { // Check if the destination node was reached or not
        cout << "No path found!" << endl;            // Outputting an error message upon condition check
        return;
    }

    vector<string> Path;                            // Reconstructing the path
    int Current = DestinationIndex;                 // Tracing back from the destination index
    while (Current != -1) {                         // While condtion for ifcurrent node exists
        Path.push_back(G.Nodes[Current].Name);      // Add current node to path
        Current = Previous[Current];                // Move to the node we came from 
    }

    for (int i = Path.size() - 1; i >= 0; i--) {    // Printing it in reverse as the path was built backwards
        cout << Path[i];
        if (i != 0) { 
            cout << " -> ";                 // An arrow between each node to emphasize the path except the last one
        }
    }
    cout << endl;
    cout << "Total Distance: " << ShortestDistance[DestinationIndex] << " KM" << endl;
}
