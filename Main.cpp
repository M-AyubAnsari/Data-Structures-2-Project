#include "Graph.hpp"
#include "Dijkstra.hpp"

int main() {
    Graph G;

    // Add all locations
    G.AddLocation("Saddar");
    G.AddLocation("Clifton");
    G.AddLocation("Gulshan");
    G.AddLocation("Defence");
    G.AddLocation("North Karachi");
    G.AddLocation("Malir");

    // Add roads between locations
    G.AddRoad("Saddar","Clifton", 3.2);
    G.AddRoad("Saddar","Gulshan", 2.5);
    G.AddRoad("Clifton","Defence", 4.1);
    G.AddRoad("Gulshan","Defence", 6.0);
    G.AddRoad("Gulshan","North Karachi", 3.8);
    G.AddRoad("Defence","Malir", 2.9);
    G.AddRoad("North Karachi","Malir", 8.5);

    // Print the Graph
    G.Print();

    // Test shortest paths
    Dijkstra D;
    D.FindShortestPath(G,"Saddar","Malir");
    D.FindShortestPath(G,"North Karachi","Clifton");
    D.FindShortestPath(G,"Defence","Saddar");

    // Test unreachable location
    G.AddLocation("Isolated Island");
    D.FindShortestPath(G,"Saddar","Isolated Island");

    return 0;
}
