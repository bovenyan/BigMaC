#include "../rules/RuleList.hpp"

#include <unordered_map>
#include <boost/unordered_set.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <set>


class SimuTest{
    private:
    // config
    rule_list * rList;
    std::string tracefile_str;
    int simuT;

    // matrix
    std::vector<int> flow_table_usage;
    int errorCount;

    public:
    SimuTest();
    void setConf(const char file_name[]);

    public:
    void evaluatePerFlow();
    void evaluateOBS();
    void evaluateBigMaC();
};
