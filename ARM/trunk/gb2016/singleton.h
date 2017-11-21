#ifndef __SINGLETON_H__
#define __SINGLETON_H__

template <typename T>
class Singleton		//应用于构造函数不带参数，返回为类引用的单例模式模板类
{
public:
	static T& GetInstance()
	{
	   static T instance;
	   return instance;
	}
private:
	Singleton();
	Singleton(const Singleton &);
	Singleton& operator = (const Singleton &);
};

template <typename T>
class PSingleton	//应用于构造函数带参数，返回为类指针的单例模式模板类
{
public:
	template<typename... Args>
	static T* Instance(Args&&... args)
	{
		static Destructor destructor;
        if (m_pInstance == nullptr)
            m_pInstance = new T(std::forward<Args>(args)...);
        return m_pInstance;
    }
	static T* GetInstance()
	{
		if (m_pInstance == nullptr)
			throw std::logic_error("the instance is not init, please initialize the instance first");
		return m_pInstance;
	}
	static void DestroyInstance()
    {
		if (m_pInstance != nullptr)
		{
			delete m_pInstance;
			m_pInstance = nullptr;
		}
    }

private:
	static T* m_pInstance;
	class Destructor
	{
	public:
		~Destructor() { DestroyInstance(); }
	};
	
	PSingleton();
	virtual ~PSingleton();
	PSingleton(const PSingleton&);
	PSingleton& operator = (const PSingleton&);
protected:
};

template <typename T>
T*  PSingleton<T>::m_pInstance = nullptr;

#endif
