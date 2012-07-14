namespace campaign
{
	enum npc_states
	{
		NS_NOTALIVE = -1,
		NS_IDLE = 0,
		NS_PATROLING,
		NS_SEARCHING,
		NS_CHASING,
		NS_AIMING,
		NS_ATTACKING,
	};

	//struct serializeable
	//{
	//	virtual int getsize();
	//	virtual int save(stream &s);
	//	virtual int load(stream &s);
	//};

	//struct cplayer: fpsent, serializeable
	//{

	//};

}