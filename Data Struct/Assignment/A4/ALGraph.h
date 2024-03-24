/*!
* @file			ALGraph.hpp
* @author		Maojie Deng, 2200840, maojie.deng@digipen.edu
* @date			29/2/2024
* @par			Assignment 4 : ALGraph using Dijkstra's Algorithm
* @brief		This file contains the functions for  finding shortest ALGraph using Dijkstra's Algorithm.
*/


//---------------------------------------------------------------------------
#ifndef ALGRAPH_H
#define ALGRAPH_H
//---------------------------------------------------------------------------
#include <vector>


struct DijkstraInfo
{
  unsigned cost;
  std::vector<unsigned> path;
};

struct AdjacencyInfo
{
  unsigned id;
  unsigned weight;
};

typedef std::vector<std::vector<AdjacencyInfo> > ALIST;
class ALGraph
{
  public:
    ALGraph(unsigned size);
    ~ALGraph(void);
    void AddDEdge(unsigned source, unsigned destination, unsigned weight);
    void AddUEdge(unsigned node1, unsigned node2, unsigned weight);

    std::vector<DijkstraInfo> Dijkstra(unsigned start_node) const;
    ALIST GetAList(void) const;
        
    

  private:

    // An EXAMPLE of some other classes you may want to create and 
    // implement in ALGraph.cpp
    class GNode
    {
    public:
        unsigned nodeID{0};
        DijkstraInfo info;
        std::vector<const AdjacencyInfo*> AdjacentList;
        GNode* previousNode = 0;
        bool isExplored = false;
        bool operator<(const GNode& rhs) const;

    };

    class GEdge;
    struct AdjInfo
    {
      unsigned weight;
      unsigned cost;
    };
    
    // Other private fields and methods
    ALIST AdjacentList;
    static const unsigned INFINITY_ALGRAPH = static_cast<unsigned>(-1);
};

#endif
