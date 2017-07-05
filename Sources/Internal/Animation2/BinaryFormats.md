
## Animation

    
    Channel
    {
        type        U1,
        dimension   U1,
        pad         U1 2,
    
        key_count   U4,
    
        key
        {
            time    F4,
            data    F4 dim
        } key_count
    }
    

    
    Track
    {
        chan_count  U4,
    
        chan
        {
            name    S0,
            data    Channel
        } chan_count
    }
    


    
    Clip
    {
        node_count  U4,
        node
        {
            id      S0,
            name    S0,
            data    Track
        } node_count
    }
    

