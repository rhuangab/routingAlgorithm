#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stack>
#include <queue>
#include <vector>
#include <cmath>
#include <unordered_map>
using namespace std;
//A node class
class Node;
class Edge{
public:
	int cost;
	Node* src;
	Node* dest;
	Edge(Node* src, Node* dest, int cost = 1){
		this->cost = cost;
		this->src = src;
		this->dest = dest;
	}
};

class Node{
public:

	int dist; //use for dijkstra's algor
	int val;
	bool visited;
	Node* previous; //use for dijkstra's algor
	vector<Edge*> neighbors;
	Node(int value):val(value), visited(false), dist(0x7FFFFFFF),
		previous(NULL){}
	void addEdge(Edge* e){
		if(e->src != this) return;
		for(Edge* e2 : neighbors){
			if(e == e2 || e2->dest == e->dest) return;
		}
		neighbors.push_back(e);
	}
};

class pqNode{
public:
	int dist;
	bool visited;
	Node* node;
	Node* pre;
	Node* next; //next hop from src to dest
	pqNode(){}
	pqNode(int d, Node* n, Node* pre=NULL)
		:dist(d),node(n),pre(NULL),visited(false),next(NULL){}
};

struct cmp{
   	bool operator()(pqNode& n1, pqNode& n2){
   		return n1.dist > n2.dist;
   	}
};

class Graph{
public:
	vector<Node*> vertex;
	bool directed;
	Graph(bool directed = false){
		this->directed = directed;
	}
	
	void addVertex(int val){
		Node* node = new Node(val);
		vertex.push_back(node);
		return;
	}
	
	Node* getVertex(int vid){
		return vertex[vid-1];
	}
	/*
	void addVertex(Node* node){
		//if(vertex.find(node) != vertex.end()) return;
		//vertex[node] = vector<Edge*>();
	}
	*/
	void reinitialize(){
		for(Node* node: vertex){
			node->visited = false;
			node->previous = NULL;
			node->dist = 0x7FFFFFFF;
		}
		return;
	}
	
	
	size_t getSize() const{
		return vertex.size();
	}

	bool hasEdge(int src_id, int dest_id){
		if(src_id >= vertex.size()) return false;
		bool exist = false;
		for(auto edge: vertex[src_id-1]->neighbors){
			if(edge->dest->val == dest_id){
				exist = true;
				break;
			}
		}
		return exist;
	}
	
	void addEdge(int src_id, int dest_id, int cost = 1){
		if(!hasEdge(src_id, dest_id))
			addEdge(vertex[src_id-1], vertex[dest_id-1], cost);
		else{
			updateEdge(src_id, dest_id, cost);
		}
	}
	
	void addEdge(Node* src, Node* dest, int cost = 1){
		if(src == dest) return;
		if(!directed){
			Edge* edge1 = new Edge(src, dest, cost);
			Edge* edge2 = new Edge(dest, src, cost);
			src->addEdge(edge1);
			dest->addEdge(edge2);
		}
		else{
			Edge* edge = new Edge(src, dest, cost);
			src->addEdge(edge);
		}
		return;
	}

	void updateEdge(int src_id, int dest_id, int cost = 1){
		if(src_id == dest_id) return;
		for(auto edge: vertex[src_id-1]->neighbors){
			if(edge->dest->val == dest_id)
			{
				edge->cost = cost;
				break;
			}
		}
		for(auto edge: vertex[dest_id-1]->neighbors){
			if(edge->dest->val == src_id)
			{
				edge->cost = cost;
				break;
			}
		}
	}
	
	void printGraph(ostream& out = std::cout){
		out<<"--------Graph structure---------\n";
		for(Node* node : vertex){
			out<<"Node "<<node->val<<":";
			for(Edge* e : node->neighbors){
				out<<e->dest->val<<"("<<e->cost<<")";
			}
			out<<std::endl;
		}
		out<<"--------------End---------------\n";
	}


	/**
   * Dijkstra Shortest path
   **/
   void Dijkstra(vector<pqNode>& distList, int src_id){
   	Node* src = getVertex(src_id);
   	priority_queue<pqNode, vector<pqNode>, cmp> pq;
   	for(Node* node : vertex){
   		if(src != node){
   			pqNode pn = pqNode(0x7FFFFFFF, node);
   			distList[pn.node->val] = pn;
   		}
   		else{
   			pqNode pn = pqNode(0, node);
   			pq.push(pn);
   			distList[pn.node->val] = pn;
   		}
   	}
   	while(!pq.empty()){
   		pqNode curPqNode = pq.top();
   		pq.pop();
   		if(curPqNode.node->visited) continue;
   		curPqNode.node->visited = true;
   		for(Edge* e : curPqNode.node->neighbors){
   			Node* curNode = curPqNode.node;
   			Node* dest = e->dest;
   			if(e->cost >= 0 && distList[dest->val].dist > distList[curNode->val].dist + e->cost){
   				distList[dest->val].dist = distList[curNode->val].dist + e->cost;
   				distList[dest->val].pre = curNode;
   				pq.push(distList[dest->val]);
   			}
   		}
   	}
   }
};

   void findNextHop(vector<pqNode>& distList, Node* src){
   		for(int i = 1; i < distList.size(); i++){
   			pqNode* cur = &distList[i];
   			if(cur->pre == NULL){
   				cur->next = cur->node;
   			}
   			else{
	   			while(cur->pre != src){
	   				cur = &distList[cur->pre->val];
	   			}
	   			distList[i].next = cur->node;
   			}	
   		}
   }

   void sendMsg(vector<vector<pqNode> >& forwardTable, int from, int to, char* msg, ostream& out=std::cout){
	   	out<<"from "<<from<<" to "<<to<<" hops ";
	   	int cur = from;
	   	while(cur != to){
	   		out << cur << " ";
	   		int nextHop = forwardTable[cur][to].next->val;
	   		cur = nextHop;
	   	}
	   	out<<"message "<<msg<<std::endl<<std::endl;
   }

   void printAllForwardTable(vector<vector<pqNode> >& forwardTable, ostream& out=std::cout){
	   	for(int i = 1; i < forwardTable.size(); i++){
	 		auto distList = forwardTable[i];
	 		//printf("Shorest path from Node %d: \n", i);
	 		for(pqNode& pn : distList){
	   			if(&pn - &distList[0] == 0) continue;
	   			//printf("To Node %d cost: %d, next: %d\n", pn.node->val, pn.dist, pn.next?pn.next->val:0);
	   			out << pn.node->val << " "<< pn.next->val << " " << pn.dist<<endl;
	   			//printf("To Node %d cost: %d, next: %d\n", 1, pn.dist, pn.next?pn.next->val:0);
	   		}
	   		out <<endl;
	 	}
   }


int main(int argc, char *argv[])
{
	if(argc != 4){
		printf("Usage: ./linkstate topofile messagefile changesfile.\n");
		exit(0);
	}

	struct edge{
		int src;
		int dest;
		int cost;
		edge(int s, int d, int c):src(s), dest(d), cost(c){}
	};

    // create the graph given in above fugure
    Graph* graph = new Graph();
    vector<edge> tempEdges;

    ifstream instr;
    instr.open(argv[1]);
    int src, dest, cost;
    int graphSize = 0;
    while(instr.good()){
    	instr >> src >> dest >> cost;
    	graphSize = max(graphSize, max(src, dest));
    	tempEdges.push_back(edge(src, dest, cost));
    }
    instr.close();
    for(int i = 1; i <= graphSize; i++){
    	graph->addVertex(i);
    }
    for(edge& e : tempEdges){
    	graph->addEdge(e.src, e.dest, e.cost);
    }

    // print the adjacency list representation of the above graph
    graph->printGraph();
    
    vector<vector<pqNode> > forwardTable(graph->getSize()+1, vector<pqNode>(graph->getSize()+1));

    //calculate forwarding table and converge.
    for(auto node : graph->vertex){
 		graph->reinitialize();
 		graph->Dijkstra(forwardTable[node->val], node->val);
 	}

 	for(int i = 1; i < forwardTable.size(); i++){
 		findNextHop(forwardTable[i], graph->getVertex(i));
 	}

 	ofstream of;
 	of.open("output.txt");

 	printAllForwardTable(forwardTable, of);

 	/** Start exchange messages **/
 	ifstream imsgstr;
 	imsgstr.open(argv[2]);
 	while(imsgstr.good()){
 		int from, to;
 		char msg[256];
 		imsgstr >> from >> to >> msg[0]; //the last one is to get rid of heading spaces.
 		imsgstr.getline(msg+1, 256);
 		sendMsg(forwardTable, from, to, msg, of);
 	}
 	imsgstr.close();

 	ifstream ichange;
 	ichange.open(argv[3]);
 	while(ichange.good()){
 		int src, dest, cost;
 		ichange >> src >> dest >> cost;
 		graph->addEdge(src, dest, cost);

 		for(auto node : graph->vertex){
	 		graph->reinitialize();
	 		//printf("Shortest Path from Node %d", node->val);
	 		graph->Dijkstra(forwardTable[node->val], node->val);
	 	}

 		for(int i = 1; i < forwardTable.size(); i++){
 			findNextHop(forwardTable[i], graph->getVertex(i));
 		}

 		printAllForwardTable(forwardTable, of);

 		/** Start exchange messages **/
	 	ifstream imsgstr;
	 	imsgstr.open(argv[2]);
	 	while(imsgstr.good()){
	 		int from, to;
	 		char msg[256];
	 		imsgstr >> from >> to >> msg[0]; //the last one is to get rid of heading spaces.
	 		imsgstr.getline(msg+1, 256);
	 		sendMsg(forwardTable, from, to, msg, of);
	 	}
	 	imsgstr.close();
 		
 	}
 	ichange.close();
 	cout <<"end"<<endl;

 	of.close();
    return 0;
}