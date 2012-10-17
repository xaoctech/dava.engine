print("autotesting_api start")

TIMEOUT = 10.0

function Yield()
    print("Yield")
    
    coroutine.yield()
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
        
        if autotestingSystem:ClickControl(name, touchId) then
            print("ClickControl found "..name)
            return true
        else
            coroutine.yield()
        end
    end
    
    print("ClickControl not found "..name)
    return false
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

print("autotesting_api finish")