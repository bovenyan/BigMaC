#ifndef TRACEGEN_H
#define TRACEGEN_H

#include "../stdafx.h"
#include "RuleList.hpp"
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>
#include <cassert>
#include <thread>
#include <future>
#include <mutex>
#include <atomic>
#include <chrono>
#include <set>
#include <map>

#include <pcap.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

using std::vector;
using std::string;
using std::atomic_uint;
using std::atomic_bool;
using std::mutex;
using boost::unordered_map;
using boost::unordered_set;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

/*
 * Usage:
 *   tracer tGen(rulelist pointer);
 *   tGen.setpara(parameter file);
 *   pFlow_pruning_gen (objective synthetic trace directory)
 */

class tracer
{
    boost::log::sources::logger tracer_log;
public:
    double flow_rate; // rate of flow 
    string trace_root_dir; 	// the root directory to save all traces

private:
    rule_list * rList;
    uint32_t flow_no;
    double simuT;
    EpochT jesusBorn;
    atomic_uint total_packet;

    // sources and gen
    string pcap_dir;		// original pcap trace direcotry
    string parsed_pcap_dir;	// directory of parsed pcap file
    string flowInfoFile_str;    // first arr time of each flow
    string headerFile_str;  // header generating from rules
   
    // intermediate
    vector<string> to_proc_files;
    string gen_trace_dir;	// the directory for generating one specific trace

public:
    tracer();
    tracer(rule_list *);

    // parameter settings 
    void set_para(string para_file); // setting parameter 
    void get_proc_files(); 
    void print_setup() const; // print the setup 

    // tool kits
    void trace_get_ts(string trace_ts_file); // Get the initial time for each parsed trace
    friend uint32_t count_proc(); // count the no. of processors to determine threads
    void merge_files(string) const; // merge_file with for format "/ptrace-" to "gen_trace_dir"
    // void raw_snapshot(string, double, double);
    // void pcap_snapshot(size_t, double, pref_addr = pref_addr(), pref_addr = pref_addr());

    /* trace generation and evaluation
     * 1. pFlow_pruning_gen(string trace_root_dir): generate traces to the root directory with "Trace_Generate" sub-dir
     * 2. flow_pruning_gen(string trace_dir): generate a specific trace with specific parameter
     * 3. f_pg_st(...): a single thread for mapping and generate traces
     * 4. flow_arr_mp(): obtain the start time of each flow for later use.
     * 5. f_arr_st(...): a single thread for counting the no. of packets for each flow
     * 6. parse_pack_file_mp(string): process the file from pcap directory and process them into 5tup file
     * 7. p_pf_st(vector<string>): obtain the pcap file in vector<string> and do it.
     * 8. packet_count_mp(...): count the packet of each flow...  // deprecated
     * 9. p_count_st(...): single thread method for packet_count... // deprecated
     */
    void pFlow_pruning_gen(); // wrapping function for flow generation
    void flow_pruneGen_mp(unordered_set<addr_5tup> & flowInfo, const vector<addr_5tup> & headers) const;
    void f_pg_st (string, uint32_t, boost::unordered_map<addr_5tup, std::pair<uint32_t, addr_5tup> > *) const;
    
    boost::unordered_set<addr_5tup> flow_arr_mp() const; // getting arrival time for each flow
    boost::unordered_set<addr_5tup> f_arr_st (string) const;
    
    void parse_pcap_file_mp(size_t, size_t) const;
    void p_pf_st(vector<string>, size_t) const;
    
    //void packet_count_mp(string, string);
    //void p_count_st(fs::path, atomic_uint*, mutex *, boost::unordered_map<addr_5tup, uint32_t>*, atomic_bool *);
};


// pcap related
#define ETHER_ADDR_LEN	6
#define ETHER_TYPE_IP		(0x0800)
#define ETHER_TYPE_8021Q 	(0x8100)

/* Ethernet header */
struct sniff_ethernet {
    u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
    u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
    u_short ether_type; /* IP? ARP? RARP? etc */
};


/* IP header */
struct sniff_ip {
    u_char ip_vhl;		/* version << 4 | header length >> 2 */
    u_char ip_tos;		/* type of service */
    u_short ip_len;		/* total length */
    u_short ip_id;		/* identification */
    u_short ip_off;		/* fragment offset field */
#define IP_RF 0x8000		/* reserved fragment flag */
#define IP_DF 0x4000		/* dont fragment flag */
#define IP_MF 0x2000		/* more fragments flag */
#define IP_OFFMASK 0x1fff	/* mask for fragmenting bits */
    u_char ip_ttl;		/* time to live */
    u_char ip_p;		/* protocol */
    u_short ip_sum;		/* checksum */
    struct in_addr ip_src,ip_dst; /* source and dest address */
};
#define IP_HL(ip)		(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)		(((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
    u_short th_sport;	/* source port */
    u_short th_dport;	/* destination port */
    tcp_seq th_seq;		/* sequence number */
    tcp_seq th_ack;		/* acknowledgement number */
    u_char th_offx2;	/* data offset, rsvd */
#define TH_OFF(th)	(((th)->th_offx2 & 0xf0) >> 4)
    u_char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    u_short th_win;		/* window */
    u_short th_sum;		/* checksum */
    u_short th_urp;		/* urgent pointer */
};

#endif
