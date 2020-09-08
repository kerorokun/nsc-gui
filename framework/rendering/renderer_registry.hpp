#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace nsc::rendering::detail {

template <typename... Args>
struct renderable {
   public:
    template <typename T>
    renderable(T x) : self_(std::make_unique<model_t<T>>(std::move(x))) {}

    void render(Args... args) {
        self_->render(std::forward<Args>(args)...);
    }

   private:
    struct concept_t {
        virtual ~concept_t() = default;
        virtual void render(Args... args) = 0;
    };

    template <typename T>
    struct model_t final : concept_t {
        model_t(T x) : data_(std::move(x)) {}
        virtual void render(Args... args) override { data_.render(std::forward<Args>(args)...); }
        T data_;
    };

    std::unique_ptr<concept_t> self_;
};
}  // namespace btn::rendering::detail

namespace nsc::rendering {

using renderable_handle = int;

template <typename... Args>
struct renderer_registry {
    renderer_registry() {}
    ~renderer_registry() {}

    renderable_handle emplace_renderable(detail::renderable<Args...> renderable) {
        auto handle = _get_next_renderable_handle();
        renderables_.emplace_back(handle, std::move(renderable));
        ++size_;
        return handle;
    }

    void remove_renderable(const renderable_handle& handle) {
        auto pos = std::lower_bound(
            renderables_.begin(), renderables_.end(), handle,
            [](const auto& a, const auto& b) { return a.first < b; });

        if (pos == renderables_.end() || pos->first != handle) {
            return;
        }

        pos->second.reset();
        --size_;

        if (size_ < (renderables_.size() / 2)) {
            renderables_.erase(std::remove_if(renderables_.begin(), renderables_.end(), [](const auto& e) { return !e.second; }));
        }
    }

    void clear_renderables() {
        renderables_.clear();
    }

    void render(Args... args) {
        for (auto& renderable : renderables_) {
            if (renderable.second) {
                (*renderable.second).render(std::forward<Args>(args)...);
            }
        }
    }

   private:
    std::vector<std::pair<renderable_handle, std::optional<detail::renderable<Args...>>>> renderables_;
    size_t size_;

    inline renderable_handle _get_next_renderable_handle() noexcept {
        static renderable_handle id_ = 0;
        return id_++;
    }
};
}  // namespace btn::rendering