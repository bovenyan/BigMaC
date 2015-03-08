#include "TraceGen.h"

using std::string;
using std::vector;
using std::pair;
using std::list;
using std::set;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::thread;
using std::atomic_uint;
using std::atomic_bool;
using std::mutex;
using boost::unordered_map;
using boost::unordered_set;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

typedef boost::unordered_map<addr_5tup, uint32_t>::iterator Map_iter;
typedef vector<fs::path> Path_Vec_T;

// constructors
tracer::tracer():total_packet(0) {
    rList = NULL;
    flow_no = 0;
    jesusBorn = EpochT(-1,0);
}

tracer::tracer(rule_list * rL):total_packet(0) {
    rList = rL;
    flow_no = 0;
    jesusBorn = EpochT(-1,0);
}


// source and gen
void tracer::set_para(string loc_para_str) {
    ifstream ff(loc_para_str);
    for (string str; getline(ff, str); ) {
        vector<string> temp;
        boost::split(temp, str, boost::is_any_of("\t"));
        if (!temp[0].compare("flow arrival rate")) {
            flow_rate = boost::lexical_cast<double>(temp[1]);
        }
        if (!temp[0].compare("simulation time")) {
            simuT = boost::lexical_cast<double>(temp[1]);
        }
        if (!temp[0].compare("root dir")) {
	        trace_root_dir = temp[1];
	    }
	    if (!temp[0].compare("flow arrival instance file")){
	        flowInfoFile_str = temp[1];
	    }
        if (!temp[0].compare("candidate header file")){
            headerFile_str = temp[1];
        }
	    if (!temp[0].compare("pcap dir")){
	        pcap_dir = temp[1];
	    }
	    if (!temp[0].compare("parsed pcap dir")){
	        parsed_pcap_dir = temp[1];
	    }
    }
}

void tracer::print_setup() const {
    cout <<" ======= SETUP BRIEF: ======="<<endl;
    cout<<"flow arrival rate set as\t"<<flow_rate <<endl;
    cout<<"header info is in : " << flowInfoFile_str << endl;
    cout<<"reference trace directory is in\t"<<parsed_pcap_dir<<endl;
    cout <<" ======= SETUP DONE: ======="<<endl;
}

void tracer::trace_get_ts(string trace_ts_file) {
    fs::path dir(parsed_pcap_dir);
    ofstream ffo(trace_ts_file);

    if (fs::exists(dir) && fs::is_directory(dir)) {
    	Path_Vec_T pvec;
    	std::copy(fs::directory_iterator(dir), fs::directory_iterator(), std::back_inserter(pvec));
    	std::sort(pvec.begin(), pvec.end());
    
    	for (Path_Vec_T::const_iterator it (pvec.begin()); it != pvec.end(); ++it){
            try {
                io::filtering_istream in;
                in.push(io::gzip_decompressor());
                ifstream infile(it->c_str());
                in.push(infile);
                string str;
                getline(in, str);
                addr_5tup f_packet(str);
                ffo<< *it << "\t" << f_packet.timestamp <<endl;
                io::close(in); // careful
            } catch (const io::gzip_error & e) {
                cout<<e.what()<<std::endl;
            }
        }
    }
    return;
}

void tracer::get_proc_files () {
    // find out how many files to process
    fs::path dir(parsed_pcap_dir);

    if (fs::exists(dir) && fs::is_directory(dir)) {
    	Path_Vec_T pvec;
    	std::copy(fs::directory_iterator(dir), fs::directory_iterator(), std::back_inserter(pvec));
    	std::sort(pvec.begin(), pvec.end());
    	
    	for (Path_Vec_T::const_iterator it (pvec.begin()); it != pvec.end(); ++it){
            try {
                io::filtering_istream in;
                in.push(io::gzip_decompressor());
                ifstream infile(it->c_str());
                in.push(infile);
                string str;
                getline(in,str);
    	    
        	    if (jesusBorn < 0){ // init jesusBorn
                    EpochT time (str);
                    jesusBorn = time;
        	    }
                
                addr_5tup packet(str, jesusBorn); // readable
    
                if (packet.timestamp > simuT) {
                    io::close(in);
                    break;
        	    }
                to_proc_files.push_back(it->string());
                io::close(in);
            } catch(const io::gzip_error & e) {
                cout<<e.what()<<endl;
            }
        }
    }
}

uint32_t count_proc() {
    ifstream infile ("/proc/cpuinfo");
    uint32_t counter = 0;
    for (string str; getline(infile,str); )
        if (str.find("processor\t") != string::npos)
            counter++;
    return counter;
}

void tracer::merge_files(string proc_dir) const {
    fs::path file (proc_dir + "/ref_trace.gz");
    if (fs::exists(file))
        fs::remove(file);

    for (uint32_t i = 0; ; ++i) {
        stringstream ss;
        ss<<proc_dir<<"/ptrace-";
        ss<<std::setw(3)<<std::setfill('0')<<i;
	    ss<<".gz";
        fs::path to_merge(ss.str());

        if (fs::exists(to_merge)) {
            // output file is gzipped
            io::filtering_ostream out;
            out.push(io::gzip_compressor());
            ofstream out_file(proc_dir+"/ref_trace.gz", std::ios_base::app);
            out.push(out_file);

            cout << "Merging:" << ss.str()<<endl;

            // unzip input file
            io::filtering_istream in;
            in.push(io::gzip_decompressor());
            ifstream in_file(ss.str().c_str());
            in.push(in_file);

            // copy 
            io::copy(in, out);
            in.pop();
            fs::remove(to_merge);
            out.pop();
        } else
            break;
    }
}


const uint32_t mask_C = ((~uint32_t(0)) << 4);
struct hostpair{
	uint32_t pairs[2];
	
	hostpair(){pairs[0] = 0; pairs[1] = 0;}
	
	//hostpair(uint32_t i, uint32_t j){pairs[0] = i & mask_C; pairs[1] = j & mask_C;}
	hostpair(uint32_t i, uint32_t j){pairs[0] = i ; pairs[1] = j;}
	hostpair(const hostpair & hp){pairs[0] = hp.pairs[0]; pairs[1] = hp.pairs[1];}

	bool operator ==(const hostpair & rhs) const{
		return (pairs[0] == rhs.pairs[0] && pairs[1] == rhs.pairs[1]);
	}

	friend size_t hash_value(hostpair const & rhs){
		size_t seed = 0;
		boost::hash_combine(seed, rhs.pairs[0]);
		boost::hash_combine(seed, rhs.pairs[1]);
		return seed;
	}

};

bool cmp(const hostpair & lhs, const hostpair & rhs){
	if (lhs.pairs[0] < rhs.pairs[0])
		return true;
	if (lhs.pairs[0] == rhs.pairs[0] && lhs.pairs[1] < rhs.pairs[1])
		return true;
	return false;
}

// ===================================== Trace Generation and Evaluation =========================

void tracer::pFlow_pruning_gen() {
    // init processing file
    if (to_proc_files.size() == 0){
    	get_proc_files();
    }
    
    // create root dir
    fs::path dir(trace_root_dir);
    if (fs::create_directory(dir)) {
        cout<<"creating: " << dir.string()<<endl;
    } else {
        cout<<"exitst: "<<dir.string()<<endl;
    }

    // prepair the header file
    cout << "Preparing header file ... " << endl;
    vector<addr_5tup> headers;
    headers = rList->header_prep();
    
    // get the flow arrival time.
    cout << "Generating arrival time for each flow ... ..."<<endl;
    unordered_set<addr_5tup> flowInfo;
    if (fs::exists(fs::path(flowInfoFile_str))){
        ifstream infile(flowInfoFile_str.c_str());
        for (string str; getline(infile, str);) {
            addr_5tup packet(str);
            if (packet.timestamp > simuT)
                break;
            flowInfo.insert(packet);
        }
        infile.close();
        cout << "Warning: using old flowInfo file" <<endl;
    } else{
        flowInfo = flow_arr_mp();
	    cout << "flowInfo file generated" <<endl;
    }

    // trace generated in format of  "trace-200k"
    stringstream ss;
    ss<<dir.string()<<"/trace-"<<flow_rate<<"k";
    gen_trace_dir = ss.str();

    // initialize prune gen
    flow_pruneGen_mp(flowInfo, headers);
}


void tracer::flow_pruneGen_mp( unordered_set<addr_5tup> & flowInfo, 
                               const vector<addr_5tup> & headers) const {

    // create trace generation directory
    if (fs::create_directory(fs::path(gen_trace_dir)))
    	cout<<"creating: "<<gen_trace_dir<<endl;
    else
    	cout<<"exists:   "<<gen_trace_dir<<endl;

    // mapping: arrival time - header // boven? why using multimap?
    std::multimap<double, addr_5tup> ts_prune_map;
    for (auto iter=flowInfo.begin(); iter != flowInfo.end(); ++iter) {
        ts_prune_map.insert(std::make_pair(iter->timestamp, *iter));
    }
    cout << "total flow no. : " << ts_prune_map.size() <<endl; // counting host

    
    // smoothing every 10 sec, map the headers
    // pruned map:  key: old header   value (flow ID, mapped five tup) 
    boost::unordered_map<addr_5tup, pair<uint32_t, addr_5tup> > pruned_map;
    const double smoothing_interval = 10.0;
    double next_checkpoint = smoothing_interval;

    int flow_thres =  smoothing_interval* flow_rate;
    uint32_t flow_id = 0;
    uint32_t actual_flow_count = 0;
    vector<addr_5tup> orig_headers_buf;

    for (auto iter = ts_prune_map.begin(); iter != ts_prune_map.end(); ++iter) {

        // do the mapping after every smoothing interval
        if (iter->first > next_checkpoint) {
            random_shuffle(orig_headers_buf.begin(), orig_headers_buf.end());

            uint32_t i = 0;
            for (i = 0; i < flow_thres && i < orig_headers_buf.size() ; ++i) {
                addr_5tup header = headers[rand() % headers.size()]; 
                pruned_map.insert( std::make_pair(orig_headers_buf[i], 
                                   std::make_pair(flow_id, header)));
                ++flow_id;
            }
            actual_flow_count += i;
            next_checkpoint += smoothing_interval;
        }
        orig_headers_buf.push_back(iter->second);
    }
    cout << "after smoothing, average: " << double(actual_flow_count)/simuT <<endl;

    // process using multi-thread;
    fs::path temp1(gen_trace_dir+"/IDtrace");
    fs::create_directory(temp1);
    fs::path temp2(gen_trace_dir+"/GENtrace");
    fs::create_directory(temp2);

    vector< std::future<void> > results_exp;

    for(uint32_t file_id = 0; file_id < to_proc_files.size(); ++file_id) {
        results_exp.push_back(std::async(std::launch::async, &tracer::f_pg_st, this, to_proc_files[file_id], file_id, &pruned_map));
    }

    for (uint32_t file_id = 0; file_id < to_proc_files.size(); ++file_id) {
        results_exp[file_id].get();
    }

    cout<< "Merging Files... "<<endl;
    merge_files(gen_trace_dir+"/IDtrace");
    merge_files(gen_trace_dir+"/GENtrace");

    cout<<"Generation Finished. Enjoy :)" << endl;
    return;
}

void tracer::f_pg_st(string ref_file, uint32_t id, boost::unordered_map<addr_5tup, pair<uint32_t, addr_5tup> > * map_ptr) const {
    cout << "Processing " << ref_file << endl;
    io::filtering_istream in;
    in.push(io::gzip_decompressor());
    ifstream infile(ref_file);
    in.push(infile);

    stringstream ss;
    ss << gen_trace_dir<< "/IDtrace/ptrace-";
    ss << std::setw(3) << std::setfill('0')<<id;
    ss <<".gz";
    io::filtering_ostream out_id;
    out_id.push(io::gzip_compressor());
    ofstream outfile_id (ss.str().c_str());
    out_id.push(outfile_id);
    out_id.precision(15);

    stringstream ss1;
    ss1 << gen_trace_dir<< "/GENtrace/ptrace-";
    ss1 << std::setw(3) << std::setfill('0')<<id;
    ss1 <<".gz";
    io::filtering_ostream out_loc;
    out_loc.push(io::gzip_compressor());
    ofstream outfile_gen (ss1.str().c_str());
    out_loc.push(outfile_gen);
    out_loc.precision(15);

    for (string str; getline(in, str); ) {
        addr_5tup packet (str, jesusBorn); // readable;
	if (packet.timestamp > simuT)
		break;
        auto iter = map_ptr->find(packet);
        if (iter != map_ptr->end()) {
            packet.copy_header(iter->second.second);
            out_id << packet.timestamp << "\t" << iter->second.first<<endl;
            out_loc << packet.str_easy_RW() << endl;
        }
    }
    cout << " Finished Processing " << ref_file << endl;
    in.pop();
    out_id.pop();
    out_loc.pop();
}

/* flow_arr_mp
 * input: string ref_trace_dir: pcap reference trace
 * 	  string flow_info_str: output trace flow first packet infol
 * output:unordered_set<addr_5tup> : the set of first packets of all flows
 *
 * function_brief:
 * obtain first packet of each flow for later flow based pruning
 */
boost::unordered_set<addr_5tup> tracer::flow_arr_mp() const {
    cout << "Processing ... To process trace files " << to_proc_files.size() << endl;
    // process using multi-thread;
    vector< std::future<boost::unordered_set<addr_5tup> > > results_exp;
    for (uint32_t file_id = 0; file_id < to_proc_files.size(); file_id++) {
        results_exp.push_back(std::async(std::launch::async, &tracer::f_arr_st, this, to_proc_files[file_id]));
    }
    vector< boost::unordered_set<addr_5tup> >results;
    for (uint32_t file_id = 0; file_id < to_proc_files.size(); file_id++) {
        boost::unordered_set<addr_5tup> res = results_exp[file_id].get();
        results.push_back(res);
    }

    // merge the results;
    boost::unordered_set<addr_5tup> flowInfo_set;
    for (uint32_t file_id = 0; file_id < to_proc_files.size(); file_id++) {
        boost::unordered_set<addr_5tup> res = results[file_id];
        for ( boost::unordered_set<addr_5tup>::iterator iter = res.begin(); iter != res.end(); iter++) {
            auto ist_res = flowInfo_set.insert(*iter);
            if (!ist_res.second) { // update timestamp;
                if (iter->timestamp < ist_res.first->timestamp) {
                    addr_5tup rec = *ist_res.first;
                    rec.timestamp = iter->timestamp;
                    flowInfo_set.insert(rec);
                }
            }
        }
    }

    // print the results;
    ofstream outfile(flowInfoFile_str);
    outfile.precision(15);
    for (boost::unordered_set<addr_5tup>::iterator iter = flowInfo_set.begin(); iter != flowInfo_set.end(); ++iter) {
        outfile<< iter->str_easy_RW() <<endl;
    }
    outfile.close();
    return flowInfo_set;
}

/* f_arr_st
 * input: string ref_file: trace file path
 * output:unordered_set<addr_5tup> : pairtial set of all arrival packet;
 *
 * function_brief:
 * single thread process of flow_arr_mp
 */
boost::unordered_set<addr_5tup> tracer::f_arr_st(string ref_file) const {
    cout<<"Procssing " << ref_file<< endl;
    boost::unordered_set<addr_5tup> partial_flow_rec;
    io::filtering_istream in;
    in.push(io::gzip_decompressor());
    ifstream infile(ref_file);
    in.push(infile);
    for (string str; getline(in, str); ) {
        addr_5tup packet(str, jesusBorn);
        if (packet.timestamp > simuT)
            break;
        partial_flow_rec.insert(packet);
    }
    io::close(in);
    cout<<"Finished procssing " << ref_file << endl;
    return partial_flow_rec;
}


void tracer::parse_pcap_file_mp(size_t min, size_t max) const {
    if (fs::create_directory(fs::path(parsed_pcap_dir)))
	    cout << "creating" << parsed_pcap_dir <<endl;
    else
	    cout << "exists: "<< parsed_pcap_dir<<endl;

    const size_t File_BLK = 3;
    size_t thread_no = count_proc();
    size_t block_no = (max-min + 1)/File_BLK;
    if (block_no * 3 < max-min + 1)
        ++block_no;

    if ( thread_no > block_no) {
        thread_no = block_no;
    }

    size_t task_no = block_no/thread_no;

    vector<string> to_proc;
    size_t thread_id = 1;
    vector< std::future<void> > results_exp;

    size_t counter = 0;
    fs::path dir(pcap_dir);
    if (fs::exists(dir) && fs::is_directory(dir)) {
	Path_Vec_T pvec;
	std::copy(fs::directory_iterator(dir), fs::directory_iterator(), std::back_inserter(pvec));
	std::sort(pvec.begin(), pvec.end());
	
	for (Path_Vec_T::const_iterator it (pvec.begin()); it != pvec.end(); ++it){
	    if (counter < min){
	    	    ++counter;
		    continue;
	    }
	    if (counter > max)
		    break;
	    ++counter;

                if (to_proc.size() < task_no*File_BLK || thread_id == thread_no) {
                    to_proc.push_back(it->string());
                } else {
		    cout <<"thread " << thread_id << " covers : "<<endl;
		    for (auto iter = to_proc.begin(); iter != to_proc.end(); ++iter){
		    	cout << *iter << endl;
		    }

                    results_exp.push_back(std::async(
                                              std::launch::async,
                                              &tracer::p_pf_st,
                                              this, to_proc,
                                              (thread_id-1)*task_no)
                                         );
		    ++thread_id;
		    to_proc.clear();
                    to_proc.push_back(it->string());
                }
        }
    }

    cout <<"thread " << thread_id << " covers :" << endl;
    for (auto iter = to_proc.begin(); iter != to_proc.end(); ++iter){
	cout << *iter << endl;
    }

    results_exp.push_back(std::async(
                              std::launch::async,
                              &tracer::p_pf_st,
                              this, to_proc,
                              (thread_no-1)*task_no)
                         );
    for (size_t i = 0; i < thread_no; ++i)
	    results_exp[i].get();

    return;
}

void tracer::p_pf_st(vector<string> to_proc, size_t id) const {
    struct pcap_pkthdr header; // The header that pcap gives us
    const u_char *packet; // The actual packet

    pcap_t *handle;
    const struct sniff_ethernet * ethernet;
    const struct sniff_ip * ip;
    const struct sniff_tcp *tcp;
    uint32_t size_ip;
    uint32_t size_tcp;


    int count = 2;
    const size_t File_BLK = 3;

    stringstream ss;
    
    ss<<parsed_pcap_dir+"/packData";
    ss<<std::setw(3)<<std::setfill('0')<<id;
    ss<<"txt.gz";

    ofstream outfile(ss.str());
    cout << "created: "<<ss.str()<<endl;
    io::filtering_ostream out;
    out.push(io::gzip_compressor());
    out.push(outfile);

    for (size_t i = 0; i < to_proc.size(); ++i) {
        if (i > count) {
            out.pop();
            outfile.close();
	    ss.str(string());
            ss.clear();
            ++id;
            ss<<parsed_pcap_dir+"/packData";
            ss<<std::setw(3)<<std::setfill('0')<<id;
            ss<<"txt.gz";
            outfile.open(ss.str());
            cout << "created: "<<ss.str()<<endl;
	    out.push(outfile);
            count += File_BLK;
        }

        char errbuf[PCAP_ERRBUF_SIZE];
        handle = pcap_open_offline(to_proc[i].c_str(), errbuf);

        while (true) {
            packet = pcap_next(handle, &header);
            if (packet == NULL)
                break;

            ethernet = (struct sniff_ethernet*)(packet);
            int ether_offset = 0;
            if (ntohs(ethernet->ether_type) == ETHER_TYPE_IP) {
                ether_offset = 14;
            } else if (ntohs(ethernet->ether_type) == ETHER_TYPE_8021Q) {
                // here may have a bug
                ether_offset = 18;
            } else {
                continue;
            }

            ip = (struct sniff_ip*)(packet + ether_offset);
            size_ip = IP_HL(ip)*4;

            if (IP_V(ip) != 4 || size_ip < 20)
                continue;
            if (uint32_t(ip->ip_p) != 6)
                continue;

            tcp = (struct sniff_tcp*)(packet + ether_offset + size_ip);
            size_tcp = TH_OFF(tcp)*4;
            if (size_tcp < 20)
                continue;

            stringstream ss;
            ss<<header.ts.tv_sec<<'%'<<header.ts.tv_usec<<'%';
            ss<<ntohl(ip->ip_src.s_addr)<<'%'<<ntohl(ip->ip_dst.s_addr);
            ss<<'%'<<tcp->th_sport<<'%'<<tcp->th_dport;

            out<<ss.str()<<endl;
        }

        pcap_close(handle);
	cout << "finished_processing : "<< to_proc[i] << endl;
    }
    io::close(out);
}


/*
void tracer::raw_snapshot(string tracedir, double start_time, double interval) {
    fs::path dir(tracedir);
    
    Path_Vec_T pvec;
    if (fs::exists(dir) && fs::is_directory(dir)) {
	std::copy(fs::directory_iterator(dir), fs::directory_iterator(), std::back_inserter(pvec));
	std::sort(pvec.begin(), pvec.end());
    }
    else 
	return;
    
    EpochT jesusBorn(-1,0);

    boost::unordered_map<pair<size_t, size_t>, size_t> hostpair_rec;
    set<size_t> hosts;

    bool stop = false;
    bool start_processing = false;

    for (Path_Vec_T::const_iterator it (pvec.begin()); it != pvec.end() && !stop; ++it){
	io::filtering_istream in;
	in.push(io::gzip_decompressor());
	ifstream infile(it->c_str());
	in.push(infile);
	string str;
	getline(in, str);

	if (jesusBorn < 0){
		jesusBorn = EpochT(str);
	}
	if (EpochT(str) < jesusBorn + start_time){
		if ( it+1 != pvec.end() )
			continue;
	}
	else if (!start_processing){
		--it;
		start_processing = true;
	}

	cout << "processing" << it->c_str() << endl;

	in.pop();
	infile.close();
	infile.open(it->c_str());
	in.push(infile);

	for (string str; getline(in, str); ){
		addr_5tup packet (str, jesusBorn);
		if (packet.timestamp < start_time + interval){
			auto key = std::make_pair(packet.addrs[0], packet.addrs[1]);
			auto res = hostpair_rec.insert(std::make_pair(key, 1));
			hosts.insert(packet.addrs[0]);
			hosts.insert(packet.addrs[1]);
			if (!res.second) 
				++hostpair_rec[key];
		}
		else{
			stop = true;
			break;
		}
	}
    }

    cout << "total host no: "<< hosts.size()<<endl;
    cout << "total hostpair no: "<< hostpair_rec.size() <<endl;

    
    ofstream ff("snapshot.dat");
    for ( auto it = hostpair_rec.begin(); it != hostpair_rec.end(); ++it){
	    int x_dist = std::distance(hosts.begin(), hosts.find(it->first.first));
	    int y_dist = std::distance(hosts.begin(), hosts.find(it->first.second));
	    ff<<x_dist<<"\t"<<y_dist<<"\t"<<it->second<<endl;
    }
}

void tracer::pcap_snapshot(size_t file_st, double interval, pref_addr src_subnet, pref_addr dst_subnet){
    fs::path dir(pcap_dir);
    jesusBorn = EpochT(-1,0);
    
    boost::unordered_map<pair<size_t, size_t>, size_t> hostpair_rec;
    set<size_t> hosts;
    bool stop = false;

    if (fs::exists(dir) && fs::is_directory(dir)) {
	Path_Vec_T pvec;
	std::copy(fs::directory_iterator(dir), fs::directory_iterator(), std::back_inserter(pvec));
	std::sort(pvec.begin(), pvec.end());

	for (auto it = pvec.begin()+file_st; !stop && it != pvec.end(); ++it){
		
		struct pcap_pkthdr header; // The header that pcap gives us
		const u_char *packet; // The actual packet
    		pcap_t *handle;
    		const struct sniff_ethernet * ethernet;
    		const struct sniff_ip * ip;
    		const struct sniff_tcp *tcp;
    		uint32_t size_ip;
    		uint32_t size_tcp;
		char errbuf[PCAP_ERRBUF_SIZE];
		
		handle = pcap_open_offline(it->c_str(), errbuf);
		
		while (true) {
            		packet = pcap_next(handle, &header);
            		if (packet == NULL)
                		break;
			ethernet = (struct sniff_ethernet*)(packet);
			
			int ether_offset = 0;
			if (ntohs(ethernet->ether_type) == ETHER_TYPE_IP) {
				ether_offset = 14;
			} else if (ntohs(ethernet->ether_type) == ETHER_TYPE_8021Q) {
                		// here may have a bug
                		ether_offset = 18;
            		} else {
                		continue;
            		}
			
			ip = (struct sniff_ip*)(packet + ether_offset);
			
			size_ip = IP_HL(ip)*4;
			if (IP_V(ip) != 4 || size_ip < 20)
				continue;
            		if (uint32_t(ip->ip_p) != 6)
               		continue;

            		tcp = (struct sniff_tcp*)(packet + ether_offset + size_ip);
            		size_tcp = TH_OFF(tcp)*4;
            		if (size_tcp < 20)
                		continue;

			if (jesusBorn < 0)
				jesusBorn = EpochT(header.ts.tv_sec, header.ts.tv_usec);
	
			EpochT cur_ts(header.ts.tv_sec, header.ts.tv_usec);
			if (cur_ts.toDouble(jesusBorn) > interval){
				stop = true;
				break;
			}

			uint32_t ip_src = ntohl(ip->ip_src.s_addr);
			uint32_t ip_dst = ntohl(ip->ip_dst.s_addr);

			if (!(src_subnet.hit(ip_src) && dst_subnet.hit(ip_dst)))
				continue;
			auto key = std::make_pair(ip_src, ip_dst);
			auto res = hostpair_rec.insert(std::make_pair(key, 1));
			hosts.insert(ip_src);
			hosts.insert(ip_dst);
			if (!res.second) 
				++hostpair_rec[key];
        	}
		pcap_close(handle);
		cout << "finished_processing : "<< it->c_str() << endl;
	}
    }

    stringstream ss;
    ss << "snapshot-"<<file_st<<".dat";
    ofstream ff(ss.str());
    ofstream ff1("hostpair");

    for ( auto it = hosts.begin(); it != hosts.end(); ++it){
    	ff1 << *it << endl;
    }

    for ( auto it = hostpair_rec.begin(); it != hostpair_rec.end(); ++it){
	    int x_dist = std::distance(hosts.begin(), hosts.find(it->first.first));
	    int y_dist = std::distance(hosts.begin(), hosts.find(it->first.second));
	    ff<<x_dist<<"\t"<<y_dist<<"\t"<<it->second<<endl;
    }

    cout << "Finished Plotting.. " <<endl;

}
*/



/* packet_count_mp
 *
 * input: string real_trace_dir: directory of real traces
 * 	  string packet_count_file: output of packet counts
 *
 * function brief
 * counting the packets for each flow using multi-thread
 */
/*
void tracer::packet_count_mp(string real_trace_dir, string packet_count_file) {
    fs::path dir(real_trace_dir);
    fs::directory_iterator end;
    boost::unordered_map<addr_5tup, uint32_t> packet_count_map;
    uint32_t cpu_count = count_proc();
    mutex mtx;
    atomic_uint thr_n(0);
    atomic_bool reachend(false);
    if (fs::exists(dir) && fs::is_directory(dir)) {
        fs::directory_iterator iter(dir);
        while (iter != end) {
            if ((!reachend) && fs::is_regular_file(iter->status())) { // create thread
                std::thread (&tracer::p_count_st, this, iter->path(), &thr_n, &mtx, &packet_count_map, &reachend).detach();
                thr_n++;
            }
            iter++;

            while (thr_n >= cpu_count) // block thread creation
                std::this_thread::yield();
            while (thr_n > 0 && iter == end) // give time for thread to terminate
                std::this_thread::yield();
        }
    }

    cout << "finished processing" <<endl;
    // sleep for a while for last thread to close
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // debug
    uint32_t dbtotal = 0;
    ofstream ff(packet_count_file);
    for(Map_iter iter = packet_count_map.begin(); iter != packet_count_map.end(); iter++) {
        ff<<iter->first.str_easy_RW()<<"\t"<<iter->second<<endl;
        dbtotal+=iter->second;
    }
    cout<<dbtotal<<endl;
    ff.close();
    cout << "finished copy" <<endl;
    return;
}
*/

/* p_count_st
 *
 * input: string gz_file_ptr gz_file_ptr: file path of real traces
 * 	  atomic_unit * thr_n_ptr: control the concurrent thread no.
 * 	  mutex * mtx: control the shared access of pc_map
 * 	  unordered_map<addr_5tup, uint32_t> * pc_map_ptr: merge the results of counting
 * 	  atmoic_bool * reachend_ptr: indicate the termination of trace gen
 *
 * function brief
 * this is the thread function that produce packet counts
 */
/*
void tracer::p_count_st(const fs::path gz_file_ptr, atomic_uint * thr_n_ptr, mutex * mtx, boost::unordered_map<addr_5tup, uint32_t> * pc_map_ptr, atomic_bool * reachend_ptr) {
    cout<<"Processing:"<<gz_file_ptr.c_str()<<endl;
    uint32_t counter = 0;
    boost::unordered_map<addr_5tup, uint32_t> packet_count_map;
    try {
        io::filtering_istream in;
        in.push(io::gzip_decompressor());
        ifstream infile(gz_file_ptr.c_str());
        in.push(infile);
        for (string str; getline(in, str); ) {
            addr_5tup packet(str, jesusBorn);
            if (packet.timestamp > simuT) {
                (*reachend_ptr) = true;
                break;
            }

            counter++;
            auto result = packet_count_map.insert(std::make_pair(packet, 1));
            if (!result.second)
                result.first->second = result.first->second + 1;

        }
        io::close(in);
    } catch (const io::gzip_error & e) {
        cout<<e.what()<<std::endl;
    }

    total_packet += counter;

    std::lock_guard<mutex> lock(*mtx);

    for (Map_iter iter = packet_count_map.begin(); iter != packet_count_map.end(); iter++) {
        auto result = pc_map_ptr->insert(*iter);
        if (!result.second)
            (pc_map_ptr->find(iter->first))->second += iter->second;
    }
    --(*thr_n_ptr);
}
*/
