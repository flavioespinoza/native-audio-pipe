#include "core/parameters/ParameterGroup.h"
#include <unordered_map>
#include <algorithm>

namespace nap {

class ParameterGroup::Impl {
public:
    std::string name;
    std::vector<std::shared_ptr<IParameter>> parameters;
    std::unordered_map<std::string, size_t> parameterIndex;
    std::vector<std::unique_ptr<ParameterGroup>> groups;
    std::unordered_map<std::string, size_t> groupIndex;

    explicit Impl(const std::string& n) : name(n) {}
};

ParameterGroup::ParameterGroup(const std::string& name)
    : pImpl(std::make_unique<Impl>(name)) {}

ParameterGroup::~ParameterGroup() = default;

ParameterGroup::ParameterGroup(ParameterGroup&&) noexcept = default;
ParameterGroup& ParameterGroup::operator=(ParameterGroup&&) noexcept = default;

const std::string& ParameterGroup::getName() const {
    return pImpl->name;
}

void ParameterGroup::setName(const std::string& name) {
    pImpl->name = name;
}

void ParameterGroup::addParameter(std::shared_ptr<IParameter> parameter) {
    if (!parameter) return;

    const std::string& name = parameter->getName();
    if (pImpl->parameterIndex.find(name) != pImpl->parameterIndex.end()) {
        return; // Already exists
    }

    pImpl->parameterIndex[name] = pImpl->parameters.size();
    pImpl->parameters.push_back(std::move(parameter));
}

void ParameterGroup::removeParameter(const std::string& name) {
    auto it = pImpl->parameterIndex.find(name);
    if (it == pImpl->parameterIndex.end()) return;

    size_t index = it->second;
    pImpl->parameters.erase(pImpl->parameters.begin() + index);
    pImpl->parameterIndex.erase(it);

    // Update indices
    for (auto& pair : pImpl->parameterIndex) {
        if (pair.second > index) {
            pair.second--;
        }
    }
}

IParameter* ParameterGroup::getParameter(const std::string& name) {
    auto it = pImpl->parameterIndex.find(name);
    if (it != pImpl->parameterIndex.end()) {
        return pImpl->parameters[it->second].get();
    }
    return nullptr;
}

const IParameter* ParameterGroup::getParameter(const std::string& name) const {
    auto it = pImpl->parameterIndex.find(name);
    if (it != pImpl->parameterIndex.end()) {
        return pImpl->parameters[it->second].get();
    }
    return nullptr;
}

bool ParameterGroup::hasParameter(const std::string& name) const {
    return pImpl->parameterIndex.find(name) != pImpl->parameterIndex.end();
}

void ParameterGroup::addGroup(std::unique_ptr<ParameterGroup> group) {
    if (!group) return;

    const std::string& name = group->getName();
    if (pImpl->groupIndex.find(name) != pImpl->groupIndex.end()) {
        return; // Already exists
    }

    pImpl->groupIndex[name] = pImpl->groups.size();
    pImpl->groups.push_back(std::move(group));
}

void ParameterGroup::removeGroup(const std::string& name) {
    auto it = pImpl->groupIndex.find(name);
    if (it == pImpl->groupIndex.end()) return;

    size_t index = it->second;
    pImpl->groups.erase(pImpl->groups.begin() + index);
    pImpl->groupIndex.erase(it);

    // Update indices
    for (auto& pair : pImpl->groupIndex) {
        if (pair.second > index) {
            pair.second--;
        }
    }
}

ParameterGroup* ParameterGroup::getGroup(const std::string& name) {
    auto it = pImpl->groupIndex.find(name);
    if (it != pImpl->groupIndex.end()) {
        return pImpl->groups[it->second].get();
    }
    return nullptr;
}

const ParameterGroup* ParameterGroup::getGroup(const std::string& name) const {
    auto it = pImpl->groupIndex.find(name);
    if (it != pImpl->groupIndex.end()) {
        return pImpl->groups[it->second].get();
    }
    return nullptr;
}

bool ParameterGroup::hasGroup(const std::string& name) const {
    return pImpl->groupIndex.find(name) != pImpl->groupIndex.end();
}

size_t ParameterGroup::getParameterCount() const {
    return pImpl->parameters.size();
}

size_t ParameterGroup::getGroupCount() const {
    return pImpl->groups.size();
}

void ParameterGroup::forEachParameter(const std::function<void(IParameter&)>& callback) {
    for (auto& param : pImpl->parameters) {
        callback(*param);
    }
}

void ParameterGroup::forEachParameter(const std::function<void(const IParameter&)>& callback) const {
    for (const auto& param : pImpl->parameters) {
        callback(*param);
    }
}

void ParameterGroup::forEachGroup(const std::function<void(ParameterGroup&)>& callback) {
    for (auto& group : pImpl->groups) {
        callback(*group);
    }
}

void ParameterGroup::forEachGroup(const std::function<void(const ParameterGroup&)>& callback) const {
    for (const auto& group : pImpl->groups) {
        callback(*group);
    }
}

void ParameterGroup::resetAll() {
    for (auto& param : pImpl->parameters) {
        param->reset();
    }
    for (auto& group : pImpl->groups) {
        group->resetAll();
    }
}

void ParameterGroup::clear() {
    pImpl->parameters.clear();
    pImpl->parameterIndex.clear();
    pImpl->groups.clear();
    pImpl->groupIndex.clear();
}

} // namespace nap
