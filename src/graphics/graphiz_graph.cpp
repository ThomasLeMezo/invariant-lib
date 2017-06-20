#include "graphiz_graph.h"
#include <gvc.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <graph.h>
#include <map>


using namespace std;
using namespace invariant;

Graphiz_Graph::Graphiz_Graph(const string &file_name, Graph *g)
{
    double d_1 = 30;
    double d_2 = 20;
    static GVC_t *gvc;
    if (!gvc)
        gvc = gvContext();

    Agraph_t *graphiz_graph; // Graph
    graphiz_graph = agopen((char*)"g", Agdirected, 0);

    std::map<Pave*,Agnode_t *> map_node;
    std::map<Face*,Agnode_t *> map_face;

    // Nodes
    for(Pave* p:g->get_paves()){
        if(!p->is_infinite()){
            Agnode_t *n;
            std::stringstream node_param;
            node_param << p->get_position() << endl;
            node_param << p << endl;
            n = agnode(graphiz_graph, (char*)(node_param.str().c_str()), true);

            std::stringstream node_pos;
            node_pos << p->get_position().mid()[0]*d_1 << ","
                                                       << -(p->get_position().mid()[1]*d_1) << "!";
            char* def = new char;
            agsafeset(n, (char*)"pos", (char*)(node_pos.str().c_str()), def);

            map_node.insert(std::pair<Pave*,Agnode_t *>(p, n));
        }
    }

    for(Pave* p:g->get_paves_not_bisectable()){
        Agnode_t *n;
        std::stringstream node_param;
        node_param << p->get_position() << endl;
        node_param << p << endl;
        n = agnode(graphiz_graph, (char*)(node_param.str().c_str()), true);
        map_node.insert(std::pair<Pave*,Agnode_t *>(p, n));
    }


    for(Pave* p:g->get_paves()){
        for(Face *f:p->get_faces_vector()){
            Agnode_t *n;
            std::stringstream face_param;
            face_param << f->get_position() << endl;
            face_param << f << endl;
            n = agnode(graphiz_graph, (char*)(face_param.str().c_str()), true);
            agsafeset(n, (char*)"shape", (char*)"rectangle", (char*)"");

            std::stringstream node_pos;
            node_pos << p->get_position().mid()[0]*(d_1-d_2) + f->get_position().mid()[0]*d_2
                    << ","
                    << -(p->get_position().mid()[1]*(d_1-d_2) + f->get_position().mid()[1]*d_2)
                    << "!";
            char* def = new char;
            agsafeset(n, (char*)"pos", (char*)(node_pos.str().c_str()), def);
            agedge(graphiz_graph, map_node[p], n, (char*)"t", 1);

            map_face.insert(std::pair<Face*,Agnode_t *>(f, n));
        }
    }

    //    for(Pave* p:g->get_paves_not_bisectable()){
    //        for(Face *f:p->get_faces_vector()){
    //            Agnode_t *n;
    //            std::stringstream face_param;
    //            face_param << f->get_position() << endl;
    //            face_param << f << endl;
    //            n = agnode(graphiz_graph, (char*)(face_param.str().c_str()), true);
    //            agsafeset(n, (char*)"shape", (char*)"rectangle", (char*)"");
    //            agsafeset(n, (char*)"constraint", (char*)"false", (char*)"");

    //            Agedge_t *e;
    //            e = agedge(graphiz_graph, map_node[p], n, (char*)"t", 1);

    //            map_face.insert(std::pair<Face*,Agnode_t *>(f, n));
    //        }
    //    }

    for(Pave* p:g->get_paves()){
        for(Face *f:p->get_faces_vector()){
            for(Face *f_n:f->get_neighbors()){
                Agedge_t *e;
                e = agedge(graphiz_graph, map_face[f], map_face[f_n], (char*)"t", 1);
                agsafeset(e, (char*)"color", (char*)"blue", (char*)"");
                agsafeset(e, (char*)"dir", (char*)"forward", (char*)"");
                agsafeset(e, (char*)"constraint", (char*)"false", (char*)"");
            }
        }
    }

    //    for(Pave* p:g->get_paves_not_bisectable()){
    //        for(Face *f:p->get_faces_vector()){
    //            for(Face *f_n:f->get_neighbors()){
    //                Agedge_t *e;
    //                e = agedge(graphiz_graph, map_face[f], map_face[f_n], (char*)"", 1);
    //                agsafeset(e, (char*)"color", (char*)"red", (char*)"");
    //                agsafeset(e, (char*)"dir", (char*)"none", (char*)"");
    //                agsafeset(e, (char*)"constraint", (char*)"false", (char*)"");
    //            }
    //        }
    //    }


    /* Set an attribute - in this case one that affects the visible rendering */
    //    agsafeset(n, (char*)"shape", (char*)"record", (char*)"");

    gvLayout(gvc, graphiz_graph, "neato");
    FILE * pFile;
    pFile = fopen (file_name.c_str(),"w");
    gvRender(gvc, graphiz_graph, "pdf", pFile);
    fclose(pFile);
    gvFreeLayout(gvc, graphiz_graph);
    agclose(graphiz_graph);
}
