#pragma once

#include "tramapoline\static_string.h"

#include <stdint.h>
#include <map>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <functional>

namespace tramapoline
{

using TypeHash = uint32_t;

struct RawTypeConstruction
{
	template <typename SerialType>
	static SerialType* Construct()
	{
		return new SerialType{};
	}
};

template<bool TSerialize, typename StreamType, typename T>
struct VirtualSerializer
{
public:
	static void Serialize(StreamType& a_stream, T& a_type, size_t VOffset)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Type is not trivially-copyable. Implement VirtualSerializer::Serialize for type T");
		a_stream.Serialize(TSerialize, (char*)(&a_type) + VOffset, static_cast<const unsigned int>(sizeof(T) - VOffset));
	}
};

template <typename BaseType, typename StreamType, size_t VOffset = 0,
	class ConstructPolicy = RawTypeConstruction>
class VirtualSerialization
{
public:
	using ConstructPolicyType = typename ConstructPolicy;

	// The policy isn't just static func since it can have state, e.g. some world/target
	VirtualSerialization(ConstructPolicyType constructPolicy)
		: constructPolicy_(constructPolicy)
	{

	}

	~VirtualSerialization() = default;

private:
	ConstructPolicyType constructPolicy_;

	template <typename BaseType, typename StreamType, typename ConstructionPolicy>
	struct Detail
	{
		struct CallableBase
		{
			virtual ~CallableBase() = default;

			virtual BaseType* Deserialize(StreamType& a_stream) = 0;
			virtual void Serialize(const BaseType* a_baseData, StreamType& a_outStream) = 0;
		};

		template <typename SerialType>
		struct AutoSerializer : CallableBase
		{
			// We have to pass the policy in at ctor time since the virtual methods below can't be templated
			AutoSerializer(ConstructionPolicy& constructionPolicy)
				: constructionPolicy_(constructionPolicy) {}

			void Serialize(const BaseType* a_baseData, StreamType& a_outStream)
			{
				static_assert(std::is_base_of_v<BaseType, SerialType>, "The type must be a subclass");

				VirtualSerializer<true, StreamType, SerialType>::
					Serialize(a_outStream, *const_cast<SerialType*>(static_cast<const SerialType*>(a_baseData)), 8);
			}

			BaseType* Deserialize(StreamType& a_stream) override
			{
				static_assert(std::is_base_of_v<BaseType, SerialType>, "The type must be a subclass");

				BaseType* type = constructionPolicy_.Construct<SerialType>();
				VirtualSerializer<false, StreamType, SerialType>::
					Serialize(a_stream, *static_cast<SerialType*>(type), 8);
				return type;
			}

			ConstructionPolicy& constructionPolicy_;

			//ConstructionCallable m_constructionCallable;
		};

		template<typename SerialType, typename ConstructPolicy>
		static auto CreateSerializer(ConstructPolicy& constructPolicy)
		{
			return std::make_unique<AutoSerializer<SerialType>>(constructPolicy);
		}
	};

public:
	template <typename SerialType/*, typename ConstructionCallable*/>
	void RegisterType(/*ConstructionCallable a_constructionCallable = std::function<SerialType*()>()*/)
	{
		TypeHash hash = hash_of<SerialType>();
		assert(m_serializers.find(hash) == m_serializers.end() && "Type should be registered once");

		auto serializer = Detail<BaseType, StreamType, ConstructPolicy>::template CreateSerializer<SerialType>(constructPolicy_);
		m_serializers.emplace(hash, std::move(serializer));

		m_typeInfoToTypeHash.insert({ (typeid(SerialType)), hash });
	}

	void Serialize(StreamType& a_outStream, const BaseType* a_baseType)
	{
		TypeHash hash = m_typeInfoToTypeHash[typeid(*a_baseType)];
		a_outStream.Write(hash);

		auto it = m_serializers.find(hash);
		if (it != m_serializers.end())
		{
			auto callable = (it)->second.get();
			std::invoke(&Detail<BaseType, StreamType, ConstructPolicy>::CallableBase::Serialize, callable, a_baseType, a_outStream);
		}
		else
		{
			throw std::runtime_error("No serializer found for event hash '%u'. Did you remember to call RegisterType<T>?\n");
		}
	}

	BaseType* Deserialize(StreamType& a_stream)
	{
		TypeHash hash;
		a_stream.Read(hash);

		auto it = m_serializers.find(hash);
		if (it != m_serializers.end())
		{
			auto callable = (it)->second.get();
			BaseType* evt = std::invoke(&Detail<BaseType, StreamType, ConstructPolicy>::CallableBase::Deserialize, callable, a_stream);
			return evt;
		}
		else
		{
			printf("No deserializer found for event hash '%u'. Did you remember to call RegisterType<T>\n", hash);
		}

		return nullptr;
	}

private:
	std::map<TypeHash, std::unique_ptr<typename Detail<BaseType, StreamType, ConstructPolicy>::CallableBase>> m_serializers;
	std::unordered_map<std::type_index, TypeHash> m_typeInfoToTypeHash;
};

} // namespace tramapoline
