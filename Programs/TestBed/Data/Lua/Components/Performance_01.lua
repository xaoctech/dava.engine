-- Base interface of UI controller
-- All interface functions are OPTIONAL

local time = 0

function init(controlRef, componentRef)
    DV.Debug("init")
     
end

function release(controlRef, componentRef)
    DV.Debug("release")   
end

function process(controlRef, componentRef, frameDelta)
    time = time + frameDelta
    DV.Debug("process time:" .. time)

    for i = 1,500,1 
    do 
        -- controlRef.position = Vector2.Make(10 + 30 * FloatMath.sin(time), 0) 
        local x = 10 + 30 * math.sin(time) 
        local y = 0  
        controlRef.position.x = x
        controlRef.position.y = y

    end
    
--    DV.Debug("process")   
end
 