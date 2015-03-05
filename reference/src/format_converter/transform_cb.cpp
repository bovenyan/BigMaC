/**
 * Author: Nanxi Kang (nkang@cs.princeton.edu)
 * All rights reserved.
 */

#include<iostream>
#include<fstream>
#include<cstdio>
#include<algorithm>

using namespace std;

const int IP_LEN = 32;
const int MAX_N = 1<<20;

long mi[IP_LEN + 10];
long pr[MAX_N][10];
int len;
char src[1000], dst[1000];

long getnumber(const char* s) {
  long value = 0, tmp = 0;
  int i;

  for (i = 0; s[i] && s[i] != '/'; ++i) {
    if (s[i] && s[i] == '.') {
      value = (value << 8) + tmp;
      tmp = 0;
    }
    else
      tmp = tmp * 10 + s[i] - '0';
  }
  value = (value << 8) + tmp;

  long mask = 0;
  for (++i; s[i]; ++i) 
    mask = mask * 10 + s[i] - '0';
  long l = mi[mask] - 1;
  return l + (value >> (32 - mask));
}

long getFather(long x) {
  return (x % 2 ? x / 2 : x / 2 - 1);
}

int getDepth(long x) {
  int ret;
  for (ret = 0 ; x; ++ret)
    x = getFather(x);
  return ret;
}

void output(const char* s) {
 long value = 0, tmp = 0;
  int i;

  for (i = 0; s[i] && s[i] != '/'; ++i) {
    if (s[i] && s[i] == '.') {
      cout << tmp << endl;
      value = (value << 8) + tmp;
      cout << value << endl;
      tmp = 0;
    }
    else
      tmp = tmp * 10 + s[i] - '0';
  }
  cout << tmp << endl;
  cout << value << endl;
  value = (value << 8) + tmp;

  int mask = 0;
  for (++i; s[i]; ++i) 
    mask = mask * 10 + s[i] - '0';
  cout << mask << endl;
  long l = (1<<mask) - 1;
  cout << mask << " " <<l + (value >> (32 - mask)) << endl;
}

int count (int start, int end) {
  int l, now = start, next;

  int c = 0;
  while (now <= end) {
    ++c;
    int k = -1;
    for (int i = 0; ; ++i) {
      l = (1 << i) -1;
      next = (now | l);
      if (next <= end && next - l == now) {
	k = next;
      }
      else {
	now = k + 1;
	break;
      }
    }
  }
  return c;
}

void init() {
  // compute powers-of-two array
  mi[0] = 1;
  for (int i = 1; i <= IP_LEN; ++i)
    mi[i] = mi[i - 1] * 2;

  //randomize
  srand(time(NULL));
}

int main(int argc, char* argv[]) {
  init();

  ifstream cb_file;
  ofstream res_file;
  int A = 2;
  cb_file.open(argv[1]);
  res_file.open(argv[2]);
  if (argc > 3)
    sscanf(argv[3], "%d", &A);

  if (!cb_file.is_open() || !res_file.is_open()) {
    cout << "cannot open file" << endl;
    return 1;
  }

  // read
  char ch[100];
  while (cb_file >> src) {
    if (src[0] == '@') {
      cb_file >> dst;
      long ks = getnumber(src+1);
      long kd = getnumber(dst);
      int sp_s, sp_e, dp_s, dp_e;
      
      cb_file >> sp_s >> ch >> sp_e;
      cb_file >> dp_s >> ch >> dp_e;
      pr[len][0] = ks;
      pr[len][1] = kd;
      pr[len][2] = count(sp_s, sp_e) * count(dp_s, dp_e);
      pr[len][3] = sp_s;
      pr[len][4] = sp_e;
      pr[len][5] = dp_s;
      pr[len][6] = dp_e;
      // generate actions randomly
      pr[len][7] = rand () % A;
      ++len;
    }
  }
  
  res_file << len << endl;
  for (int i = 0; i < len; ++i) {
    res_file << pr[i][0] << " " << pr[i][1] << " " 
	     << pr[i][2] << " " << pr[i][3] << " " 
	     << pr[i][4] << " " << pr[i][5] << " " 
	     << pr[i][6] << " " << pr[i][7] << endl;
  }

  cb_file.close();
  res_file.close();
}
