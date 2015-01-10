/*  
  This class provides some tools statically accessible from each class;
*/

#ifndef __MY_TOOLBOX_H__   
#define __MY_TOOLBOX_H__   

// using namespace std;

class MyToolbox {
 private:
  static int n_;  // number of storage nodes in the network
  static int k_;  // number of sensors in the network
  static int C1_; // parameter to set the max number of hops of a source block
  static int max_msg_hops_; // maximum number of steps a message can do from the source, in the RW
 public:
  static void set_n(int);
  static void set_k(int);
  static void set_C1(int);
  static int get_n() {return n_;}
  static int get_k() {return k_;}
  static int get_max_msg_hops() {return max_msg_hops_;}
  static int get_ideal_soliton_distribution_degree();
};

#endif