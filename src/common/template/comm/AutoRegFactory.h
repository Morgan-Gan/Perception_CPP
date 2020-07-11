#pragma once
#include <map>
#include <string>
#include <functional>
#include <memory>

namespace common_template
{
	class Message
	{
	public:
		virtual ~Message() {}

		virtual void foo()
		{

		}
	};

	class Message1 : public Message
	{
	public:

		Message1()
		{
			std::cout << "message1" << std::endl;
		}

		Message1(int a)
		{
			std::cout << "message1" << std::endl;
		}

		~Message1()
		{
		}

		void foo() override
		{
			std::cout << "message1" << std::endl;
		}
	};


	class factory
	{
	public:
		template<typename T>
		class register_t
		{
		public:
			register_t(const std::string& key)
			{
				factory::get().map_.emplace(key, [] { return new T(); });
			}

			template<typename... Args>
			register_t(const std::string& key, Args... args)
			{
				factory::get().map_.emplace(key, [&] { return new T(args...); });
			}
		};

		static Message* produce(const std::string& key)
		{
			if (map_.find(key) == map_.end())
				throw std::invalid_argument("the message key is not exist!");

			return map_[key]();
		}

		static std::unique_ptr<Message> produce_unique(const std::string& key)
		{
			return std::unique_ptr<Message>(produce(key));
		}

		static std::shared_ptr<Message> produce_shared(const std::string& key)
		{
			return std::shared_ptr<Message>(produce(key));
		}

	private:
		factory() {};
		factory(const factory&) = delete;
		factory(factory&&) = delete;

		static factory& get()
		{
			static factory instance;
			return instance;
		}

		static std::map<std::string, std::function<Message*()>> map_;
	};

	std::map<std::string, std::function<Message*()>> factory::map_;

	#define REGISTER_MESSAGE_VNAME(T) reg_msg_##T##_
	#define REGISTER_MESSAGE(T, key, ...) static factory::register_t<T> REGISTER_MESSAGE_VNAME(T)(key, ##__VA_ARGS__);
}

int TestAutoFactory()
{
	REGISTER_MESSAGE(Message1, "message1");
	Message* p = factory::produce("message1");
	p->foo();  

	auto p2 = factory::produce_unique("message1");
	p2->foo();
}