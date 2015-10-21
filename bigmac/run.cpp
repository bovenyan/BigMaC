#include <iostream>
#include <boost/program_options.hpp>

#include "sharedHeader.h"
#include "./mdCoding/mdCoding.h"
#include "./placement/routing.h"

namespace po = boost::program_options;

using std::cerr;
using std::cout;

void logging_init(int level) {
    logging::add_file_log(
        logging_keywords::file_name = "bigmac.log",
        // logging_keywords::rotation_size = 10 * 1024 * 1024,
        logging_keywords::time_based_rotation = logging_sinks::file::rotation_at_time_point(0,0,0),
        logging_keywords::auto_flush = true,
        logging_keywords::format = (
                                       logging_expr::stream <<
                                       logging_expr::format_date_time< boost::posix_time::ptime >
                                       ("TimeStamp", "%Y-%m-%d %H:%M:%S") << ": <" <<
                                       logging::trivial::severity << "> "<< logging_expr::smessage
                                   )
    );

    switch (level) {
    case 0:
        logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::trace
        );
        break;
    case 1:
        logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::debug
        );
        break;
    case 2:
        logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::info
        );
        break;
    case 3:
        logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::warning
        );
        break;
    case 4:
        logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::error
        );
        break;
    default:
        logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::fatal
        );
    }

    logging::add_common_attributes();
}

struct ParamType {
    int nsRuleNo;
    int sRuleNo;
    // mdCoding
    int avgDependency;
    double bypassProb;
    int groupSize;
    double rewiringProb;
    // Placement
    string topofile;
};

/*
 * Option setting for mdCoding
 */ 
void mdCodingOptions(po::options_description & desc, ParamType & param) {
    desc.add_options()
    ("dependency, d", po::value<int>(&param.avgDependency)->default_value(10), "No. of SRules overlapping each NSRule")
    ("group, g", po::value<int>(&param.groupSize)->default_value(50), "No. of NSrules in a group")
    ("bypass", po::value<double>(&param.bypassProb)->default_value(0.05), "Probability of communications to bypass")
    ("rewiring", po::value<double>(&param.rewiringProb)->default_value(0.05), "No. of communications to rewire");
}

void mdCodingParsing(po::variables_map v_map, ParamType & param, logging_src::severity_logger< severity_level > & logger_main) {
    if (v_map.count("mdcoding")) {
        mdCoding mdCodingTestObj;

        mdCodingTestObj.randGenEqList(param.nsRuleNo, param.sRuleNo, param.avgDependency,
                                      param.bypassProb, int(param.nsRuleNo/param.groupSize),
                                      param.rewiringProb);
        BOOST_LOG_SEV(logger_main, debug) << "Finished generating equations";

        mdCodingTestObj.coding();
        BOOST_LOG_SEV(logger_main, info) << "Finished coding equations";

        mdCodingTestObj.codingVerify();
        BOOST_LOG_SEV(logger_main, info) << "Finished verifying coding";
    }
}

/* 
 * Option setting for placement
 */ 

void placementOptions(po::options_description & desc, ParamType & param) {
    desc.add_options()
    ("topo", po::value<string>(&param.topofile)->default_value("./test.topo"), "Topology File");
}

void placementParsing(po::variables_map v_map, ParamType & param, logging_src::severity_logger< severity_level > & logger_main) {
    if (v_map.count("placement")) { 
        Topology topo(param.topofile);
        topo.calShortest();
        topo.printAllShortestPath();
    }
}

int main(int argc, char * argv[]) {
    try {
        int log;
        logging_src::severity_logger< severity_level > logger_main;

        ParamType param;

        po::options_description desc("Allowed options");
        desc.add_options()
        ("help", "produce help message")
        ("log", po::value<int>(&log)->default_value(3), "Log level: \n trace=0, debug=1, info=2, warning=3, error=4, fatal=5")
        ("nsRule, n", po::value<int>(&param.nsRuleNo)->default_value(1000), "No. of NSRules")
        ("sRule, s", po::value<int>(&param.sRuleNo)->default_value(1000), "No. of SRules")
        ("debug", "Log >= 1")
        ("mdcoding, M", "testing metadata coding")
        ("placement, P", "testing routing and rule placement")
        ;

        // adding Options for each project
        mdCodingOptions(desc, param);
        placementOptions(desc, param);

        po::positional_options_description pos_op;

        po::variables_map v_map;
        po::store(po::command_line_parser(argc, argv).
                  options(desc).positional(pos_op).run(), v_map);
        po::notify(v_map);

        if (v_map.count("help")) {
            cout<<"Usage: Blah [options]\n";
            cout<<desc;
            return 0;
        }

        if (v_map.count("debug")) {
            log = 1;
        }

        logging_init(log);

        // adding parser for each project
        mdCodingParsing(v_map, param, logger_main);
        placementParsing(v_map, param, logger_main);

        return 0;
    } catch (std::exception & e) {
        cerr << "Error: "<<e.what() << "\n";
        return 1;
    } catch (...) {
        cerr << "Exception of unknown type!\n";
    }
    return 0;
}
