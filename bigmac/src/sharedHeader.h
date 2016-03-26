#ifndef SHARED_HEADER
#define SHARED_HEADER

#include <boost/log/core.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>


namespace logging = boost::log;
namespace logging_src = boost::log::sources;
namespace logging_sinks = boost::log::sinks;
namespace logging_keywords = boost::log::keywords;
namespace logging_expr = boost::log::expressions;

using namespace logging::trivial;

#endif
