class Component
{
public:

	static Component * CreateComponent(String componentName); //create or get from cache

	List<Pool> data;

private:
	static Map<String. Component * > cache;//<name, component>
};
