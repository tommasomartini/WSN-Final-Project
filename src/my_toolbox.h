/*  
  This class provides some tools statically accessible from every class;
*/

#ifndef __MY_TOOLBOX_H__   
#define __MY_TOOLBOX_H__  

// #include <vector> 
#include <map> 

using namespace std;

class MyToolbox {
 public:
  typedef unsigned long MyTime;

  static const double LIGHT_SPEED = 299792458; // meter / seconds

  // getters
  static MyTime get_current_time() {return current_time_;}
  static int get_n() {return n_;}
  static int get_k() {return k_;}
  static int get_bits_for_measure() {return bits_for_measure_;}
  static int get_bits_for_id() {return bits_for_id_;}
  static int get_bits_for_phy_mac_overhead() {return bits_for_phy_mac_overhead_;}
  static int get_max_msg_hops() {return max_msg_hops_;}
  static double get_channel_bit_rate_() {return channel_bit_rate_;}
  static map<int, MyTime> get_timetable() {return timetable_;}
  static int get_ping_frequency() {return ping_frequency_;}
  static int get_check_sensors_frequency_() {return check_sensors_frequency_;}

  // setters
  static void set_current_time(MyTime);
  static void set_n(int);
  static void set_k(int);
  static void set_C1(int);
  static void set_bits_for_measure(int);
  static void set_bits_for_id(int);
  static void set_bits_for_phy_mac_overhead(int);
  static void set_channel_bit_rate(double);
  static void set_timetable(map<int, MyTime>);
  static void set_ping_frequency(int);
  static void set_check_sensors_frequency(int);

  // functions
  static int get_ideal_soliton_distribution_degree();
  static int get_robust_soliton_distribution_degree();  // still to implement!
  static MyTime get_random_processing_time();
  // Return a uniform random offset a node must wait when it finds the channel busy.
  // The value is uniformly distributed between 1ns and MAX_OFFSET ns
  static MyTime get_retransmission_offset();

 private:
  static const int MEAN_PROCESSING_TIME = 100000; // 100us, in nano-seconds
  static const double STD_DEV_PROCESSING_TIME = 1000000; // 1ms, in nano-seconds

  // TODO: remove this from here! Should be more accessible from a user
  static const MyTime MAX_OFFSET = 1000000; // 1ms, in nano-seconds

  static MyTime current_time_; // to keep track of the time

  static int n_;  // number of storage nodes in the network
  static int k_;  // number of sensors in the network
  static int C1_; // parameter to set the max number of hops of a source block
  static int bits_for_measure_; // number of bits to represent a measure
  static int bits_for_id_; // number of bits to represent a node identifier
  static int bits_for_phy_mac_overhead_; // number of bits for PHY and MAC overhead
  static int max_msg_hops_; // maximum number of steps a message can do from the source, in the RW
  static double channel_bit_rate_;  // bit rate of the wireless channel, in bit/seconds
  static int ping_frequency_; // frequency at which sensors do "ping"
  static int check_sensors_frequency_; // frequency at which node chek if its supervisioned sensors are alive

  /*  This timetable contains pairs of the type:
        - key = node_id
        - value = time at which the node is going to be "left free"
  */
  static map<int, MyTime> timetable_; 
};

#endif