/*  
  This class provides some tools statically accessible from every class;
*/

#ifndef __MY_TOOLBOX_H__   
#define __MY_TOOLBOX_H__  

// #include <vector> 
#include <map> 

using namespace std;

class MyToolbox {
 private:
  static int n_;  // number of storage nodes in the network
  static int k_;  // number of sensors in the network
  static int C1_; // parameter to set the max number of hops of a source block
  static int max_msg_hops_; // maximum number of steps a message can do from the source, in the RW

  /*  This timetable contains pairs of the type:
        - key = node_id
        - value = time at which the node is going to be "let free"
  */
  static map<int, int> timetable_; 
 public:
  static void set_n(int);
  static void set_k(int);
  static void set_C1(int);
  static void set_timetable(map<int, int>);
  static int get_n() {return n_;}
  static int get_k() {return k_;}
  static int get_max_msg_hops() {return max_msg_hops_;}
  static map<int, int> get_timetable() {return timetable_;}
  static int get_ideal_soliton_distribution_degree(); 
};

#endif