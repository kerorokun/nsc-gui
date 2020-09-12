#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace nsc {
using obj_handle = int;
using group_handle = std::size_t;

namespace detail {

struct _registry_object {
};

template <typename Handle, typename T>
struct pair {
    Handle handle;
    std::optional<T> value;
};

}  // namespace detail

template <typename T>
class group : public detail::_registry_object {
    using obj_pair = detail::pair<obj_handle, T>;

   public:
    class iterator final {
       public:
        using It = typename std::vector<obj_pair>::iterator;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        iterator(It from, It to) noexcept {
            it = from;
            end = to;
            if (it != end && !it->value) {
                ++(*this);
            }
        }

        iterator& operator++() {
            while (++it != end && !it->value) {
            }
            return *this;
        }

        iterator& operator++(int) {
            auto temp = *this;
            ++*this;
            return temp;
        }

        bool operator==(const iterator& other) const noexcept { return other.it == it; }
        bool operator!=(const iterator& other) const noexcept { return !(*this == other); }

        reference operator*() { return *it->value; }
        pointer operator->() { return &*it->value; }

       private:
        It it;
        It end;
    };

    class const_iterator final {
       public:
        using It = typename std::vector<obj_pair>::const_iterator;
        using value_type = T;
        using reference = const T&;
        using pointer = const T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        const_iterator(It from, It to) noexcept : it{from}, end{to} {
            if (it != end && !it->value) {
                ++(*this);
            }
        }

        const_iterator& operator++() {
            while (++it != end && !it->value) {
            }
            return *this;
        }

        const_iterator& operator++(int) {
            auto temp = *this;
            ++*this;
            return temp;
        }

        bool operator==(const const_iterator& other) const noexcept { return other.it == it; }
        bool operator!=(const const_iterator& other) const noexcept { return !(*this == other); }

        reference operator*() { return *it->value; }
        pointer operator->() { return &*it->value; }

       private:
        It it;
        It end;
    };

    iterator begin() { return {registry_.begin(), registry_.end()}; }
    iterator end() { return {registry_.end(), registry_.end()}; }

    const_iterator begin() const { return {registry_.begin(), registry_.end()}; }
    const_iterator end() const { return {registry_.end(), registry_.end()}; }

    template <typename... Args>
    obj_handle emplace(Args... args) {
        auto handle = _generate_next_obj_id();
        registry_.emplace_back(obj_pair{handle, T{std::forward<Args>(args)...}});
        ++size_;
        return handle;
    }

    obj_handle push_back(const T& t) {
        auto handle = _generate_next_obj_id();
        registry_.push_back(obj_pair{handle, t});
        ++size_;
        return handle;
    }

    T* get(const obj_handle& handle) {
        auto pos = std::lower_bound(registry_.begin(),
                                    registry_.end(),
                                    handle,
                                    [](const auto& a, const auto& b) { return a.handle < b; });

        if (pos == registry_.end() || pos->handle != handle || !pos->value) {
            return nullptr;
        }

        return &(*(pos->value));
    }

    void remove(const obj_handle& handle) {
        auto pos = std::lower_bound(registry_.begin(),
                                    registry_.end(),
                                    handle,
                                    [](const auto& a, const auto& b) { return a.handle < b; });

        if (pos == registry_.end() || pos->handle != handle) {
            return;
        }

        pos->value.reset();
        --size_;

        if (size_ < (registry_.size() / 2)) {
            registry_.erase(std::remove_if(registry_.begin(),
                                           registry_.end(),
                                           [](const auto& e) { return !e.value; }));
        }
    }

   private:
    inline obj_handle _generate_next_obj_id() noexcept {
        static obj_handle id_ = 0;
        return id_++;
    }

    size_t size_;
    std::vector<obj_pair> registry_;
};

class registry {
    using group_pair = detail::pair<group_handle, std::unique_ptr<detail::_registry_object>>;

   public:
    template <typename T, typename... Args>
    obj_handle create(Args&&... args) {
        return make_group<T>()->emplace(std::forward<Args>(args)...);
    }

    template <typename T, typename Loader, typename... Args>
    obj_handle create(Loader* loader, Args&&... args) {
        static_assert(std::is_member_function_pointer_v<decltype(&Loader::create)>, "CANVAS: Attempted to create a type T with a loader that does not have a <T create(...)> function");
        static_assert(std::is_same<decltype(loader->create(args...)), T>::value, "CANVAS: Attempted to create a type T with a loader whose create(...) method doesn't return T!");
        T obj = loader->create(std::forward<Args>(args)...);
        return make_group<T>()->push_back(obj);
    }

    template <typename T>
    group<T>* get_group() {
        auto handle = _get_group_handle<T>();
        auto pos = std::lower_bound(groups_.begin(),
                                    groups_.end(),
                                    handle,
                                    [](const auto& a, const auto& b) { return a.handle < b; });
        if (pos == groups_.end() || pos->handle != handle || !pos->value) {
            return nullptr;
        }

        return static_cast<group<T>*>(pos->value->get());
    }

    template <typename T>
    T* get(obj_handle id) {
        if (auto group = get_group<T>(); group) {
            return group->get(id);
        }
        return nullptr;
    }

    template <typename T>
    void remove(const obj_handle& id) {
        if (auto group = get_group<T>(); group) {
            group->remove(id);
        }
    }

   private:
    inline group_handle _generate_next_group_handle() noexcept {
        static group_handle _id = 0;
        return ++_id;
    }

    template <typename T>
    group_handle _get_group_handle() {
        static const group_handle id = _generate_next_group_handle();
        return id;
    }

    template <typename T>
    group<T>* make_group() {
        auto existing_group = get_group<T>();
        if (existing_group) {
            return existing_group;
        }

        auto handle = _get_group_handle<T>();
        auto new_group = std::make_unique<group<T>>();
        groups_.push_back(group_pair {handle, std::move(new_group)});
        ++size_;
        return get_group<T>();
    }

    size_t size_;
    std::vector<group_pair> groups_;
};
}  // namespace nsc
