print("autotesting_api start")

TIMEOUT = 10.0

function Yield()
    print("Yield")
    
    coroutine.yield()
end

function OnError(msg)
    print("OnError "..msg)
    autotestingSystem:OnError(msg)
end

function ResumeTest()
    print("ResumeTest")
    if coroutine.status(co) == "suspended" then
        coroutine.resume(co)
        print("ResumeTest done")
    else
        print("ResumeTest failed. status:", coroutine.status(co))
        StopTest()
    end
end

function CreateTest(test)
    print("CreateTest")
    co = coroutine.create(test) -- create a coroutine with foo as the entry
    print(type(co))                 -- display the type of object "co"
    
    print(AutotestingSystem)
    autotestingSystem = AutotestingSystem.Singleton_Autotesting_Instance()
    print("AutotestingSystem.Singleton_Autotesting_Instance")
    
    print(autotestingSystem:GetTimeElapsed())
    
    print("CreateTest done")
end

function StartTest(test)
    print("StartTest")
    CreateTest(test)
    ResumeTest()
end

function StopTest()
    print("StopTest")
    autotestingSystem:StopTest()
end

function Wait(waitTime)
    print("Wait "..waitTime)
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        print("Waiting "..elapsedTime)
        coroutine.yield()
    end
    
    print("Wait done")
end

function WaitControl(name, time)
    local waitTime = time or TIMEOUT
    print("WaitControl name="..name.." waitTime="..waitTime)
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        print("Searching "..elapsedTime)
        
        if autotestingSystem:FindControl(name) then
            print("WaitControl found "..name)
            return true
        else
            coroutine.yield()
        end
    end
    
    print("WaitControl not found "..name)
    return false
end

function ClickControl(name, touchId, time)
    local waitTime = time or TIMEOUT
    local touchId = touchId or 1
    print("ClickControl name="..name.." touchId="..touchId.." waitTime="..waitTime)
    
    local elapsedTime = 0.0
    while elapsedTime < waitTime do
        elapsedTime = elapsedTime + autotestingSystem:GetTimeElapsed()
        print("Searching "..elapsedTime)
        
        local control = autotestingSystem:FindControl(name)
        if control then
            print("ClickControl found "..name)
            print(control)
            
            local position = control:GetPosition(true)
            print(position)
            print("position.x="..position.x)
            print("position.y="..position.y)
            
            local geomData = control:GetGeometricData()
            print(geomData)
            local rect = geomData:GetUnrotatedRect()
            print(rect)
            
            print("rect.x="..rect.x)
            print("rect.y="..rect.y)
            print("rect.dx="..rect.dx)
            print("rect.dy="..rect.dy)
            
            position.x = rect.x + rect.dx/2
            position.y = rect.y +rect.dy/2
            print("position.x="..position.x)
            print("position.y="..position.y)
            
            autotestingSystem:TouchDown(position, touchId)
            print("ClickControl TouchDown")
            
            Wait(0.5)
            
            autotestingSystem:TouchUp(touchId)
            print("ClickControl TouchUp")
            
            return true
        else
            coroutine.yield()
        end
    end
    
    print("ClickControl not found "..name)
    return false
end

function SetText(path, text)
    print("SetText path="..path.." text="..text)
    return autotestingSystem:SetText(path, text)
end

print("autotesting_api finish")