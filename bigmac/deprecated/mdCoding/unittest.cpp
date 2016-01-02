#include <iostream>
#include <boost/program_options.hpp>

#include "../sharedHeader.h"
#include "mdCoding.h"

namespace po = boost::program_options;

using std::exception;
using std::cerr;
using std::cout;

void logging_init(int level) {
    logging::add_file_log(
        logging_keywords::file_name = "mdCoding.log",
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

int main(int ac, char * av[]) {
    try {
        int log;
	logging_src::severity_logger< severity_level > logger_main;
        // Testing Metadata Coding
        int nsRuleNo;
        int sRuleNo;
        int avgDependency;
        double bypassProb;
        int groupSize;
        double rewiringProb;

        po::options_description desc("Allowed options");
        desc.add_options()
        ("help", "produce help message")
        ("log", po::value<int>(&log)->default_value(3), "Log level: \n trace=0, debug=1, info=2, warning=3, error=4, fatal=5")
        ("debug", "Log >= 1")
        ("mdcoding, M", "testing metadata coding")
        ("nsRule, n", po::value<int>(&nsRuleNo)->default_value(1000), "No. of NSRules")
        ("sRule, s", po::value<int>(&sRuleNo)->default_value(1000), "No. of SRules")
        ("dependency, d", po::value<int>(&avgDependency)->default_value(10), "No. of SRules overlapping each NSRule")
        ("group, g", po::value<int>(&groupSize)->default_value(50), "No. of NSrules in a group")
        ("bypass", po::value<double>(&bypassProb)->default_value(0.05), "Probability of communications to bypass")
        ("rewiring", po::value<double>(&rewiringProb)->default_value(0.05), "No. of communications to rewire")
        ;

        po::positional_options_description pos_op;
        // pos_op.add("input-file", -1);

        po::variables_map v_map;
        po::store(po::command_line_parser(ac, av).
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

        if (v_map.count("mdcoding")) {
            mdCoding mdCodingTestObj;

            mdCodingTestObj.randGenEqList(nsRuleNo, sRuleNo, avgDependency,
                                          bypassProb, int(nsRuleNo/groupSize), rewiringProb);
	    BOOST_LOG_SEV(logger_main, debug) << "Finished generating equations";

            mdCodingTestObj.coding();
	    BOOST_LOG_SEV(logger_main, info) << "Finished coding equations";

            mdCodingTestObj.codingVerify();
	    BOOST_LOG_SEV(logger_main, info) << "Finished verifying coding";
        }

        return 0;
    }
    catch (exception& e) {
        cerr << "Error: "<<e.what() << "\n";
        return 1;
    }
    catch (...) {
        cerr << "Exception of unknown type!\n";
    }
    return 0;
}
