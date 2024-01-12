#include <iostream>
#include <math.h>

class Vertex {
    public:
        bool in;
        int id;
        float x, y, z;

        Vertex() {}
        Vertex(bool in, int id, float x, float y, float z) {
            this->in = in;
            this->id = id;
            this->x = x;
            this->y = y;
            this->z = z;
        }
        Vertex(float x, float y, float z) {
            this->in = 0;
            this->id = 0;
            this->x = x;
            this->y = y;
            this->z = z;
        }
        static Vertex cross(Vertex v1, Vertex v2) {
            float cross_x = v1.y * v2.z - v1.z * v2.y;
            float cross_y = -1*(v1.x * v2.z - v1.z * v2.x);
            float cross_z = v1.x * v2.y - v1.y * v2.x;
            
            return Vertex(cross_x, cross_y, cross_z);
        }
        static float magnitude(Vertex v) {
            return sqrt(pow(v.x, 2) + pow(v.y, 2) + pow(v.z, 2));
        }
        friend bool operator == (Vertex const& v1, Vertex const& v2) noexcept {   
            return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
        }
        friend std::ostream& operator<<(std::ostream &strm, const Vertex &v) {
            return strm << v.id << ": (" << v.x << ", " << v.y << ", " << v.z << ")";
        }

};


class Edge;

class HalfEdge {
    public:
        Vertex* to;
        HalfEdge* next_cw;
        HalfEdge* twin;
        Edge* parent;
        HalfEdge() {}
        Vertex* from();
};

class HalfEdge;

class Edge {
    public:
        int id;
        HalfEdge* he;
        bool done;
        Edge() {
            this->he = nullptr;
            this->done = false;
        }
};