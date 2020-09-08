#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace nsc {
using obj_handle = int;

namespace detail {
using type_id = std::size_t;

inline type_id _generate_next_type_id() noexcept {
    static type_id _id = 0;
    return _id++;
}

template <typename T>
type_id _get_type_id() {
    static const type_id id = _generate_next_type_id();
    return id;
}

struct _registry_object {
};
}  // namespace detail

template <typename T>
struct object_vector : public detail::_registry_object {
    inline obj_handle _generate_next_obj_id() noexcept {
        static obj_handle _id = 0;
        return _id++;
    }

    template <typename... Args>
    obj_handle emplace(Args... args) {
        auto id = _generate_next_obj_id();
        objs.push_back(T{std::forward<Args>(args)...});
        ids.push_back(id);
        ids_to_pos[id] = ids.size() - 1;
        return id;
    }

    obj_handle push_back(const T& t) {
        auto id = _generate_next_obj_id();
        objs.push_back(t);
        ids.push_back(id);
        ids_to_pos[id] = ids.size() - 1;
        return id;
    }

    T* get(obj_handle id) {
        if (!ids_to_pos.count(id) || ids_to_pos[id] < 0) {
            return nullptr;
        }

        return &objs[ids_to_pos[id]];
    }

    void remove(obj_handle id) {
        if (!ids_to_pos.count(id) || ids_to_pos[id] < 0) {
            return;
        }

        auto idx = ids_to_pos[id];
        ids_to_pos[id] = -1;
        objs.erase(objs.begin() + idx);
        ids.erase(ids.begin() + idx);

        for (int i = 0; i < ids.size(); i++) {
            ids_to_pos[ids[i]] = i;
        }
    }

    // objs and ids are parallel vectors
    std::vector<T> objs;
    std::vector<obj_handle> ids;
    std::unordered_map<obj_handle, int> ids_to_pos;
};

class object_registry {
   public:
    template <typename T, typename... Args>
    obj_handle create(Args&&... args) {
        return _get_obj_vec_or_make<T>()->emplace(std::forward<Args>(args)...);
    }

    template <typename T, typename Loader, typename... Args>
    obj_handle create(Loader* loader, Args&&... args) {
        static_assert(std::is_member_function_pointer_v<decltype(&Loader::create)>, "CANVAS: Attempted to create a type T with a loader that does not have a <T create(...)> function");
        static_assert(std::is_same<decltype(loader->create(args...)), T>::value, "CANVAS: Attempted to create a type T with a loader whose create(...) method doesn't return T!");
        T obj = loader->create(std::forward<Args>(args)...);
        return _get_obj_vec_or_make<T>()->push_back(obj);
    }

    template <typename T>
    std::vector<T>* get_all() {
        return &(_get_obj_vec_or_make<T>()->objs);
    }

    template <typename T>
    T* get(obj_handle id) {
        return _get_obj_vec_or_make<T>()->get(id);
    }

    template <typename T>
    void remove(obj_handle id) {
        if (_has_type<T>()) {
            _get_obj_vec_or_make<T>()->remove(id);
        }
    }

   private:
    template <typename T>
    object_vector<T>* _get_obj_vec_or_make() {
        auto id = detail::_get_type_id<T>();

        if (!object_collections.count(id)) {
            object_collections.insert({id, std::make_unique<object_vector<T>>()});
        }

        return static_cast<object_vector<T>*>(object_collections[id].get());
    }

    template <typename T>
    bool _has_type() {
        auto id = detail::_get_type_id<T>();
        return object_collections.count(id);
    }

    std::unordered_map<detail::type_id, std::unique_ptr<detail::_registry_object>> object_collections;
};
}  // namespace btn
