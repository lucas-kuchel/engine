#pragma once

#include <data/sparse.hpp>
#include <data/tree.hpp>

namespace data {
    // @brief Handle for a resource in a registry
    // @tparam The type stored in the registry
    // @note No instance of the type is stored in this handle
    template <typename T>
    class Handle {
    public:
        // @brief Represents a "dead" or uninitialised handle
        static constexpr Handle<T> Dead = Handle{};

        constexpr Handle()
            : id_(std::numeric_limits<std::size_t>::max()), generation_(std::numeric_limits<std::size_t>::max()) {
        }

        Handle(const Handle&) = default;
        Handle(Handle&&) noexcept = default;

        Handle& operator=(const Handle&) = default;
        Handle& operator=(Handle&&) noexcept = default;

        bool operator==(const Handle<T>& other) const {
            return id_ == other.id_ && generation_ == other.generation_;
        }

        bool operator!=(const Handle<T>& other) const {
            return id_ != other.id_ || generation_ != other.generation_;
        }

        explicit operator bool() const {
            return *this != Dead;
        }

        bool operator<(const Handle<T>& other) const {
            return id_ < other.id_ || (id_ == other.id_ && generation_ < other.generation_);
        }

        bool operator>(const Handle<T>& other) const {
            return id_ > other.id_ || (id_ == other.id_ && generation_ > other.generation_);
        }

        bool operator<=(const Handle<T>& other) const {
            return *this < other || *this == other;
        }

        bool operator>=(const Handle<T>& other) const {
            return *this > other || *this == other;
        }

    private:
        std::size_t id_ = std::numeric_limits<std::size_t>::max();
        std::size_t generation_ = std::numeric_limits<std::size_t>::max();

        template <typename>
        friend class Registry;

        friend struct std::hash<Handle<T>>;
    };

    // @brief Represents a registry of a given type
    // @tparam Any type
    template <typename T>
    class Registry {
    public:
        Registry() = default;
        ~Registry() = default;

        Registry(const Registry&) = default;
        Registry(Registry&&) noexcept = default;

        Registry& operator=(const Registry&) = default;
        Registry& operator=(Registry&&) noexcept = default;

        // @brief Inserts a new element into the registry
        // @param Element to insert
        // @return A handle to the new element
        [[nodiscard]] Handle<T> insert(T&& data) {
            Handle<T> handle;

            if (!freeList_.empty()) {
                handle.id_ = freeList_.back();
                handle.generation_ = generations_[handle.id_];
                freeList_.pop_back();
            }
            else {
                handle.id_ = generations_.size();
                generations_.push_back(0);
                handle.generation_ = 0;
            }

            data_.insert(handle.id_, std::forward<T>(data));

            return handle;
        }

        // @brief Removes the element corresponding to the handle
        // @param Handle to the element
        // @throws std::runtime_error if handle has no matching entry
        void remove(Handle<T> handle) {
            if (!data_.contains(handle.id_) || generations_[handle.id_] != handle.generation_) {
                throw std::runtime_error("Call failed: data::Registry::remove(): index out of range");
            }

            generations_[handle.id_]++;
            freeList_.push_back(handle.id_);
            data_.remove(handle.id_);
        }

        // @brief Retrieves the element corresponding to the provided handle
        // @param Handle to the element
        // @throws std::runtime_error if the handle has no matching entry
        [[nodiscard]] T& get(Handle<T> handle) {
            if (!data_.contains(handle.id_) || generations_[handle.id_] != handle.generation_) {
                throw std::runtime_error("Call failed: data::Registry::get(): index out of range");
            }

            return data_.get(handle.id_);
        }

        // @brief Retrieves the element corresponding to the provided handle
        // @param Handle to the element
        // @throws std::runtime_error if the handle has no matching entry
        [[nodiscard]] const T& get(Handle<T> handle) const {
            if (!data_.contains(handle.id_) || generations_[handle.id_] != handle.generation_) {
                throw std::runtime_error("Call failed: data::Registry::get(): index out of range");
            }

            return data_.get(handle.id_);
        }

    private:
        SparseSet<T> data_;

        std::vector<std::size_t> freeList_;
        std::vector<std::size_t> generations_;
    };

    template <typename T>
    inline constexpr bool CanBeComponent = !std::is_pointer_v<T> &&
                                           !std::is_reference_v<T> &&
                                           !std::is_const_v<T> &&
                                           !std::is_volatile_v<T> &&
                                           !std::is_array_v<T> &&
                                           !std::is_function_v<T> &&
                                           !std::is_void_v<T> &&
                                           !std::is_same_v<T, std::nullptr_t> &&
                                           std::is_standard_layout_v<T>;

    template <typename... Ts>
    inline constexpr bool CanBeComponents = (CanBeComponent<Ts> && ...);

    template <typename...>
    inline constexpr bool ComponentsAreUnique = true;

    template <typename T, typename... Us>
    inline constexpr bool ComponentsAreUnique<T, Us...> = (!std::is_same_v<T, Us> && ...) && ComponentsAreUnique<Us...>;

    template <typename... Ts>
    requires(ComponentsAreUnique<Ts...> && CanBeComponents<Ts...> && sizeof...(Ts) > 0)
    class MultiRegistryComponents {
    public:
        MultiRegistryComponents() = delete;

        template <typename T>
        static constexpr bool Contains = (std::is_same_v<Ts, T> || ...);

        template <typename T>
        requires(Contains<T>)
        static constexpr std::size_t index() {
            constexpr std::array<bool, sizeof...(Ts)> matchFlags = {std::is_same_v<T, Ts>...};

            for (std::size_t i = 0; i < matchFlags.size(); i++) {
                if (matchFlags[i]) {
                    return i;
                }
            }

            return std::numeric_limits<std::size_t>::max();
        }
    };

    template <typename>
    inline constexpr bool IsMultiRegistryComponents = false;

    template <typename... Ts>
    inline constexpr bool IsMultiRegistryComponents<MultiRegistryComponents<Ts...>> = true;

    template <typename T>
    requires(IsMultiRegistryComponents<T>)
    class MultiRegistry;

    template <typename>
    inline constexpr bool IsMultiRegistry = false;

    template <typename D>
    inline constexpr bool IsMultiRegistry<MultiRegistry<D>> = true;

    class MultiHandle {
    public:
        constexpr MultiHandle()
            : id_(std::numeric_limits<std::size_t>::max()), generation_(std::numeric_limits<std::size_t>::max()) {
        }

        MultiHandle(const MultiHandle&) = default;
        MultiHandle(MultiHandle&&) noexcept = default;

        MultiHandle& operator=(const MultiHandle&) = default;
        MultiHandle& operator=(MultiHandle&&) noexcept = default;

        bool operator==(const MultiHandle& other) const {
            return id_ == other.id_ && generation_ == other.generation_;
        }

        bool operator!=(const MultiHandle& other) const {
            return id_ != other.id_ || generation_ != other.generation_;
        }

        explicit operator bool() const {
            return *this != MultiHandle{};
        }

        bool operator<(const MultiHandle& other) const {
            return id_ < other.id_ || (id_ == other.id_ && generation_ < other.generation_);
        }

        bool operator>(const MultiHandle& other) const {
            return id_ > other.id_ || (id_ == other.id_ && generation_ > other.generation_);
        }

        bool operator<=(const MultiHandle& other) const {
            return *this < other || *this == other;
        }

        bool operator>=(const MultiHandle& other) const {
            return *this > other || *this == other;
        }

    private:
        MultiHandle(std::size_t id, std::size_t generation)
            : id_(id), generation_(generation) {
        }

        std::size_t id_ = std::numeric_limits<std::size_t>::max();
        std::size_t generation_ = std::numeric_limits<std::size_t>::max();

        template <typename T>
        requires(IsMultiRegistryComponents<T>)
        friend class MultiRegistry;

        template <typename R, typename... Ts>
        requires(IsMultiRegistry<R> && ComponentsAreUnique<Ts...> && (R::DescriptorType::template Contains<Ts> && ...))
        friend class EntityView;

        friend struct std::hash<MultiHandle>;
    };

    template <typename R, typename... Ts>
    requires(IsMultiRegistry<R> && ComponentsAreUnique<Ts...> && (R::DescriptorType::template Contains<Ts> && ...))
    class EntityView {
    public:
        class Iterator {
        public:
            Iterator(R* registry, data::SparseSet<MultiHandle>* entities, std::size_t index, std::size_t end)
                : registry_(registry), handles_(entities), index_(index), end_(end) {
            }

            auto operator*() const {
                MultiHandle target = handles_->dense()[index_];

                return std::tuple<MultiHandle, Ts&...>(target, registry_->template sparseSet<Ts>().dense()[registry_->template sparseSet<Ts>().sparse()[target.id_]]...);
            }

            bool operator!=(const EntityView::Iterator&) const {
                return index_ < end_;
            }

            EntityView::Iterator& operator++() {
                ++index_;

                return *this;
            }

        private:
            R* registry_;

            data::SparseSet<MultiHandle>* handles_;

            std::size_t index_;
            std::size_t end_;
        };

        using ConstIterator = const Iterator;

        EntityView(R* registry, data::SparseSet<MultiHandle>& entities)
            : registry_(registry), end_(entities.dense().size()), handles_(&entities) {
        }

        [[nodiscard]] Iterator begin() {
            return EntityView::Iterator(registry_, handles_, 0, end_);
        }

        [[nodiscard]] Iterator end() {
            return EntityView::Iterator(registry_, handles_, end_, end_);
        }

        [[nodiscard]] ConstIterator cbegin() const {
            return EntityView::Iterator(registry_, handles_, 0, end_);
        }

        [[nodiscard]] ConstIterator cend() const {
            return EntityView::Iterator(registry_, handles_, end_, end_);
        }

    private:
        R* registry_;

        std::size_t end_;

        data::SparseSet<MultiHandle>* handles_;
    };

    template <typename... Ts>
    class MultiRegistry<MultiRegistryComponents<Ts...>> {
    public:
        using BitsetType = std::bitset<sizeof...(Ts)>;
        using ArchetypeType = data::SparseSet<MultiHandle>;
        using DescriptorType = MultiRegistryComponents<Ts...>;

        MultiRegistry() = default;
        ~MultiRegistry() = default;

        MultiRegistry(const MultiRegistry&) = delete;
        MultiRegistry(MultiRegistry&&) noexcept = default;

        MultiRegistry& operator=(const MultiRegistry&) = delete;
        MultiRegistry& operator=(MultiRegistry&&) noexcept = default;

        [[nodiscard]] inline MultiHandle insertBlank() {
            if (freeList_.empty()) {
                std::size_t size = handles_.size();

                handles_.push_back({size, 0});
                entityMasks_.emplace_back();

                return handles_.back();
            }
            else {
                std::size_t index = freeList_.back();
                MultiHandle& handle = handles_[index];
                BitsetType& bitset = entityMasks_[index];

                handle.generation_++;
                freeList_.pop_back();
                bitset.reset();

                return handle;
            }
        }

        template <typename... TQueried>
        requires((MultiRegistryComponents<Ts...>::template Contains<TQueried> && ...) && (sizeof...(TQueried) != 0))
        inline MultiHandle insert() {
            MultiHandle handle = insertBlank();
            std::size_t id = handle.id_;
            BitsetType mask = bitmask<TQueried...>();

            entityMasks_[id] = mask;

            ArchetypeType& archetype = archetypes_.insert(mask);

            archetype.insert(id, handle);

            (addToSparseSet<TQueried>(handle, TQueried()), ...);

            return handle;
        }

        template <typename... TQueried>
        requires((MultiRegistryComponents<Ts...>::template Contains<TQueried> && ...) && (sizeof...(TQueried) != 0))
        inline MultiHandle insert(TQueried&&... components) {
            MultiHandle handle = insertBlank();
            std::size_t id = handle.id_;
            BitsetType mask = bitmask<TQueried...>();

            entityMasks_[id] = mask;

            ArchetypeType& archetype = archetypes_.insert(mask);

            archetype.insert(id, handle);

            (addToSparseSet<TQueried>(handle, std::forward<TQueried>(components)), ...);

            return handle;
        }

        inline void remove(MultiHandle handle) {
            if (!contains(handle)) {
                throw std::runtime_error("Call failed: data::MultiRegistry::remove(): Element does not exist");
            }

            std::size_t id = handle.id_;
            std::size_t generation = handle.generation_;

            const BitsetType& bitset = entityMasks_[id];
            ArchetypeType& archetype = archetypes_.Get(bitset);

            removeFromSparseSets(handle, bitset);

            auto& dense = archetype.dense();

            archetype.remove(id);

            if (dense.size() == 0) {
                archetypes_.remove(bitset);
            }

            freeList_.push_back(id);
            handles_[id] = {std::numeric_limits<std::size_t>::max(), generation};
            entityMasks_[id].reset();
        }

        [[nodiscard]] inline bool contains(MultiHandle handle) const {
            std::size_t& id = handle.id_;
            std::size_t& generation = handle.generation_;

            return id < handles_.size() && handles_[id].generation_ == generation && handles_[id].id_ != std::numeric_limits<std::size_t>::max();
        }

        template <typename U>
        requires(MultiRegistryComponents<Ts...>::template Contains<U>)
        [[nodiscard]] inline bool hasComponent(MultiHandle handle) const {
            std::size_t& id = handle.id_;

            if (contains(handle)) {
                constexpr std::size_t index = MultiRegistryComponents<Ts...>::template index<U>();

                return entityMasks_[id].test(index);
            }

            return false;
        }

        template <typename... TQueried>
        requires((MultiRegistryComponents<Ts...>::template Contains<TQueried> && ...) && sizeof...(TQueried) != 0)
        [[nodiscard]] inline bool hasComponents(MultiHandle handle) const {
            return (hasComponent<TQueried>(handle) && ...);
        }

        template <typename TComponent>
        requires(MultiRegistryComponents<Ts...>::template Contains<TComponent>)
        inline void addComponent(MultiHandle handle, TComponent&& component) {
            if (!contains(handle) || hasComponent<TComponent>(handle)) {
                throw std::runtime_error("Call failed: data::MultiRegistry::remove(): Element does not exist or already has this component type");
            }

            auto& set = std::get<data::SparseSet<TComponent>>(sparseSets_);

            constexpr std::size_t index = MultiRegistryComponents<Ts...>::template index<TComponent>();

            BitsetType& targetBitset = entityMasks_[handle.id_];
            BitsetType oldBitset = entityMasks_[handle.id_];

            targetBitset.set(index);

            if (updateArchetype(handle, oldBitset, targetBitset)) {
                set.insert(handle.id_, std::forward<TComponent>(component));
            }
        }

        template <typename... TQueried>
        requires((MultiRegistryComponents<Ts...>::template Contains<TQueried> && ...) && sizeof...(TQueried) != 0)
        inline void addComponents(MultiHandle handle, TQueried&&... components) {
            (addComponent<TQueried>(handle, std::forward<TQueried>(components)), ...);
        }

        template <typename TComponent>
        requires(MultiRegistryComponents<Ts...>::template Contains<TComponent>)
        inline void removeComponent(MultiHandle handle) {
            if (!contains(handle) || !hasComponent<TComponent>(handle)) {
                throw std::runtime_error("Call failed: data::MultiRegistry::remove(): Element does not exist or already has no component of this type");
            }

            auto& set = std::get<data::SparseSet<TComponent>>(sparseSets_);

            constexpr std::size_t index = MultiRegistryComponents<Ts...>::template index<TComponent>();

            BitsetType& targetBitset = entityMasks_[handle.id_];
            BitsetType oldBitset = entityMasks_[handle.id_];

            targetBitset.reset(index);

            updateArchetype(handle, oldBitset, targetBitset);

            set.remove(handle.id_);
        }

        template <typename... TQueried>
        requires((MultiRegistryComponents<Ts...>::template Contains<TQueried> && ...) && sizeof...(TQueried) != 0)
        inline void removeComponents(MultiHandle handle) {
            return (removeComponent<TQueried>(handle), ...);
        }

        template <typename TComponent>
        requires(MultiRegistryComponents<Ts...>::template Contains<TComponent>)
        inline TComponent& getElement(MultiHandle handle) {
            if (!contains(handle) || !hasComponent<TComponent>(handle)) {
                throw std::runtime_error("Call failed: data::MultiRegistry::remove(): Element does not exist or already has no component of this type");
            }

            auto& set = std::get<data::SparseSet<TComponent>>(sparseSets_);

            return set.dense()[set.sparse()[handle.id_]];
        }

        template <typename TComponent>
        requires(MultiRegistryComponents<Ts...>::template Contains<TComponent>)
        inline const TComponent& getElement(MultiHandle handle) const {
            if (!contains(handle) || !hasComponent<TComponent>(handle)) {
                throw std::runtime_error("Call failed: data::MultiRegistry::remove(): Element does not exist or already has no component of this type");
            }

            const auto& set = std::get<data::SparseSet<TComponent>>(sparseSets_);

            return set.dense()[set.sparse()[handle.id_]];
        }

        [[nodiscard]] inline data::BitsetTree<ArchetypeType, sizeof...(Ts)>& GetArchetypes() {
            return archetypes_;
        }

        [[nodiscard]] inline const data::BitsetTree<ArchetypeType, sizeof...(Ts)>& GetArchetypes() const {
            return archetypes_;
        }

        [[nodiscard]] ArchetypeType& GetArchetype(const BitsetType& mask) {
            return archetypes_.Get(mask);
        }

        [[nodiscard]] const ArchetypeType& GetArchetype(const BitsetType& mask) const {
            return archetypes_.Get(mask);
        }

        template <typename TComponent>
        requires(MultiRegistryComponents<Ts...>::template Contains<TComponent>)
        [[nodiscard]] inline constexpr data::SparseSet<TComponent>& sparseSet() {
            return std::get<data::SparseSet<TComponent>>(sparseSets_);
        }

        template <typename TComponent>
        requires(MultiRegistryComponents<Ts...>::template Contains<TComponent>)
        [[nodiscard]] inline constexpr const data::SparseSet<TComponent>& sparseSet() const {
            return std::get<data::SparseSet<TComponent>>(sparseSets_);
        }

        template <typename... TQueried>
        requires((MultiRegistryComponents<Ts...>::template Contains<TQueried> && ...) && sizeof...(TQueried) != 0)
        [[nodiscard]] static inline constexpr BitsetType bitmask() {
            BitsetType bitset;

            (bitset.set(MultiRegistryComponents<Ts...>::template index<TQueried>()), ...);

            return bitset;
        }

        template <typename... TQueried>
        requires(MultiRegistryComponents<Ts...>::template Contains<TQueried> && ...)
        [[nodiscard]] inline auto entityView(ArchetypeType& archetype) {
            return EntityView<Registry<MultiRegistryComponents<Ts...>>, TQueried...>(this, archetype);
        }

        template <typename... TQueried>
        requires(MultiRegistryComponents<Ts...>::template Contains<TQueried> && ...)
        [[nodiscard]] inline auto entityView(ArchetypeType& archetype) const {
            return EntityView<Registry<MultiRegistryComponents<Ts...>>, TQueried...>(this, archetype);
        }

    private:
        [[nodiscard]] inline bool updateArchetype(MultiHandle handle, const BitsetType& oldBitset, const BitsetType& newBitset) {
            if (oldBitset == newBitset) {
                return false;
            }

            auto& oldArchetype = archetypes_.Get(oldBitset);

            if (!oldArchetype.remove(handle.id_)) {
                return false;
            }

            auto& dense = oldArchetype.dense();

            if (dense.size() == 0) {
                archetypes_.remove(oldBitset);
            }

            ArchetypeType& archetype = archetypes_.insert(newBitset);
            auto& id = handle.id_;
            archetype.insert(id, handle);

            return true;
        }

        template <std::size_t... Ns>
        inline void removeFromSparseSetsImplementation(MultiHandle handle, const BitsetType& mask, std::index_sequence<Ns...>) {
            ((mask.test(Ns) ? (void)std::get<Ns>(sparseSets_).remove(handle.id_) : (void)0), ...);
        }

        inline void removeFromSparseSets(MultiHandle handle, const BitsetType& mask) {
            removeFromSparseSetsImplementation(handle, mask, std::index_sequence_for<Ts...>{});
        }

        template <typename T, typename U>
        inline void addToSparseSet(MultiHandle& handle, U&& component) {
            data::SparseSet<T>& sparseSet = std::get<data::SparseSet<T>>(sparseSets_);
            sparseSet.insert(handle.id_, std::forward<U>(component));
        }

        std::tuple<data::SparseSet<Ts>...> sparseSets_;

        data::BitsetTree<ArchetypeType, sizeof...(Ts)> archetypes_;

        std::vector<BitsetType> entityMasks_;
        std::vector<MultiHandle> handles_;
        std::vector<std::size_t> freeList_;
    };
}

namespace std {
    template <typename T>
    struct hash<data::Handle<T>> {
        std::size_t operator()(const data::Handle<T>& handle) const noexcept {
            std::size_t hash1 = std::hash<std::size_t>{}(handle.id_);
            std::size_t hash2 = std::hash<std::size_t>{}(handle.generation_);

            return hash1 ^ (hash2 << 1);
        }
    };

    template <>
    struct hash<data::MultiHandle> {
        std::size_t operator()(const data::MultiHandle& handle) const noexcept {
            std::size_t hash1 = std::hash<std::size_t>{}(handle.id_);
            std::size_t hash2 = std::hash<std::size_t>{}(handle.generation_);

            return hash1 ^ (hash2 << 1);
        }
    };
}
