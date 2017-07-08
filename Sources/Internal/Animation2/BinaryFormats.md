## Animation Clip File

    FileHeader
	{
        signature       U4,
        version         U4,
	}
    
    AnimationClip
    {
        node_count      U4,
        
        nodes[node_count]
        {
            UID         S0,
            name        S0,
            data        Track
        }
    }
    
## Track Data

    Track
    {
        signature       U4,
        channel_count   U4,
    
        channels[channel_count]
        {
            name        S0,
            data        Channel
        }
    }
    
##Channel Data

    Channel
    {
        signature       U4
        type            U1,
        dimension       U1,
        pad             U1 2,
        key_count       U4,
    
        keys[key_count]
        {
            time        F4,
            data        F4 dim
        }
    }
