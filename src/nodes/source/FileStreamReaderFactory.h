#ifndef NAP_FILESTREAMREADERFACTORY_H
#define NAP_FILESTREAMREADERFACTORY_H

#include "FileStreamReader.h"
#include <memory>
#include <string>

namespace nap {

/**
 * @brief Factory for creating FileStreamReader instances.
 */
class FileStreamReaderFactory {
public:
    /**
     * @brief Create a new FileStreamReader instance.
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<FileStreamReader> create()
    {
        return std::make_unique<FileStreamReader>();
    }

    /**
     * @brief Create a new FileStreamReader with a file loaded.
     * @param filePath Path to the audio file
     * @return Unique pointer to the new node, or nullptr if file loading fails
     */
    static std::unique_ptr<FileStreamReader> create(const std::string& filePath)
    {
        auto node = std::make_unique<FileStreamReader>();
        if (!node->loadFile(filePath)) {
            return nullptr;
        }
        return node;
    }

    /**
     * @brief Create a new FileStreamReader with looping enabled.
     * @param filePath Path to the audio file
     * @param looping Whether to loop the file
     * @return Unique pointer to the new node
     */
    static std::unique_ptr<FileStreamReader> create(const std::string& filePath, bool looping)
    {
        auto node = std::make_unique<FileStreamReader>();
        node->loadFile(filePath);
        node->setLooping(looping);
        return node;
    }
};

} // namespace nap

#endif // NAP_FILESTREAMREADERFACTORY_H
