#include <map>
#include <vector>
#include <iostream>
#include <math.h>
#include <algorithm>
#include <set>
#include <queue>
#include <numeric>
#include <stdio.h>
#include <tuple>
#include <bitset>
#include "tri_table.h"

using namespace std;


Vertex verts[8];    //vertices
Edge edges[12];     //full edges

Vertex* HalfEdge::from() {
    return twin->to;
}

void setVerts(int n) {
    int index = 0;
    for(int i = 0; i < 8; i++)
        verts[i].in = 0;
    while (n != 0) {
        int rem = n % 2;
        verts[index].in = rem;
        n /= 2;
        index++;
    }
}

void setEdgesNotDone() {
    for(int e = 0; e < 12; e++) {
        edges[e].done = false;
    }
}

float dist(Vertex v1, Vertex v2) {
    float d1 = pow(abs(v1.x - v2.x), 2);
    float d2 = pow(abs(v1.y - v2.y), 2);
    float d3 = pow(abs(v1.z - v2.z), 2);
    return sqrt(d1 + d2 + d3);
}

tuple<Vertex, float> getTriArea(Edge e1, Edge e2, Edge e3) {
    vector<Vertex> v;
    for(Edge e : {e1, e2, e3}) {
        float x_avg = (e.he->to->x + e.he->from()->x)/2;
        float y_avg = (e.he->to->y + e.he->from()->y)/2;
        float z_avg = (e.he->to->z + e.he->from()->z)/2;
        v.push_back(Vertex(x_avg, y_avg, z_avg));
    }

    bool valid = !((v[0].x == v[1].x && v[1].x == v[2].x) ||
                (v[0].y == v[1].y && v[1].y == v[2].y) ||
                (v[0].z == v[1].z && v[1].z == v[2].z));

    Vertex v1(v[1].x - v[0].x, v[1].y - v[0].y, v[1].z - v[0].z);
    Vertex v2(v[2].x - v[0].x, v[2].y - v[0].y, v[2].z - v[0].z);
    
    Vertex cross = Vertex::cross(v1, v2);
    float mag = Vertex::magnitude(cross);
    Vertex cross_norm(cross.x/mag, cross.y/mag, cross.z/mag);

    float a = dist(v[0], v[1]);
    float b = dist(v[1], v[2]);
    float c = dist(v[2], v[0]);

    float s = (a + b + c) / 2;
    return {cross_norm, sqrt(s*(s - a)*(s - b)*(s - c))};
}

bool checkTransverse(Edge e) {
    //half edge to vertex in != half edge twin vertex in
    return e.he->to->in != e.he->twin->to->in;
}

void triangulatePolyR(vector<int> rem_v, queue<vector<int>> rem_vl,
            vector<vector<int>> parent_tris, vector<vector<vector<int>>> &tri_table) {
    int v1 = rem_v[0];
    int v2 = rem_v[1];

    for(int i = 2; i < rem_v.size(); i++) {
        int v3 = rem_v[i];
        //add new triangle
        vector<vector<int>> nparent_tris(parent_tris);
        nparent_tris.push_back({v1, v2, v3});

        //sub rem verts list
        queue<vector<int>> s_rem_vl(rem_vl);  
        //sub rem verts1
        vector<int> s_rem_v1(rem_v.begin()+1, rem_v.begin()+i+1);
        if(s_rem_v1.size() > 2)
            s_rem_vl.push(s_rem_v1);
        //sub rem verts2
        vector<int> s_rem_v2(rem_v.begin() + i, rem_v.end());
        s_rem_v2.insert(s_rem_v2.begin(), rem_v[0]);
        if(s_rem_v2.size() > 2)
            s_rem_vl.push(s_rem_v2);

        //get next thing in queue
        if(s_rem_vl.size() > 0) {
            vector<int> nrem_v = s_rem_vl.front();
            s_rem_vl.pop();
            triangulatePolyR(nrem_v, s_rem_vl, nparent_tris, tri_table);
        } else {
            tri_table.push_back(nparent_tris);
        }
    }
}

vector<int> triangulatePoly(vector<int> p_edges) {
    vector<int> rem_v(p_edges);
    vector<vector<int>> parent_tris;
    queue<vector<int>> rem_v_list;
    vector<vector<vector<int>>> tri_table;
    triangulatePolyR(rem_v, rem_v_list, parent_tris, tri_table);

    float min_area = 0;
    vector<int> min_option;
    int bestComplexity = INT_MAX;
    for(int i = 0; i < tri_table.size(); i++) {
        vector<vector<int>> option = tri_table[i];

        vector<int> option_tris;
        float total_area = 0;
        vector<Vertex> crosses;
        int complexity = 0;
        for(vector<int> tri : option) {
            auto [cross, area] = getTriArea(edges[tri[0]], edges[tri[1]], edges[tri[2]]);
            bool same = 0;
            for(Vertex v : crosses) {
                if(cross == v) {
                    cout << "YES ";
                    same = 1;
                }
            }
            //If the plane of the triangle is the same as an existing one, it does not add complexity
            if(!same) {
                crosses.push_back(cross);
                complexity++;
            }

            total_area += area;
            option_tris.insert(option_tris.end(), tri.begin(), tri.end());
            for(int val : tri) {
                cout << val << ", ";
            }
        }
        cout << endl;
        cout << "Complexity: " << complexity << endl;
        if(complexity < bestComplexity) {
            min_option = option_tris;
            bestComplexity = complexity;
        }
    }
    cout << endl;
    return min_option;
}

HalfEdge* posHalfEdge(Edge e) {
    if(e.he->to->in == 1)
        return e.he;
    else
        return e.he->twin;
}

HalfEdge* nextCWHalfEdge(HalfEdge* he) {
    do {
        he = he->next_cw;
    } while (!checkTransverse(*(he->parent)));
    return he;
}

void generateEdgeTable() {

    vector<vector<int>> tri_table(pow(2, 8));
    vector<int> edge_table;

    int max_size = 0;
    for (int i = 0; i < pow(2, 8); i++) {
        setVerts(i);
        setEdgesNotDone();
        int e_table_val = 0;
        vector<int> p_edges;
        for(int e = 0; e < 12; e++) {
            //go thorugh all edges if that edge is an edge that goes from inside shape to outside shape
            if(!edges[e].done && checkTransverse(edges[e])) {
                HalfEdge* current_he = posHalfEdge(edges[e]);
                //while that edge has not been contained (looped back to)
                //essentially finds all transverse edges that are connected to that edge
                while(!edges[e].done) {
                    //
                    current_he = nextCWHalfEdge(current_he);
                    e_table_val += (1 << current_he->parent->id);
                    p_edges.push_back(current_he->parent->id);
                    current_he->parent->done = true;
                    current_he = current_he->twin;  //switching half edge switches the face
                }
            }
            vector<int> new_poly;
            if(p_edges.size() >= 3) {
                cout << i << ": " << endl;
                new_poly = triangulatePoly(p_edges);
                if(new_poly.size() > max_size) 
                    max_size = new_poly.size();
            }
            tri_table[i].insert(tri_table[i].end(), new_poly.begin(), new_poly.end());
            p_edges.clear();
        }
        edge_table.push_back(e_table_val);
    }
    cout << max_size << endl;
    for(int r = 0; r < tri_table.size(); r++) {
        //bitset<8> index_bin(r);
        //cout << index_bin << ": ";
        vector<int> setup = tri_table[r];
        vector<int> output(setup);
        output.resize(max_size);
        fill(output.begin()+setup.size(), output.end(), -1);
        cout << "{";
        for(int c = 0; c < output.size(); c++) {
            cout << output[c];
            if(c < (int)output.size()-1)
                cout << ", ";
        }
        cout << "}";
        if(r < (int)tri_table.size()-1)
            cout << ",";
        cout << endl;
    }
}

int main(int argc, char *argv[]) {

    //set up vertices
    int x = 0;
    int y = 0;
    int z = 0;
    bool dir = 0;   //x or y
    int sign = 1;   //+ or -
    int dist = 2;   //length of each jump
    for(int i = 0; i < 8; i++) {
        verts[i] = Vertex(0, i, x, y, z);
        //jump in x or y
        if(dir)
            x += sign*dist;
        else
            y += sign*dist;
        //jump up in z dir after 4 verts
        if((i+1)%4 == 0) 
            z += dist;
        //switch dir
        dir = !dir;
        //switch sign after jump in x and y
        if((i+1)%2 == 0)
            sign *= -1;
    }
    
    //set up bottom edges
    HalfEdge h_edges_b[4];
    for(int i = 0; i < 4; i++) {
        int next = i+1;
        if(i == 3)  //loop
            next = 0;
        h_edges_b[next].to = &verts[next];
        h_edges_b[next].next_cw = &h_edges_b[i];
        h_edges_b[next].parent = &edges[next];
        //set edge half edge
        edges[next].he = &h_edges_b[next];   
    }

    //set up top edges
    HalfEdge h_edges_t[4];
    for(int i = 4; i < 8; i++) {
        int next = i+1;
        if(i == 7)  //loop
            next = 4;
        h_edges_t[i-4].to = &verts[next];
        h_edges_t[i-4].next_cw = &h_edges_t[next-4];
        h_edges_t[i-4].parent = &edges[i];
        //set edge half edge
        edges[i].he = &h_edges_t[i-4];   
    }

    //vertical faces
    HalfEdge faces[4][4];
    for(int f = 0; f < 4; f++) {
        HalfEdge* face = faces[f];
        //set up edge convetion for faces
        int te = f+4;
        int re = f+8;
        int be = f;
        int le = f+9;
        int e[4] = {te, re, be, le};
        //set up vertex convention for faces
        int v0 = f+4;
        int v1 = f;
        int v2 = f+1;
        int v3 = f+5;
        int v[4] = {v0, v1, v2, v3};

        if(f == 3) {
            v[2] = 0;   //vert loop
            v[3] = 4;   //vert loop
            e[3] = 8;   //edge loop
        }

        for(int i = 0; i < 4; i++ ) {
            int next = i+1;
            //next face edge loop     
            if(i == 3)
                next = 0;
            //set face edge properties
            face[i].to = &verts[v[i]]; 
            face[i].next_cw = &face[next];
            face[i].parent = &edges[e[i]];
            //set edge properties
            edges[e[i]].id = e[i];
            if(i == 3) {    //only do once per face
                //set half edges
                edges[e[i]].he = &face[i];
            } else if (e[i] != 8) {
                //set twins
                face[i].twin = edges[e[i]].he;
                edges[e[i]].he->twin = &face[i]; 
            }
        } 
    }
    //fix last edge
    faces[0][1].twin = edges[8].he;
    edges[8].he->twin = &faces[0][1]; 

    generateEdgeTable();

    //https://gist.github.com/porumbes/cb8e075e70d4e7b4a6df
}