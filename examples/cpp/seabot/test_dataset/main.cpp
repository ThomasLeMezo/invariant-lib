#include "dataSet/datasetnode.h"

#include <iostream>
#include "vibes/vibes.h"
#include <cstring>
#include <omp.h>

#include <ibex.h>

using namespace std;
using namespace invariant;

int main(int argc, char *argv[]){

    std::vector<std::array<int, 2>> position;
    position.push_back(std::array<int, 2>({0, 10}));
    position.push_back(std::array<int, 2>({0, 10}));
    std::vector<std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > > leaf_list;
    DataSetNode<double, 2> test(position, leaf_list);


//    for(size_t i = 0; i<leaf_list.size(); i++){
//        for(array<int, 2>&a:leaf_list[i].second)
//            cout << "(" << a[0] << ", " << a[1] << ") ";
//        cout << endl;
//    }

    for(std::pair<DataSetVirtualNode*, std::vector<std::array<int, 2> > > &leaf:leaf_list){
        vector<array<int, 2>> position = leaf.second;
        array<array<double, 2>, 2> data;
        data[0][0] = position[0][0]; data[0][1] = position[0][0];
        data[1][0] = position[1][0]; data[1][1] = position[1][0];
        leaf.first->set_node_val(data, true);
    }

    test.fill_tree();

    std::vector<std::array<int, 2>> target;
    target.push_back(std::array<int, 2>({10, 0}));
    target.push_back(std::array<int, 2>({10, 0}));

    array<array<double, 2>, 2> data;
    data[0][0] = 3.0; data[0][1] = 4.0;
    data[1][0] = 1.0; data[1][1] = 2.0;

//    array<array<double, 2>, 2> data;
//    data[0][0] = 10.0; data[0][1] = 11.0;
//    data[1][0] = 1.0; data[1][1] = 2.0;

    test.eval_invert(target, position, data);

    cout << test.get_number_node() << endl;
    cout << test.get_number_leaf() << endl;

    cout << target[0][0] << " " << target[0][1] << endl;
    cout << target[1][0] << " " << target[1][1] << endl;

}
