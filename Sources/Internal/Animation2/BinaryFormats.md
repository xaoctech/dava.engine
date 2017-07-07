
## Skeleton


    
    Skeleton
    {
        node_count          U4;
    
        data
        {
            parent_i        U4;
            bind_matrix     F4 16;
            inv_bind_matrix F4 16;
        } node_count;
    
        info
        {
            id              S0;
            name            S0;
        } node_count;
    }
    


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
    

