class Entity
{
public:
	int32 family;
	void AddComponent(String componentName);//

private:
	Component components[];
};