main()
{
	//should be delayed (probably to the end of current frame)
	Entity * stone = EntityManager::CreateEntity();
	stone->AddComponent("VisibilityAABBox");
	stone->AddComponent("SrawScene");
}