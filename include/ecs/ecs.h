#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <engine/utils.h>

namespace ecs
{
    using EntityID = uint32_t;

    // A Base class so the World can store different types of pools in one map
    class IPool
    {
    public:
        virtual ~IPool() = default;
        virtual void Remove(EntityID id) = 0;
    };

    // A Pool specifically for one type of Component (e.g., Position)
    template <typename T>
    class ComponentPool : public IPool
    {
    public:
        std::vector<T> data;                                // Dense array of actual data
        std::vector<EntityID> denseToEntity;                // Tells us: "Who owns Index 5?"
        std::unordered_map<EntityID, size_t> entityToIndex; // Tells us: "Where is Entity 42?"

        void Add(EntityID id, T component)
        {
            entityToIndex[id] = data.size();
            denseToEntity.push_back(id);
            data.push_back(component);
        }

        void Remove(EntityID id) override
        {
            if (entityToIndex.find(id) == entityToIndex.end())
                return;

            size_t indexToRemove = entityToIndex[id];
            size_t lastIndex = data.size() - 1;
            EntityID lastEntity = denseToEntity[lastIndex];

            // 1. Swap the data: Move the last element into the hole
            data[indexToRemove] = std::move(data[lastIndex]);
            denseToEntity[indexToRemove] = lastEntity;

            // 2. Update the mapping for the entity that just moved
            entityToIndex[lastEntity] = indexToRemove;

            // 3. Pop the last (now duplicate) elements
            data.pop_back();
            denseToEntity.pop_back();
            entityToIndex.erase(id);
        }

        T &Get(EntityID id) { return data[entityToIndex[id]]; }
    };

    class World
    {
    private:
        EntityID nextEntity = 0;
        // Maps a Component Type to its specific Pool
        std::unordered_map<std::type_index, std::shared_ptr<IPool>> componentPools;

    public:
        EntityID CreateEntity() { return nextEntity++; }

        template <typename T>
        ComponentPool<T> *GetPool()
        {
            auto type = std::type_index(typeid(T));
            if (componentPools.find(type) == componentPools.end())
            {
                componentPools[type] = std::make_shared<ComponentPool<T>>();
            }
            return static_cast<ComponentPool<T> *>(componentPools[type].get());
        }

        template <typename T>
        bool HasComponent(EntityID id)
        {
            auto type = std::type_index(typeid(T));

            // 1. Check if the pool for this type even exists
            auto it = componentPools.find(type);
            if (it == componentPools.end())
                return false;

            // 2. Access the pool and check if the entity is in it
            auto *pool = static_cast<ComponentPool<T> *>(it->second.get());
            return pool->entityToIndex.find(id) != pool->entityToIndex.end();
        }

        // Optional: A helper for your Entity wrapper too
        void RemoveEntity(EntityID id)
        {
            for (auto const &[type, pool] : componentPools)
            {
                pool->Remove(id);
            }
        }

        template <typename T>
        void AddComponent(EntityID id, T component)
        {
            GetPool<T>()->Add(id, component);
        }

        template <typename T>
        T &GetComponent(EntityID id)
        {
            return GetPool<T>()->Get(id);
        }
    };
}