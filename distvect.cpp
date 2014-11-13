#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <stack>
#include <algorithm>

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

typedef class RoutingTableNode{
public:
	int dist;
	int next_hop_id;
	RoutingTableNode(){dist = 0x7FFFFFFF; next_hop_id = 0;}
	RoutingTableNode(int d, int n)
		:dist(d),next_hop_id(n){}
}rnode;

typedef class RoutingTableRow{
public:
	int src_id;
	//dest_id -> distance
	std::unordered_map<int, rnode> dest; 
	//int dest_id;
	//int dist;
	RoutingTableRow(){src_id = 0;}
	RoutingTableRow(int sid)
		:src_id(sid){}
}rrow;

/**
	Every vertice contains:
		id: the id in the graph
		val: the node text.
		edges: dest_id -> Edge{id, dest_id, dist}
		routingTable: (neighbour and itself) id -> routing table node
			routing table node:
				From src_id: 
				To : [dest_id1 -> dist1], 
					 [dest_id2 -> dist2]....
**/
class Vertice{
public:
	int id;
	int val;
	std::unordered_map<int, Edge> edges;
	std::unordered_map<int, rrow> routingTable;

	Vertice(){id = 0; val = 0;}
	Vertice(int tid, int tval):id(tid), val(tval){
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

	std::unordered_map<int, Edge>& getEdges(){
		return edges;
	}

	void addEdge(int dest_id, int dist = 0x7FFFFFFF){
		if(edges.find(dest_id) != edges.end())return;
		edges[dest_id] = Edge(id, dest_id,dist);
		routingTable[id].dest[dest_id] = rnode(dist,dest_id);
		//printf("Edge size:%d, src:%d, dest: %d\n", edges.size(),id,dest_id);
	}

	void updateEdge(int dest_id, int dist){
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

	void removeEdge(int dest_id){
		if(edges.find(dest_id) != edges.end()){
			edges.erase(dest_id);
			removeRrow(dest_id);
		}
	}
	/*
	void initRoutingTable(){
		return;
		routingTable[id] = RoutingTableRow(id);
		//distance to itself is zero
		routingTable[id].dest[id] = rnode(0, id);
		for(auto& pair : edges){
			int dest_id = pair.first;
			int dist = pair.second.dist;
			routingTable[id].dest[dest_id] = rnode(dist,dest_id);
		}
	}*/
	/*
	void addRnode(int dest_id, int dist = 0x7FFFFFFF){
		if(routingTable.find(dest_id) != routingTable.end()) return;
		if(dest_id == id) dist = 0;
		routingTable[dest_id] = RoutingTableRow(id);
	}
	*/
	/*
	bool updateRrow(int dest_id, int dist){
		if(routingTable.find(dest_id) != routingTable.end()){
			routingTable[dest_id].dist = dist;
			return true;
		}
		else return false;
	}*/


	/** This function will check whether the edge cost since last iteration.
		If an edge which an shortest path go through is removed or changed, it
		will make the shortest path to a node to 0x7FFFFFFF. Let the distance
		vector algoithm rerun to converge the routing table.
	**/
	bool updateRrow(rrow& r2){
		routingTable[r2.src_id] = r2;
		//update my own rrow
		bool changed = false;
		for(auto& pair: r2.dest){
			if(routingTable[id].dest.find(pair.first)==routingTable[id].dest.end())
			{
				routingTable[id].dest[pair.first]= rnode(edges[r2.src_id].dist
					+ pair.second.dist,r2.src_id);
				changed = true;
			}
			else if(routingTable[id].dest[pair.first].next_hop_id == r2.src_id){
				if(routingTable[id].dest[pair.first].dist != 
						pair.second.dist + edges[r2.src_id].dist)
				{
					changed = true;
					routingTable[id].dest[pair.first].dist = pair.second.dist + edges[r2.src_id].dist;
				}
			}
			else if(pair.second.next_hop_id == id){
				continue;
			}
			else if(edges[r2.src_id].dist + pair.second.dist < routingTable[id].dest[pair.first].dist){
				routingTable[id].dest[pair.first].dist=edges[r2.src_id].dist
						+ pair.second.dist;
				routingTable[id].dest[pair.first].next_hop_id = r2.src_id;
				changed = true;
			}
		}
		return changed;
	}

	void removeRrow(int dest_id){
		if(routingTable.find(dest_id) != routingTable.end()){
			routingTable.erase(dest_id);
		}
	}

};

class Graph{
private:
	int next_id;
	std::unordered_map<int, Vertice> vertex; //id -> vertice
	std::map<int, int> valToId; //val -> id
	std::priority_queue<int> collectedBackId;

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
	/*
	void initAllRoutingTables(){
		for(auto& pair : vertex){
			pair.second.initRoutingTable();
		}
	}*/

	void runDistVectAlgor(){
		while(true){
			bool changed = false;
			for(auto& pair : vertex){
				//id : pair.first
				//Vertice : pair.second
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
		out<<"-------Graph End--------"<<std::endl;
	}

	void printARoutingTable(int id, std::ostream& out){
		//auto rptr = &vertex[id].routingTable;
		//cout<<vertex[id].val
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

	void printAllRoutingTables(std::ostream& out){
		for(auto& pair: valToId){
			printARoutingTable(pair.second, out);
		}
	}

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
int main(int argc, char* argv[]){
	if(argc != 4){
		printf("Usage: ./distvec topofile messagefile changesfile.\n");
		exit(0);
	}
	Graph graph;
	ofstream out;
	out.open("output2.txt");

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
    graph.printGraph(std::cout);
    //graph.initAllRoutingTables();
    graph.runDistVectAlgor();
    graph.printAllRoutingTables(out);

    /** Start exchange messages **/
 	ifstream imsgstr;
 	imsgstr.open(argv[2]);
 	while(imsgstr.good()){
 		int from, to;
 		char msg[256];
 		imsgstr >>from >>to >>msg[0]; //the last one is to get rid of heading spaces.
 		imsgstr.getline(msg+1, 256);
 		graph.sendMsg(from, to, msg, out);
 	}
 	imsgstr.close();

 	/** Topology will change now **/
 	ifstream ichange;
 	ichange.open(argv[3]);
 	while(ichange.good()){
 		int src, dest, cost;
 		ichange >> src >> dest >> cost;
 		graph.addOrUpdateEdge(src, dest, cost);
 		graph.addOrUpdateEdge(dest, src, cost);
 		graph.printGraph(std::cout);
 		graph.runDistVectAlgor();
    	graph.printAllRoutingTables(out);
 	}
 	ichange.close();

 	out.close();
    return 0;
}


