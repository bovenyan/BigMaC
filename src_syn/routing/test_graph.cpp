#include "../stdafx.h"
#include "GraphGen.h"

using std::cout; using std::endl;

void print_usage(){
        cout << "missing input: -I -g <graph_file> -e <edge_no>"<<endl;
        cout << "               -I -p <path_file> "<<endl;
        cout << "               -F -k <k_value>"<<endl;
}

int main(int argc, char * argv[]){
    if (argc < 3){
        print_usage();
    }

    if (!strcmp(argv[1], "-I")){
        // Internet
        int edge_no = 50;
        const char * file_name;
        const char * path_file_name;
        
        bool read_graph = false;
        bool read_path = false;

        for (int i = 2; i < argc-1; ++i){
            if (!strcmp(argv[i], "-e"))
                edge_no = atoi(argv[i+1]);
            if (!strcmp(argv[i], "-g")){
                file_name = argv[i+1];
                read_graph = true;
            }
            if (!strcmp(argv[i], "-p")){
                path_file_name = argv[i+1];
                read_path = true;
            }
        }

        InternetGraph iGraph;

        // calculate shortest path
        if (read_graph){
            iGraph.ReadGraph(file_name, edge_no);
            iGraph.CalShortestPath("./path_rec.txt");

            // retrieve path
            for (int i = 0; i < iGraph.edge_ID.size(); ++i){
                for (int j = 0; j < iGraph.edge_ID.size(); ++j)
                    cout<<iGraph.edge_ID[i]<<"-"<<iGraph.edge_ID[j]<<" : "<< iGraph.print_path(i,j)<<endl;
            }
        }

        // readpath
        if (read_path){
            iGraph.ReadPath(path_file_name);

            // retrieve path
            for (int i = 0; i < iGraph.edge_ID.size(); ++i){
                for (int j = 0; j < iGraph.edge_ID.size(); ++j)
                    cout<<iGraph.edge_ID[i]<<"-"<<iGraph.edge_ID[j]<<" : "<< iGraph.print_path(i,j)<<endl;
            }
        }
    }

    if (!strcmp(argv[1], "-F")){
        // FatTree
    }
}
