/*!
* @file			ALGraph.cpp
* @author		Maojie Deng, 2200840, maojie.deng@digipen.edu
* @date			29/2/2024
* @par			Assignment 4 : ALGraph using Dijkstra's Algorithm
* @brief		This file contains the logic finding shortest ALGraph using Dijkstra's Algorithm.
*/

#include "ALGraph.h"
#include <iostream>
#include <queue>

/**
 * @brief Constructor for ALGraph class.
 * 
 * Initializes an adjacency list for a graph with a specified number of nodes.
 * Each node initially has no edges.
 * 
 * @param size The number of nodes in the graph.
 */
ALGraph::ALGraph(unsigned size) 
{
    //Initialize the AdjacentList with the size
    for (unsigned i = 0; i < size; ++i) 
    {
        AdjacentList.push_back({});
    }
}

/**
 * @brief Destructor for the ALGraph class.
 */
ALGraph::~ALGraph()
{

}

/**
 * @brief Adds a directed edge between two nodes in the graph.
 * 
 * Inserts the edge in the adjacency list of the source node in a sorted order based on weight,
 * and then by destination node ID as a tiebreaker.
 * 
 * @param source The source node of the edge.
 * @param destination The destination node of the edge.
 * @param weight The weight of the edge.
 */
void ALGraph::AddDEdge(unsigned source, unsigned destination, unsigned weight) 
{
    
    unsigned id = --source ;
    AdjacencyInfo info{destination, weight};
    auto& adj_list = AdjacentList.at(id);
    auto it = adj_list.begin(); // Use 'it' for clarity and consistency
    bool inserted = false;

    while (it != adj_list.end()) 
    {
        if (info.weight < it->weight || (info.weight == it->weight && info.id < it->id)) 
        {
            adj_list.insert(it, info);
            inserted = true;
            break; // Exit the loop since insertion is done
        }
        ++it; // Move to the next item in the list
    }

    if (!inserted) 
    {
        adj_list.push_back(info); // Insert at the end if no suitable position was found
    }
}

/**
 * @brief Adds an undirected edge between two nodes in the graph.
 * 
 * This is achieved by adding two directed edges, one in each direction.
 * 
 * @param node1 One of the nodes to connect.
 * @param node2 The other node to connect.
 * @param weight The weight of the edge.
 */
void ALGraph::AddUEdge(unsigned node1, unsigned node2, unsigned weight) 
{
    //Add the edge to both nodes
    AddDEdge(node1, node2, weight);
    AddDEdge(node2, node1, weight);
}


/**
 * @brief Computes the shortest paths from a start node to all other nodes in the graph using Dijkstra's algorithm.
 * 
 * @param start_node The starting node for the paths.
 * @return std::vector<DijkstraInfo> A vector containing the shortest path information to each node.
 */
std::vector<DijkstraInfo> ALGraph::Dijkstra(unsigned start_node) const 
{
    std::vector<DijkstraInfo> info(AdjacentList.size());  //Distance
    std::vector<GNode> graphNodes(AdjacentList.size());   //Nodes

    //Initialize the result for all of the nodes
    for (size_t i = 0; i < AdjacentList.size(); ++i)
    {
        info[i].cost = INFINITY_ALGRAPH;
        graphNodes[i].nodeID = static_cast<unsigned int>(i + 1);  // Initialize node IDs
    }

    //Start node is 0
    info[start_node - 1].cost = 0;
    //It's path is itself
    info[start_node - 1].path.push_back(start_node);

    for (size_t i = 0; i < AdjacentList.size(); ++i)
    {
        // Find the node with the minimum cost, from the set of nodes that have not been visited
        int min_cost_node = -1;
        for (size_t j = 0; j < AdjacentList.size(); ++j)
        {
            if (!graphNodes[j].isExplored && (min_cost_node == -1 || info[j].cost < info[min_cost_node].cost))
            {
                min_cost_node = static_cast<int>(j);
            }
        }

        // If no unvisited node is found, break the loop
        if (min_cost_node == -1)
        {
            break;
        }

        // Mark the node as visited
        graphNodes[min_cost_node].isExplored = true;

        // Update the cost of each neighbor of the current node
        for (const auto& neighbor : AdjacentList[min_cost_node])
        {
            // If the cost of the current node plus the weight of the edge to the neighbor is less than the current cost of the neighbor
            unsigned alt_cost = info[min_cost_node].cost + neighbor.weight;
            // Update the cost of the neighbor and its path
            if (alt_cost < info[neighbor.id - 1].cost)
            {
                info[neighbor.id - 1].cost = alt_cost;
                info[neighbor.id - 1].path = info[min_cost_node].path;
                info[neighbor.id - 1].path.push_back(neighbor.id);
            }
        }
    }
    
    return info;

}

/**
 * @brief Retrieves the adjacency list of the graph.
 * 
 * @return ALIST The adjacency list of the graph.
 */
ALIST ALGraph::GetAList() const 
{
    return AdjacentList;
}

/**
 * @brief Operator overloading for comparing two GNode objects based on their cost.
 * 
 * @param rhs The right-hand side GNode object to compare with.
 * @return true If the cost of the left-hand side object is less than the right-hand side object.
 * @return false Otherwise.
 */
bool ALGraph::GNode::operator<(const GNode &rhs) const
{
  return info.cost < rhs.info.cost;
}