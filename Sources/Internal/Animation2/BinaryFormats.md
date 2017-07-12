## Animation Clip File

    FileHeader
	{
        signature       U4,
        version         U4,
        crc32           U4,
        dataSize        U4,
	}
    
    AnimationClip
    {
        node_count      U4,
        
        node
        {
            UID         S0,
            name        S0,
            pad         U1 0...3 *(string aligment by zeros)*
            data        Track
        } [node_count]
    }
    
## Track Data

    Track
    {
        signature       U4,
        channel_count   U4,
    
        channels
        {
            target      U1,
            pad         U1 3,
            data        Channel
        } [channel_count]
    }
    
## Channel Data

    Channel
    {
        signature       U4
        dimension       U1,
        interpolation   U1,
        pad             U1 2,
        key_count       U4,
    
        keys[key_count]
        {
            time        F4,
            data        F4 dim
        }
    }
