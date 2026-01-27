#ifndef NAP_REVERBCONVOLUTIONFACTORY_H
#define NAP_REVERBCONVOLUTIONFACTORY_H

#include "ReverbConvolution.h"
#include <memory>
#include <string>

namespace nap {

class ReverbConvolutionFactory {
public:
    static std::unique_ptr<ReverbConvolution> create()
    {
        return std::make_unique<ReverbConvolution>();
    }

    static std::unique_ptr<ReverbConvolution> create(const std::string& irFilePath)
    {
        auto node = std::make_unique<ReverbConvolution>();
        node->loadImpulseResponse(irFilePath);
        return node;
    }

    static std::unique_ptr<ReverbConvolution> create(const float* irData, std::size_t irLength)
    {
        auto node = std::make_unique<ReverbConvolution>();
        node->loadImpulseResponse(irData, irLength);
        return node;
    }
};

} // namespace nap

#endif // NAP_REVERBCONVOLUTIONFACTORY_H
