#include "akutik/tools/IgluStore.hpp"
#include <algorithm>

namespace akutik {
namespace tools {

IgluStore::IgluStore(std::shared_ptr<Storage> storage)
    : storage_(std::move(storage))
{
}

bool IgluStore::remove(const std::string& namespace_, const std::string& key) {
    std::string fullKey = makeKey(namespace_, key);
    return storage_->remove(fullKey);
}

std::vector<std::string> IgluStore::keysInNamespace(const std::string& namespace_) {
    std::string pattern = namespace_ + ":*";
    auto fullKeys = storage_->keys(pattern);

    // Strip namespace prefix from keys
    std::vector<std::string> keys;
    keys.reserve(fullKeys.size());

    for (const auto& fullKey : fullKeys) {
        auto [ns, key] = splitKey(fullKey);
        if (ns == namespace_) {
            keys.push_back(key);
        }
    }

    return keys;
}

size_t IgluStore::clearNamespace(const std::string& namespace_) {
    auto keys = keysInNamespace(namespace_);

    size_t removed = 0;
    for (const auto& key : keys) {
        if (remove(namespace_, key)) {
            removed++;
        }
    }

    return removed;
}

bool IgluStore::beginTransaction() {
    return storage_->beginTransaction();
}

bool IgluStore::commit() {
    return storage_->commit();
}

bool IgluStore::rollback() {
    return storage_->rollback();
}

bool IgluStore::batch(std::function<bool()> operations) {
    if (!beginTransaction()) {
        return false;
    }

    bool success = operations();

    if (success) {
        return commit();
    } else {
        rollback();
        return false;
    }
}

size_t IgluStore::count(const std::string& namespace_) {
    return keysInNamespace(namespace_).size();
}

bool IgluStore::namespaceExists(const std::string& namespace_) {
    return count(namespace_) > 0;
}

std::string IgluStore::makeKey(const std::string& namespace_, const std::string& key) const {
    return namespace_ + ":" + key;
}

std::pair<std::string, std::string> IgluStore::splitKey(const std::string& fullKey) const {
    size_t colonPos = fullKey.find(':');
    if (colonPos == std::string::npos) {
        return {"", fullKey};
    }

    std::string namespace_ = fullKey.substr(0, colonPos);
    std::string key = fullKey.substr(colonPos + 1);
    return {namespace_, key};
}

} // namespace tools
} // namespace akutik
