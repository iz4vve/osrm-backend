#include "engine/plugins/nearest.hpp"
#include "engine/api/nearest_api.hpp"
#include "engine/api/nearest_parameters.hpp"
#include "engine/phantom_node.hpp"
#include "util/integer_range.hpp"

#include <cstddef>
#include <string>

#include <boost/assert.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace osrm
{
namespace engine
{
namespace plugins
{

NearestPlugin::NearestPlugin(const int max_results_) : max_results{max_results_} {}

Status NearestPlugin::HandleRequest(const RoutingAlgorithmsInterface &algorithms,
                                    const api::NearestParameters &params,
                                    util::json::Object &json_result) const
{
    BOOST_ASSERT(params.IsValid());

    const auto &facade = algorithms.GetFacade();

    if (max_results > 0 &&
        (boost::numeric_cast<std::int64_t>(params.number_of_results) > max_results))
    {
        return Error("TooBig",
                     "Number of results " + std::to_string(params.number_of_results) +
                         " is higher than current maximum (" + std::to_string(max_results) + ")",
                     json_result);
    }

    if (!CheckAllCoordinates(params.coordinates))
        return Error("InvalidOptions", "Coordinates are invalid", json_result);

    if (params.coordinates.size() != 1)
    {
        return Error("InvalidOptions", "Only one input coordinate is supported", json_result);
    }

    auto phantom_nodes = GetPhantomNodes(facade, params, params.number_of_results);

    if (phantom_nodes.front().size() == 0)
    {
        return Error("NoSegment", "Could not find a matching segments for coordinate", json_result);
    }
    BOOST_ASSERT(phantom_nodes.front().size() > 0);

    api::NearestAPI nearest_api(facade, params);
    nearest_api.MakeResponse(phantom_nodes, json_result);

    return Status::Ok;
}
}
}
}
