
local time = 0
local sum = 0

function init(controlRef, componentRef)
    DV.Debug("init")
end

function release(controlRef, componentRef)
    DV.Debug("release")   
end

function process(controlRef, componentRef, frameDelta)
    time = time + frameDelta
    
    for i = 1,500,1 
    do 
        sum = math.sin(i) + sum
    end
end
 