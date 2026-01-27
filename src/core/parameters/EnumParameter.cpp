#include "core/parameters/EnumParameter.h"
#include <algorithm>

namespace nap {

class EnumParameter::Impl {
public:
    std::string name;
    std::vector<std::string> options;
    size_t selectedIndex;
    size_t defaultIndex;
    ChangeCallback changeCallback;

    Impl(const std::string& n, const std::vector<std::string>& opts, size_t defIdx)
        : name(n)
        , options(opts)
        , defaultIndex(defIdx) {
        selectedIndex = std::min(defIdx, options.empty() ? 0 : options.size() - 1);
    }
};

EnumParameter::EnumParameter(const std::string& name,
                             const std::vector<std::string>& options,
                             size_t defaultIndex)
    : pImpl(std::make_unique<Impl>(name, options, defaultIndex)) {}

EnumParameter::~EnumParameter() = default;

const std::string& EnumParameter::getName() const {
    return pImpl->name;
}

ParameterType EnumParameter::getType() const {
    return ParameterType::Enum;
}

void EnumParameter::reset() {
    setSelectedIndex(pImpl->defaultIndex);
}

size_t EnumParameter::getSelectedIndex() const {
    return pImpl->selectedIndex;
}

void EnumParameter::setSelectedIndex(size_t index) {
    if (pImpl->options.empty()) return;

    size_t oldIndex = pImpl->selectedIndex;
    size_t newIndex = std::min(index, pImpl->options.size() - 1);

    if (oldIndex != newIndex) {
        pImpl->selectedIndex = newIndex;
        if (pImpl->changeCallback) {
            pImpl->changeCallback(oldIndex, newIndex);
        }
    }
}

const std::string& EnumParameter::getSelectedValue() const {
    static const std::string empty;
    if (pImpl->options.empty()) return empty;
    return pImpl->options[pImpl->selectedIndex];
}

bool EnumParameter::setSelectedValue(const std::string& value) {
    auto it = std::find(pImpl->options.begin(), pImpl->options.end(), value);
    if (it != pImpl->options.end()) {
        setSelectedIndex(std::distance(pImpl->options.begin(), it));
        return true;
    }
    return false;
}

const std::vector<std::string>& EnumParameter::getOptions() const {
    return pImpl->options;
}

size_t EnumParameter::getOptionCount() const {
    return pImpl->options.size();
}

void EnumParameter::setOptions(const std::vector<std::string>& options) {
    pImpl->options = options;
    if (!options.empty() && pImpl->selectedIndex >= options.size()) {
        pImpl->selectedIndex = options.size() - 1;
    }
}

void EnumParameter::addOption(const std::string& option) {
    pImpl->options.push_back(option);
}

void EnumParameter::selectNext() {
    if (pImpl->options.empty()) return;
    setSelectedIndex((pImpl->selectedIndex + 1) % pImpl->options.size());
}

void EnumParameter::selectPrevious() {
    if (pImpl->options.empty()) return;
    if (pImpl->selectedIndex == 0) {
        setSelectedIndex(pImpl->options.size() - 1);
    } else {
        setSelectedIndex(pImpl->selectedIndex - 1);
    }
}

size_t EnumParameter::getDefaultIndex() const {
    return pImpl->defaultIndex;
}

void EnumParameter::setChangeCallback(ChangeCallback callback) {
    pImpl->changeCallback = std::move(callback);
}

void EnumParameter::removeChangeCallback() {
    pImpl->changeCallback = nullptr;
}

} // namespace nap
