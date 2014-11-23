/**
 *-------------------------------------------
 Graph structure.
 	Graph:
 		src_id -> Vertice:
 				destination_vertice_id -> Edges:
 					src_id
 					dest_id
 					distance
	
		Example: Graph with vertext: 1,2,3,4

		1 ------------------ 2
		|			2 		 |
		|					 |
		|1					 |3 
		|					 |
		3 -------------------4
					2


		The FINAL routing table IN vertice 1 would be:
			From 	1	2 	3	4
			To 1:		0 	2 	1 	3
			To 2:		2 	0 	3 	3 (this row comes neighbour 2's advertisment)
			To 3:		1 	3 	0 	2 (this row comes neighbour 3's advertisment)
		The structure of routingTable would be:
			routingTableRow: routingTableNode, ..., routingTableNode
			...
			routingTableRow: routingTableNode, ..., routingTableNode
 *-------------------------------------------
 */

#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <stack>
#include <algorithm>
#include <cstring>

/**
 * Class: Edge
 * Description: A representation of a single edge. 
 *	The edge will be store in the source node.
 * Member:
 * 	src_id: 	the id of source node, not the name. Name is store in the graph.
 * 	dest_id: 	the id of destination node, not the name.
 *	dist:		the cost/distance of this edge.
 */
class Edge{
public:
	int src_id;
	int dest_id;
	int dist;
	Edge(){src_id = 0; dest_id = 0; dist = 0x7FFFFFFF;}
	Edge(int sid, int did, int distance = 0x7FFFFFFF):src_id(sid), dest_id(did),dist(distance){}
	Edge(const Edge& e){
		src_id = e.src_id;
		dest_id = e.dest_id;
		dist = e.dist;
	}
};

/**
 * Class RoutingTableNode
 * Description: The element in the routing table row
 */
typedef class RoutingTableNode{
public:
	int dist;
	int next_hop_id;
	RoutingTableNode(){dist = 0x7FFFFFFF; next_hop_id = 0;}
	RoutingTableNode(int d, int n)
		:dist(d),next_hop_id(n){}
}rnode;

/**
 * Class RoutingTableRow
 * Description: 
 *	The row of a vertice,
 * 	containing the dist from a vertice to all other connected vertex.
 *	A routingTableRow would contain multiple routingTableNodes.
 **/
typedef class RoutingTableRow{
public:
	int src_id;
	//Mapping: dest_id -> routingTableNode.
	std::unordered_map<int, rnode> dest; 
	RoutingTableRow(){src_id = 0;}
	RoutingTableRow(int sid)
		:src_id(sid){}
}rrow;

/**
 *	Class: vertice.
 *	Description:
 *	Every vertice contains:
 *		id: the id in the graph
 *		val: the node text.
 *		edges: dest_id -> Edge{id, dest_id, dist}
 *		routingTable: (neighbour and itself) id -> routing table node
 *			routing table node:
 *				From src_id: 
 *				To : [dest_id1 -> dist1], 
 *					 [dest_id2 -> dist2]....
**/
class Vertice{
public:
	int id;
	int val; //Name or Value of the node.
	//Mapping: dest_id -> Edge.
	std::unordered_map<int, Edge> edges;
	//Mapping: id -> routingTableRow
	std::unordered_map<int, rrow> routingTable;

	Vertice(){id = 0; val = 0;}
	Vertice(int tid, int tval):id(tid), val(tval){
		//create a routing table row for itself.
		routingTable[id] = RoutingTableRow(id);
		//distance to itself is zero
		routingTable[id].dest[id] = rnode(0, id);
	}

	Vertice(const Vertice& v){
		id = v.id;
		val = v.val;
		routingTable[id] = RoutingTableRow(id);
		//distance to itself is zero
		routingTable[id].dest[id] = rnode(0, id);
	}

	/** Get the map of all edges **/
	std::unordered_map<int, Edge>& getEdges(){
		return edges;
	}

	/** Add a edge to this vertice. **/
	void addEdge(int dest_id, int dist = 0x7FFFFFFF){
		if(edges.find(dest_id) != edges.end())return;
		edges[dest_id] = Edge(id, dest_id,dist);
		//Also add the rnode in the rrow of this node. For initializing.
		routingTable[id].dest[dest_id] = rnode(dist,dest_id);
	}

	/** Update the cost of an edge. 
	 * 	If dist < 0. delete this edge. 
	 *	Right now negative cost would consider as deleting an edge.
	 **/
	void updateEdge(int dest_id, int dist){
		//If the edge does not exist, ignore the call, return.
		if(edges.find(dest_id) != edges.end()){
			if(dist >= 0){
				edges[dest_id].dist = dist;
				routingTable[id].dest[dest_id].dist = dist;
			}
			else{
				removeEdge(dest_id);
				routingTable[id].dest.erase(dest_id);
				//because the edge is not exist, delete all rnode that next_hop_id == dest_id
				std::stack<int> toRemoved; 
				for(auto& pair: routingTable[id].dest){
					if(pair.second.next_hop_id == dest_id){
						toRemoved.push(pair.first);
					}
				}
				while(!toRemoved.empty()){
					routingTable[id].dest.erase(toRemoved.top());
					toRemoved.pop();
				}
			}
		}
	}

	/** Remove an edge. **/
	void removeEdge(int dest_id){
		if(edges.find(dest_id) != edges.end()){
			edges.erase(dest_id);
			removeRrow(dest_id);
		}
	}

	void removeRrow(int dest_id){
		if(routingTable.find(dest_id) != routingTable.end()){
			routingTable.erase(dest_id);
		}
	}

	/** This function will check whether the edge cost is changed since last iteration.
		If an edge which an shortest path go through is removed or changed, it
		will make the shortest path to a node to 0x7FFFFFFF. Let the distance
		vector algoithm rerun to converge the routing table.
	**/
	bool updateRrow(rrow& r2){
		routingTable[r2.src_id] = r2;
		//update my own rrow
		bool changed = false;
		for(auto& pair: r2.dest){
			/*
			 *	pair.first: 	dest_id
			 *	pair.second: 	routingTableNode.
			 */
			//If there is a new rnode in r2, add it.
			if(routingTable[id].dest.find(pair.first)==routingTable[id].dest.end())
			{
				routingTable[id].dest[pair.first]= rnode(edges[r2.src_id].dist
					+ pair.second.dist,r2.src_id);
				changed = true;
			}
			/*
			 * Update the cost if this node is where the shortest path that
			 * current running vertice is going through.
			 * The cost should be updated no matter larger or smaller than the 
			 *	last cost. Because the network condition will be changed.
			 */
			else if(routingTable[id].dest[pair.first].next_hop_id == r2.src_id){
				if(routingTable[id].dest[pair.first].dist != 
						pair.second.dist + edges[r2.src_id].dist)
				{
					changed = true;
					routingTable[id].dest[pair.first].dist = pair.second.dist + edges[r2.src_id].dist;
				}
			}
			/* If r2.src_id go to pair.first(which is the dest_id)
			 * through this vertice(the current running vertice), 
			 * this vertice should ignore the cost, otherwise this might 
			 * cause the problem called 'couting to infinity'.
			 * For more info, refer to Rever poisoning:
			 * 	http://en.wikipedia.org/wiki/Route_poisoning
			 */
			else if(pair.second.next_hop_id == id){
				continue;
			}
			/*Update if there is a shorter path from other neighbour.*/
			else if(edges[r2.src_id].dist + pair.second.dist < routingTable[id].dest[pair.first].dist){
				routingTable[id].dest[pair.first].dist=edges[r2.src_id].dist
						+ pair.second.dist;
				routingTable[id].dest[pair.first].next_hop_id = r2.src_id;
				changed = true;
			}
		}
		return changed;
	}
};

class Graph{
private:
	int next_id;
	std::unordered_map<int, Vertice> vertex; //id -> vertice
	std::map<int, int> valToId; //val -> id
	std::priority_queue<int> collectedBackId; //collect id back if vertice is deleted.

public:
	Graph(){
		next_id = 1;
	}

	void addVertice(int val){
		if(valToId.find(val) != valToId.end()) return;
		int nid;
		if(!collectedBackId.empty()){
			nid = collectedBackId.top();
			collectedBackId.pop();
		}
		else nid = next_id++;
		valToId[val] = nid;
		vertex[nid] = Vertice(nid, val);
	}
	/*
	void removeVertice(int id){
		if(vertex.find(id) != vertex.end()){
			int val = vertex[id].val;
			for(auto& pair : vertex[id].getEdges()){
				int neighbour_id = pair.first;
				vertex[neighbour_id].removeEdge(id);
			}
			for(auto& vpair : vertex){
				if(vpair.first != id){
					vpair.second.removeRrow(id);
				}
			}
			vertex.erase(id);
			valToId.erase(val);
		}
	}*/

	size_t size(){
		return vertex.size();
	}

	void addOrUpdateEdge(int src, int dest, int dist = 1){
		if(valToId.find(src)==valToId.end()) addVertice(src);
		if(valToId.find(dest)==valToId.end()) addVertice(dest);
		int src_id = valToId[src];
		int dest_id = valToId[dest];
		if(vertex[src_id].edges.find(dest_id) == vertex[src_id].edges.end()){
			if(dist < 0) return;
			vertex[src_id].addEdge(dest_id, dist);
		}
		else{
			vertex[src_id].updateEdge(dest_id, dist);
		}
	}

	void runDistVectAlgor(){
		//Keep running until no changes happen.
		while(true){
			bool changed = false;
			for(auto& pair : vertex){
				/*id : pair.first
				 *Vertice : pair.second
				 *Get the rrow from neighbour's routing table.
				 */
				for(auto& edgePair: pair.second.edges){
					int dest_id = edgePair.first;
					changed = pair.second.updateRrow(vertex[dest_id].routingTable[dest_id])
						|| changed;
				}
			}
			if(!changed) break;
		}
	}

	void printGraph(std::ostream& out){
		out<<"-------Graph-----------"<<std::endl;
		for(auto& vpair : vertex){
			out << vpair.second.val << ": ";
			for(auto& epair : vpair.second.edges){
				out<<vertex[epair.second.dest_id].val
					<<"("<<epair.second.dist<<")";
			}
			out<<std::endl;
		}
		out<<"-------Graph End--------\n\n";
	}

	/* Print the routing table of a single vertex. */
	void printARoutingTable(int id, std::ostream& out){
		out << "Vertice " << vertex[id].val<<"'s table: ";
		out << "<dest> <next> <distance>." << std::endl;
		for(auto vpair : valToId){
			int dest_id = vpair.second;
			rnode* r = NULL;
			if(vertex[id].routingTable[id].dest.find(dest_id)
				!= vertex[id].routingTable[id].dest.end())
				r = &vertex[id].routingTable[id].dest[dest_id];
			if(r)
				out<<vpair.first<<" "
					<<vertex[r->next_hop_id].val<<" "
					<<r->dist
					<<std::endl;
			else
				out<<vpair.first<<" "<<"None"<<"None"<<std::endl;
		}
		out<<std::endl;
	}

	/* Print all vertex's routing table.*/
	void printAllRoutingTables(std::ostream& out){
		for(auto& pair: valToId){
			printARoutingTable(pair.second, out);
		}
	}

	/* Try to send message: from -> to, tracking the intermediate route.
	 * Input Format: <source> <destination> <message>
	 * Output Format: From <source> to <destination> hops <hop> ... <hop> <message content>
	 * For example: if the shortest path from 1 to 3 is 1-4-2-3.
	 * Send a message "Hello, Node 4"
	 * Output would be: From 1 to 3 hops 1 4 2 Hello, Node 4
	*/
	void sendMsg(int from, int to, char* msg, std::ostream& out){
		int src_id = valToId[from];
		int dest_id = valToId[to];
		out<<"from "<<from<<" to "<<to<<" hops ";
		int cur = src_id;
		while(cur != dest_id){
			out<<vertex[cur].val<<" ";
			cur = vertex[cur].routingTable[cur].dest[dest_id].next_hop_id;
		}
		out<<"message "<<msg<<std::endl<<std::endl;
	}
};

using namespace std;
/* Read message from istream, could be std::cin or ifstream 
 * Input Format: <source> <destination> <message>
 * Output Format: From <source> to <destination> hops <hop> ... <hop> <message content>
 */
void readMessage(ostream& out, istream& in, Graph& graph){
 	while(in.good()){
 		int from, to;
 		char msg[256];
 		in >>from;
 		if(from == -1) break;
 		in >>to >>msg[0]; //the last one is to get rid of heading spaces.
 		in.getline(msg+1, 256);
 		graph.sendMsg(from, to, msg, out);
 	}
}

/* Using std io. */
void run(ostream& out, istream& in){
	Graph graph;
	/* Read topology from file */
    out << "Please input graph topology, <vertice 1> <vectice 2> <dist> : \n";
    out <<"Enter -1 to finish this section.\n";
    int src, dest, cost;
    while(true){
    	in >> src;
    	if(src == -1) break;
    	in >> dest >> cost;
    	graph.addVertice(src);
    	graph.addVertice(dest);
    	graph.addOrUpdateEdge(src, dest, cost);
    	graph.addOrUpdateEdge(dest, src, cost);
    }
    graph.printGraph(out);
    graph.runDistVectAlgor();
    graph.printAllRoutingTables(out);

    /** Start exchange messages **/
 	out <<"Please input message to sent, input Format: <source> <destination> <message>.\n";
 	out <<"Enter -1 to finish this section.\n";
 	readMessage(out, in, graph);

 	/** Topology will change now **/
 	while(true){
 		out <<"Please input the changes happen to the graph.\n";
	 	out <<"With format: <vertice 1> <vectice 2> <dist>\n";
	 	out <<"Negative distance means deleting the existing edge\n";
	 	out <<"Enter -1 to finish this section.\n";
 		int src, dest, cost;
 		in >> src;
 		if(src == -1) break;
 		in >> dest >> cost;
 		graph.addOrUpdateEdge(src, dest, cost);
 		graph.addOrUpdateEdge(dest, src, cost);
 		graph.printGraph(out);
 		graph.runDistVectAlgor();
    	graph.printAllRoutingTables(out);

    	/** Start exchange messages **/
	 	out <<"Please input message to sent, input Format: <source> <destination> <message>.\n";
	 	out << "Enter a single -1 when you are finished with message sending: \n";
	 	readMessage(out, in, graph);
 	}
}
/* Using file io. */
void run(ostream& out, char* argv[])
{
	Graph graph;
	/* Read topology from file */
	std::ifstream instr;
    instr.open(argv[1]);
    int src, dest, cost;
    while(instr.good()){
    	instr >> src >> dest >> cost;
    	graph.addVertice(src);
    	graph.addVertice(dest);
    	graph.addOrUpdateEdge(src, dest, cost);
    	graph.addOrUpdateEdge(dest, src, cost);
    }
    instr.close();
    graph.printGraph(out);
    graph.runDistVectAlgor();
    graph.printAllRoutingTables(out);

    /** Start exchange messages **/
 	ifstream imsgstr;
 	imsgstr.open(argv[2]);
 	readMessage(out, imsgstr, graph);
 	imsgstr.close();

 	/** Topology will change now **/
 	ifstream ichange;
 	ichange.open(argv[3]);
 	while(ichange.good()){
 		int src, dest, cost;
 		ichange >> src >> dest >> cost;
 		graph.addOrUpdateEdge(src, dest, cost);
 		graph.addOrUpdateEdge(dest, src, cost);
 		graph.printGraph(out);
 		graph.runDistVectAlgor();
    	graph.printAllRoutingTables(out);

    	/** Start exchange messages **/
    	ifstream imsgstr;
	 	imsgstr.open(argv[2]);
	 	readMessage(out, imsgstr, graph);
	 	imsgstr.close();
 	}
 	ichange.close();
 	std::cout<<"Output to distvec_output.txt.\n";
}

int main(int argc, char* argv[]){
	if(!(argc == 2 && strcmp(argv[1],"stdio") == 0) && argc != 4){
		printf("Usage: ./distvec stdio or ./distvec <topofile> <messagefile> <changesfile>.\n");
		exit(0);
	}
	
	if(argc == 4){
		ofstream out;
		out.open("distvec_output.txt");
		ifstream in;
		run(out, argv);
		out.close();
	}
	else{
		run(std::cout, std::cin);
	}

    return 0;
}


