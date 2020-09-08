#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace nsc::detail {
struct _event_registry {};
}  // namespace nsc::detail

namespace nsc {
using event_handle = size_t;

template <class E>
class event_callback_registry : public detail::_event_registry {
   public:
    typedef std::function<void(const E&)> Callback;

    event_handle subscribe(Callback&& c) {
        auto handle = _get_next_event_handle();
        callbacks_.emplace_back(handle, std::move(c));
        ++size_;
        return handle;
    }

    void unsubscribe(const event_handle handle) {
        auto pos = std::lower_bound(
            callbacks_.begin(), callbacks_.end(), handle,
            [](const auto& a, const auto& b) { return a.first < b; });
        if (pos == callbacks_.end() || pos->first != handle) {
            return;
        }

        pos->second.reset();
        --size_;

        if (size_ < (callbacks_.size() / 2)) {
            callbacks_.erase(
                std::remove_if(callbacks_.begin(), callbacks_.end(),
                               [](const auto& e) { return !e.second; }));
        }
    }

    void publish(const E& e) {
        for (auto& callback : callbacks_) {
            if (callback.second) {
                (*callback.second)(e);
            }
        }
    }

   private:
    std::vector<std::pair<event_handle, std::optional<Callback>>> callbacks_;
    size_t size_;

    inline event_handle _get_next_event_handle() noexcept {
        static event_handle id_ = 0;
        return id_++;
    }
};

class event_handler {
   public:
    template <typename E, typename Connection>
    event_handle subscribe(Connection&& conn) {
        auto reg = _get_registry<E>();
        if (!reg) {
            auto id = _get_event_registry_id<E>();
            registries_.emplace_back(
                id, std::make_unique<event_callback_registry<E>>());
            reg = _get_registry<E>();
            ++size_;
        }
        return reg->subscribe(std::move(conn));
    }

    template <typename E>
    void publish(const E& e) {
        if (auto reg = _get_registry<E>(); reg) {
            reg->publish(e);
        }
    }

    template <typename E>
    void unsubscribe(const event_handle handle) {
        if (auto reg = _get_registry<E>(); reg) {
            reg->unsubscribe(handle);
        }
    }

   private:
    using event_registry_id = std::size_t;
    using event_registry_ptr = std::unique_ptr<detail::_event_registry>;
    std::vector<std::pair<event_registry_id, std::optional<event_registry_ptr>>> registries_;
    size_t size_;

    inline event_registry_id _generate_next_event_registry_id() noexcept {
        static event_registry_id _id = 0;
        return _id++;
    }

    template <typename T>
    event_registry_id _get_event_registry_id() {
        static const event_registry_id id = _generate_next_event_registry_id();
        return id;
    }

    template <typename T>
    event_callback_registry<T>* _get_registry() {
        auto id = _get_event_registry_id<T>();
        auto pos = std::lower_bound(
            registries_.begin(), registries_.end(), id,
            [](const auto& a, const auto& b) { return a.first < b; });
        if (pos == registries_.end() || pos->first != id || !pos->second) {
            return nullptr;
        }

        return static_cast<event_callback_registry<T>*>(
            pos->second.value().get());
    }
};

}  // namespace nsc
