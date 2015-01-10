#include <math.h>
#include <iostream>
#include <stdlib.h>     /* srand, rand */

#include "my_toolbox.h"

using namespace std;


int MyToolbox::n_ = 0;
int MyToolbox::k_ = 0;
int MyToolbox::C1_ = 0;
int MyToolbox::max_msg_hops_ = 0;

void MyToolbox::set_n(int n) {
  n_ = n;
}

void MyToolbox::set_k(int k) {
  k_ = k;
}

void MyToolbox::set_C1(int C1) {
  C1_ = C1;
  max_msg_hops_ = ceil(C1 * n_ * log(n_));
}

int MyToolbox::get_ideal_soliton_distribution_degree() {

  /*
    What is the smallest interval of probability [1/(i^2 - i) - 1/(i^2 + i)] ?
    I must have a granularity smallest than this interval, therwise I would be
    sure I won't ever have some values!

    The smallest interval is the last one, always.

    Devo dividere l'intervallo in modo che l'unità (1/M) sia piu' corta dell'intervallo
    piu' corto.

    Divido l'intervallo [0, 1] in M intervalli.
    La lunghezza di un intervallo (1/M) deve essere piu' grande dell'intervallo di prob minore,
    che e' 1/(k^2 - k).

    1/M < int_min = 1 / (k^2 - k) => M > k(k - 1)

    Per andare sul sicuro: M = 10 * k(k - 1)

    Ho M intervalli. Genero un numero random tra 0 e M - 1 con "rand() % M" e 
    poi divido ancora per (M - 1) per normalizzare tra 0 e 1.

    The intervl bounds are so divided:
    [       ]      ]    ]  ] ]
  */

  int M = k_ * (k_ - 1);
  double prob = (rand() % M) / (double)(M - 1);

  // cout << "Prob: " << prob << endl;

  double up_bound = 1. / k_;
  double interval_length = up_bound;
  if (prob <= up_bound) { // between 0 and 1/k -> degree = 1;
    return 1;
  }
  for (int i = 2; i <= k_; ++i) {
    interval_length = 1. / (i * i - i);
    up_bound += interval_length;
    // cout << "Step: " << i << ". Interval length: " << interval_length << ", up_bound: " << up_bound << endl;
    if (prob <= up_bound) {
      return i;
    }
  }

  return k_;
}