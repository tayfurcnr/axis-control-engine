#include "ace/communication/alp.hpp"

namespace ace::communication {

AlpFrame AlpParser::parse(std::string_view message) const
{
    // ALP is kept as a transport placeholder for future integration.
    // The raw frame is handed off unchanged.
    return {message};
}

}  // namespace ace::communication
