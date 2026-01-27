#ifndef NAP_PARAMETER_GROUP_H
#define NAP_PARAMETER_GROUP_H

#include "api/IParameter.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace nap {

/**
 * @brief Container for organizing related parameters
 *
 * Groups parameters together for logical organization,
 * supports nested groups and iteration over parameters.
 */
class ParameterGroup {
public:
    explicit ParameterGroup(const std::string& name);
    ~ParameterGroup();

    // Non-copyable, movable
    ParameterGroup(const ParameterGroup&) = delete;
    ParameterGroup& operator=(const ParameterGroup&) = delete;
    ParameterGroup(ParameterGroup&&) noexcept;
    ParameterGroup& operator=(ParameterGroup&&) noexcept;

    // Identity
    const std::string& getName() const;
    void setName(const std::string& name);

    // Parameter management
    void addParameter(std::shared_ptr<IParameter> parameter);
    void removeParameter(const std::string& name);
    IParameter* getParameter(const std::string& name);
    const IParameter* getParameter(const std::string& name) const;
    bool hasParameter(const std::string& name) const;

    // Nested groups
    void addGroup(std::unique_ptr<ParameterGroup> group);
    void removeGroup(const std::string& name);
    ParameterGroup* getGroup(const std::string& name);
    const ParameterGroup* getGroup(const std::string& name) const;
    bool hasGroup(const std::string& name) const;

    // Iteration
    size_t getParameterCount() const;
    size_t getGroupCount() const;

    void forEachParameter(const std::function<void(IParameter&)>& callback);
    void forEachParameter(const std::function<void(const IParameter&)>& callback) const;
    void forEachGroup(const std::function<void(ParameterGroup&)>& callback);
    void forEachGroup(const std::function<void(const ParameterGroup&)>& callback) const;

    // Bulk operations
    void resetAll();
    void clear();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nap

#endif // NAP_PARAMETER_GROUP_H
